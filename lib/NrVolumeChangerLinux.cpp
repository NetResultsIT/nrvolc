#include "NrVolumeChangerLinux.h"
#include <algorithm>
#include <alsa/asoundlib.h>
#include <chrono>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pulse/pulseaudio.h>
#include <thread>
#include <vector>

namespace {
const char *prefix(bool input) {
  return input ? "pulse:source:" : "pulse:sink:";
}
bool valid(double v) { return std::isfinite(v) && v >= 0 && v <= 100; }
struct Device {
  std::string name, description;
  pa_cvolume volume;
};
struct Pulse {
  pa_mainloop *loop = nullptr;
  pa_context *context = nullptr;
  bool done = false, success = false;
  std::string defaultInput, defaultOutput;
  std::vector<Device> devices;
  std::chrono::steady_clock::time_point deadline;
  Pulse()
      : deadline(std::chrono::steady_clock::now() + std::chrono::seconds(2)) {
    loop = pa_mainloop_new();
    if (loop)
      context = pa_context_new(pa_mainloop_get_api(loop), "nrvolchanger");
  }
  ~Pulse() {
    if (context) {
      pa_context_disconnect(context);
      pa_context_unref(context);
    }
    if (loop)
      pa_mainloop_free(loop);
  }
  bool pump() {
    if (!context || std::chrono::steady_clock::now() >= deadline)
      return false;
    int r;
    if (pa_mainloop_iterate(loop, 0, &r) < 0)
      return false;
    pa_context_state_t s = pa_context_get_state(context);
    if (s == PA_CONTEXT_FAILED || s == PA_CONTEXT_TERMINATED)
      return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    return true;
  }
  bool connect() {
    if (!context || pa_context_connect(context, nullptr, PA_CONTEXT_NOAUTOSPAWN,
                                       nullptr) < 0)
      return false;
    while (pa_context_get_state(context) != PA_CONTEXT_READY)
      if (!pump())
        return false;
    return true;
  }
  bool wait(pa_operation *op) {
    if (!op)
      return false;
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING && !done)
      if (!pump())
        break;
    bool result = done && success;
    if (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
      pa_operation_cancel(op);
    pa_operation_unref(op);
    return result;
  }
};
void serverCb(pa_context *, const pa_server_info *i, void *p) {
  Pulse &s = *static_cast<Pulse *>(p);
  s.done = true;
  s.success = i;
  if (i) {
    if (i->default_source_name)
      s.defaultInput = i->default_source_name;
    if (i->default_sink_name)
      s.defaultOutput = i->default_sink_name;
  }
}
void sinkCb(pa_context *, const pa_sink_info *i, int eol, void *p) {
  Pulse &s = *static_cast<Pulse *>(p);
  if (eol) {
    s.done = true;
    s.success = eol > 0;
    return;
  }
  if (i && i->name && pa_cvolume_valid(&i->volume))
    s.devices.push_back(
        {i->name, i->description ? i->description : i->name, i->volume});
}
void sourceCb(pa_context *, const pa_source_info *i, int eol, void *p) {
  Pulse &s = *static_cast<Pulse *>(p);
  if (eol) {
    s.done = true;
    s.success = eol > 0;
    return;
  }
  if (i && i->name && i->monitor_of_sink == PA_INVALID_INDEX &&
      pa_cvolume_valid(&i->volume))
    s.devices.push_back(
        {i->name, i->description ? i->description : i->name, i->volume});
}
void successCb(pa_context *, int ok, void *p) {
  Pulse &s = *static_cast<Pulse *>(p);
  s.done = true;
  s.success = ok != 0;
}
bool list(Pulse &p, bool input) {
  p.done = p.success = false;
  return p.wait(input ? pa_context_get_source_info_list(p.context, sourceCb, &p)
                      : pa_context_get_sink_info_list(p.context, sinkCb, &p));
}
bool pulseVolume(bool input, const std::string &uid, bool set, double &value) {
  if (set && !valid(value))
    return false;
  Pulse p;
  if (!p.connect())
    return false;
  std::string name;
  if (uid.empty()) {
    if (!p.wait(pa_context_get_server_info(p.context, serverCb, &p)))
      return false;
    name = input ? p.defaultInput : p.defaultOutput;
  } else {
    std::string pre(prefix(input));
    if (uid.compare(0, pre.size(), pre))
      return false;
    name = uid.substr(pre.size());
  }
  if (name.empty() || !list(p, input))
    return false;
  auto it = std::find_if(p.devices.begin(), p.devices.end(),
                         [&](const Device &d) { return d.name == name; });
  if (it == p.devices.end())
    return false;
  if (!set) {
    value = 100.0 * pa_cvolume_max(&it->volume) / PA_VOLUME_NORM;
    return true;
  }
  pa_cvolume vol = it->volume;
  pa_volume_t target =
      static_cast<pa_volume_t>(std::lround(value * PA_VOLUME_NORM / 100.0));
  pa_cvolume_scale(&vol, target);
  p.done = p.success = false;
  return p.wait(input ? pa_context_set_source_volume_by_name(
                            p.context, name.c_str(), &vol, successCb, &p)
                      : pa_context_set_sink_volume_by_name(
                            p.context, name.c_str(), &vol, successCb, &p));
}
// PortAudio's display-name shortening, reproduced for strict identity
// verification.
std::string shortPcm(const char *pcm, const char *card) {
  const char *p = pcm, *c = card, *last = p;
  while (*c) {
    while (*p && *c && *p == *c) {
      ++p;
      ++c;
      if (*p == ' ' || !*p)
        last = p;
    }
    p = last;
    while (*c && *c++ != ' ') {
    }
  }
  if (!*p)
    return "-";
  while (*last == ' ')
    ++last;
  if ((*last == '-' || *last == ':') && last[1] == ' ')
    last += 2;
  return last;
}
bool parseHardwareUid(const std::string &uid, size_t &pos, int &card, int &dev) {
  pos = uid.rfind(" (");
  size_t end = uid.size();
  if (pos == std::string::npos || end < pos + 6 || uid[end - 1] != ')')
    return false;
  std::string route = uid.substr(pos + 2, end - pos - 3);
  size_t start = 0;
  if (route.compare(0, 7, "plughw:") == 0)
    start = 7;
  else if (route.compare(0, 3, "hw:") == 0)
    start = 3;
  else
    return false;
  size_t comma = route.find(',', start);
  auto number = [](const std::string &s, size_t first, size_t last, int &out) {
    if (first >= last || (last - first > 1 && s[first] == '0'))
      return false;
    int n = 0;
    for (size_t i = first; i < last; ++i) {
      if (s[i] < '0' || s[i] > '9' || n > (INT_MAX - (s[i] - '0')) / 10)
        return false;
      n = n * 10 + s[i] - '0';
    }
    out = n;
    return true;
  };
  if (comma == std::string::npos || !number(route, start, comma, card) ||
      !number(route, comma + 1, route.size(), dev))
    return false;
  return true;
}
bool hardware(bool input, const std::string &uid, bool set, double &value) {
  if (set && !valid(value))
    return false;
  size_t pos;
  int card, dev;
  if (!parseHardwareUid(uid, pos, card, dev))
    return false;
  std::string cardId = "hw:" + std::to_string(card);
  snd_mixer_t *m = nullptr;
  if (snd_mixer_open(&m, 0) < 0)
    return false;
  snd_hctl_t *hctl = nullptr;
  if (snd_mixer_attach(m, cardId.c_str()) < 0 ||
      snd_mixer_get_hctl(m, cardId.c_str(), &hctl) < 0 || !hctl) {
    snd_mixer_close(m);
    return false;
  }
  // Keep the same open control handle for identity validation and writes.
  snd_ctl_t *ctl = snd_hctl_ctl(hctl);
  if (!ctl) {
    snd_mixer_close(m);
    return false;
  }
  snd_ctl_card_info_t *ci;
  snd_ctl_card_info_alloca(&ci);
  snd_pcm_info_t *pi;
  snd_pcm_info_alloca(&pi);
  // PortAudio queries capture first and playback second, then builds the
  // displayed name from the last successful PCM info query.
  snd_pcm_info_set_device(pi, dev);
  snd_pcm_info_set_subdevice(pi, 0);
  snd_pcm_info_set_stream(pi, SND_PCM_STREAM_CAPTURE);
  int captureResult = snd_ctl_pcm_info(ctl, pi);
  bool capture = captureResult >= 0;
  std::string pcmName = capture ? snd_pcm_info_get_name(pi) : "";
  snd_pcm_info_set_stream(pi, SND_PCM_STREAM_PLAYBACK);
  int playbackResult = snd_ctl_pcm_info(ctl, pi);
  bool playback = playbackResult >= 0;
  auto expectedMissing = [](int result) {
    return result >= 0 || result == -ENOENT || result == -ENXIO;
  };
  if (playback)
    pcmName = snd_pcm_info_get_name(pi);
  bool identity =
      expectedMissing(captureResult) && expectedMissing(playbackResult) &&
      (input ? capture : playback) && snd_ctl_card_info(ctl, ci) >= 0;
  if (identity) {
    identity = uid.substr(0, pos) ==
               std::string(snd_ctl_card_info_get_name(ci)) + ": " +
                   shortPcm(pcmName.c_str(), snd_ctl_card_info_get_name(ci));
  }
  // A card-wide control cannot safely be associated with one of several
  // PCM devices in the same direction without an explicit mapping.
  int next = -1, count = 0, enumResult = 0;
  while (identity && (enumResult = snd_ctl_pcm_next_device(ctl, &next)) >= 0 &&
         next >= 0) {
    snd_pcm_info_set_device(pi, next);
    snd_pcm_info_set_subdevice(pi, 0);
    snd_pcm_info_set_stream(pi, input ? SND_PCM_STREAM_CAPTURE
                                      : SND_PCM_STREAM_PLAYBACK);
    int result = snd_ctl_pcm_info(ctl, pi);
    if (result >= 0)
      ++count;
    else if (!expectedMissing(result)) {
      identity = false;
      break;
    }
  }
  if (!identity || enumResult < 0 || count != 1) {
    snd_mixer_close(m);
    return false;
  }
  bool ok = snd_mixer_selem_register(m, nullptr, nullptr) >= 0 &&
            snd_mixer_load(m) >= 0;
  // A single direction-capable volume element is the only unambiguous choice.
  snd_mixer_elem_t *chosen = nullptr;
  if (ok)
    for (snd_mixer_elem_t *e = snd_mixer_first_elem(m); e;
         e = snd_mixer_elem_next(e)) {
      if (!snd_mixer_selem_is_active(e) ||
          !(input ? snd_mixer_selem_has_capture_volume(e)
                  : snd_mixer_selem_has_playback_volume(e)))
        continue;
      if (chosen) {
        ok = false;
        break;
      }
      chosen = e;
    }
  if (!chosen)
    ok = false;
  long lo = 0, hi = 0, current = 0;
  if (ok) {
    int rangeResult =
        input ? snd_mixer_selem_get_capture_volume_range(chosen, &lo, &hi)
              : snd_mixer_selem_get_playback_volume_range(chosen, &lo, &hi);
    if (rangeResult < 0 || hi <= lo)
      ok = false;
  }
  std::vector<std::pair<snd_mixer_selem_channel_id_t, long>> channels;
  if (ok)
    for (int c = 0; c <= SND_MIXER_SCHN_LAST; ++c) {
      auto channel = static_cast<snd_mixer_selem_channel_id_t>(c);
      long v;
      if (!(input ? snd_mixer_selem_has_capture_channel(chosen, channel)
                  : snd_mixer_selem_has_playback_channel(chosen, channel)))
        continue;
      if ((input ? snd_mixer_selem_get_capture_volume(chosen, channel, &v)
                 : snd_mixer_selem_get_playback_volume(chosen, channel, &v)) <
          0) {
        ok = false;
        break;
      }
      channels.push_back(std::make_pair(channel, v));
      current = std::max(current, v - lo);
    }
  if (channels.empty())
    ok = false;
  if (ok && !set)
    value = 100.0 * current / (hi - lo);
  if (ok && set)
    for (const auto &c : channels) {
      long v =
          lo + std::lround((hi - lo) * value / 100.0 *
                           (current ? double(c.second - lo) / current : 1.0));
      if ((input
               ? snd_mixer_selem_set_capture_volume(chosen, c.first, v)
               : snd_mixer_selem_set_playback_volume(chosen, c.first, v)) < 0) {
        ok = false;
        break;
      }
    }
  snd_mixer_close(m);
  return ok;
}
// Only accept ALSA aliases whose evaluated configuration is a direct Pulse
// plugin with no explicit server or device. Environment overrides are refused
// because they can route the stream somewhere other than the server default.
bool pulseAlias(const std::string &uid, bool input) {
  if (uid != "default" && uid != "pulse")
    return false;
  const char *overrideName = std::getenv(input ? "PULSE_SOURCE" : "PULSE_SINK");
  if (overrideName && *overrideName)
    return false;
  snd_config_t *root = nullptr, *pcm = nullptr, *item = nullptr;
  if (snd_config_update_ref(&root) < 0)
    return false;
  bool ok = snd_config_search_definition(root, "pcm", uid.c_str(), &pcm) >= 0;
  const char *type = nullptr;
  ok = ok && snd_config_search(pcm, "type", &item) >= 0 &&
       snd_config_get_string(item, &type) >= 0 &&
       std::strcmp(type, "pulse") == 0;
  for (const char *key : {"device", "server"}) {
    if (ok && snd_config_search(pcm, key, &item) >= 0) {
      const char *value = nullptr;
      if (snd_config_get_string(item, &value) < 0 || (value && *value))
        ok = false;
    }
  }
  if (pcm)
    snd_config_delete(pcm);
  snd_config_unref(root);
  return ok;
}
NrVolcErrorType volume(bool input, const std::string &uid, bool set,
                       double &v) {
  if (set && !valid(v))
    return NRVOLC_ERROR;
  bool ok;
  if (uid.empty() || uid.compare(0, 6, "pulse:") == 0) {
    ok = pulseVolume(input, uid, set, v);
  } else if (pulseAlias(uid, input)) {
    ok = pulseVolume(input, "", set, v);
  } else {
    ok = hardware(input, uid, set, v);
  }
  return ok ? NRVOLC_NO_ERROR : NRVOLC_ERROR;
}
} // namespace
NrVolcErrorType
NrVolumeChangerLinuxImpl::getDeviceList(std::map<std::string, std::string> &out,
                                        NRVOLC::DeviceType type) const {
  if (type != NRVOLC::INPUT_DEVICE && type != NRVOLC::OUTPUT_DEVICE &&
      type != NRVOLC::ANY_DEVICE)
    return NRVOLC_ERROR;
  Pulse p;
  if (!p.connect())
    return NRVOLC_ERROR;
  std::map<std::string, std::string> result;
  for (int input = 0; input <= 1; ++input) {
    if ((input && type == NRVOLC::OUTPUT_DEVICE) ||
        (!input && type == NRVOLC::INPUT_DEVICE))
      continue;
    p.devices.clear();
    if (!list(p, input))
      return NRVOLC_ERROR;
    for (const Device &d : p.devices) {
      std::string id = std::string(prefix(input)) + d.name;
      result[d.description + " [" + id + "]"] = id;
    }
  }
  out.swap(result);
  return NRVOLC_NO_ERROR;
}
NrVolcErrorType NrVolumeChangerLinuxImpl::setDefaultInputVolume(double v) {
  return volume(true, "", true, v);
}
NrVolcErrorType
NrVolumeChangerLinuxImpl::getDefaultInputVolume(double &v) const {
  return volume(true, "", false, v);
}
NrVolcErrorType NrVolumeChangerLinuxImpl::setDefaultOutputVolume(double v) {
  return volume(false, "", true, v);
}
NrVolcErrorType
NrVolumeChangerLinuxImpl::getDefaultOutputVolume(double &v) const {
  return volume(false, "", false, v);
}
NrVolcErrorType
NrVolumeChangerLinuxImpl::setInputDeviceVolume(const std::string &id,
                                               double v) {
  return id.empty() ? NRVOLC_ERROR : volume(true, id, true, v);
}
NrVolcErrorType
NrVolumeChangerLinuxImpl::getInputDeviceVolume(const std::string &id,
                                               double &v) const {
  return id.empty() ? NRVOLC_ERROR : volume(true, id, false, v);
}
NrVolcErrorType
NrVolumeChangerLinuxImpl::setOutputDeviceVolume(const std::string &id,
                                                double v) {
  return id.empty() ? NRVOLC_ERROR : volume(false, id, true, v);
}
NrVolcErrorType
NrVolumeChangerLinuxImpl::getOutputDeviceVolume(const std::string &id,
                                                double &v) const {
  return id.empty() ? NRVOLC_ERROR : volume(false, id, false, v);
}
