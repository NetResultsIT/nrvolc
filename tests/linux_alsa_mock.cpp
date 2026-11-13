// Linker-wrapped ALSA fixture exercises the production hardware path without
// requiring kernel sound devices. The mixer and its control share one handle.
#include "../lib/NrVolumeChangerLinux.cpp"
#include <cassert>
#include <cerrno>
#include <cstring>

namespace {
snd_mixer_t *mixer = reinterpret_cast<snd_mixer_t *>(0x10);
snd_hctl_t *hctl = reinterpret_cast<snd_hctl_t *>(0x20);
snd_ctl_t *ctl = reinterpret_cast<snd_ctl_t *>(0x30);
snd_mixer_elem_t *elem1 = reinterpret_cast<snd_mixer_elem_t *>(0x40);
snd_mixer_elem_t *elem2 = reinterpret_cast<snd_mixer_elem_t *>(0x50);
long channels[] = {90, 50};
int pcmCount = 1, controls = 1, nextError = 0, pcmError = 0;
bool readError = false, writeError = false, rangeError = false;
const char *cardName = "Card";
snd_pcm_stream_t stream;
int writes = 0, captureWrites = 0;
} // namespace
extern "C" {
int __wrap_snd_mixer_open(snd_mixer_t **out, int) {
  *out = mixer;
  return 0;
}
int __wrap_snd_mixer_close(snd_mixer_t *m) {
  assert(m == mixer);
  return 0;
}
int __wrap_snd_mixer_attach(snd_mixer_t *m, const char *id) {
  assert(m == mixer && !strcmp(id, "hw:0"));
  return 0;
}
int __wrap_snd_mixer_get_hctl(snd_mixer_t *m, const char *, snd_hctl_t **out) {
  assert(m == mixer);
  *out = hctl;
  return 0;
}
snd_ctl_t *__wrap_snd_hctl_ctl(snd_hctl_t *h) {
  assert(h == hctl);
  return ctl;
}
int __wrap_snd_ctl_card_info(snd_ctl_t *c, snd_ctl_card_info_t *) {
  assert(c == ctl);
  return 0;
}
const char *__wrap_snd_ctl_card_info_get_name(const snd_ctl_card_info_t *) {
  return cardName;
}
int __wrap_snd_ctl_pcm_info(snd_ctl_t *c, snd_pcm_info_t *p) {
  assert(c == ctl);
  unsigned device = snd_pcm_info_get_device(p);
  stream = snd_pcm_info_get_stream(p);
  if (pcmError && device == 1)
    return -EIO;
  return device < static_cast<unsigned>(pcmCount) ? 0 : -ENOENT;
}
const char *__wrap_snd_pcm_info_get_name(const snd_pcm_info_t *) {
  return stream == SND_PCM_STREAM_PLAYBACK ? "Card Speaker" : "Card Mic";
}
int __wrap_snd_ctl_pcm_next_device(snd_ctl_t *c, int *device) {
  assert(c == ctl);
  if (nextError)
    return -EIO;
  *device = *device + 1 < pcmCount ? *device + 1 : -1;
  return 0;
}
int __wrap_snd_mixer_selem_register(snd_mixer_t *,
                                    struct snd_mixer_selem_regopt *,
                                    snd_mixer_class_t **) {
  return 0;
}
int __wrap_snd_mixer_load(snd_mixer_t *) { return 0; }
snd_mixer_elem_t *__wrap_snd_mixer_first_elem(snd_mixer_t *) {
  return controls ? elem1 : nullptr;
}
snd_mixer_elem_t *__wrap_snd_mixer_elem_next(snd_mixer_elem_t *e) {
  return e == elem1 && controls > 1 ? elem2 : nullptr;
}
int __wrap_snd_mixer_selem_is_active(snd_mixer_elem_t *) { return 1; }
int __wrap_snd_mixer_selem_has_playback_volume(snd_mixer_elem_t *) { return 1; }
int __wrap_snd_mixer_selem_has_capture_volume(snd_mixer_elem_t *) { return 1; }
int __wrap_snd_mixer_selem_has_playback_channel(
    snd_mixer_elem_t *, snd_mixer_selem_channel_id_t c) {
  return c == 0 || c == 1;
}
int __wrap_snd_mixer_selem_has_capture_channel(snd_mixer_elem_t *,
                                               snd_mixer_selem_channel_id_t c) {
  return c == 0 || c == 1;
}
int __wrap_snd_mixer_selem_get_playback_volume_range(snd_mixer_elem_t *,
                                                     long *lo, long *hi) {
  *lo = 10;
  *hi = 110;
  return rangeError ? -EIO : 0;
}
int __wrap_snd_mixer_selem_get_capture_volume_range(snd_mixer_elem_t *e,
                                                    long *lo, long *hi) {
  return __wrap_snd_mixer_selem_get_playback_volume_range(e, lo, hi);
}
int __wrap_snd_mixer_selem_get_playback_volume(snd_mixer_elem_t *,
                                               snd_mixer_selem_channel_id_t c,
                                               long *v) {
  if (readError)
    return -EIO;
  *v = channels[c];
  return 0;
}
int __wrap_snd_mixer_selem_get_capture_volume(snd_mixer_elem_t *e,
                                              snd_mixer_selem_channel_id_t c,
                                              long *v) {
  return __wrap_snd_mixer_selem_get_playback_volume(e, c, v);
}
int __wrap_snd_mixer_selem_set_playback_volume(snd_mixer_elem_t *,
                                               snd_mixer_selem_channel_id_t c,
                                               long v) {
  if (writeError)
    return -EIO;
  ++writes;
  channels[c] = v;
  return 0;
}
int __wrap_snd_mixer_selem_set_capture_volume(snd_mixer_elem_t *e,
                                              snd_mixer_selem_channel_id_t c,
                                              long v) {
  ++captureWrites;
  return __wrap_snd_mixer_selem_set_playback_volume(e, c, v);
}
}
int main() {
  const std::string uid = "Card: Speaker (hw:0,0)";
  double v = 0;
  assert(hardware(false, uid, false, v) && v == 80);
  // PortAudio formats duplex names using the final playback query, even for
  // input.
  assert(hardware(true, uid, false, v) && v == 80);
  v = 40;
  assert(hardware(false, uid, true, v));
  assert(channels[0] == 50 && channels[1] == 30 && writes == 2);
  assert(hardware(true, uid, false, v) && v == 40);
  v = 60;
  assert(hardware(true, uid, true, v));
  assert(channels[0] == 70 && channels[1] == 40 && writes == 4 &&
         captureWrites == 2);
  assert(hardware(true, uid, false, v) && v == 60);
  cardName = "Replacement";
  v = 1234;
  assert(!hardware(false, uid, false, v) && v == 1234);
  cardName = "Card";
  pcmCount = 2;
  v = 40;
  assert(!hardware(false, uid, true, v) && writes == 4);
  v = 1234;
  pcmError = 1;
  assert(!hardware(false, uid, false, v) && v == 1234);
  pcmError = 0;
  pcmCount = 1;
  controls = 2;
  assert(!hardware(false, uid, false, v) && v == 1234);
  controls = 1;
  nextError = 1;
  assert(!hardware(false, uid, false, v) && v == 1234);
  nextError = 0;
  rangeError = true;
  assert(!hardware(false, uid, false, v) && v == 1234);
  rangeError = false;
  readError = true;
  assert(!hardware(false, uid, false, v) && v == 1234);
  readError = false;
  writeError = true;
  v = 20;
  assert(!hardware(false, uid, true, v) && writes == 4);
}
