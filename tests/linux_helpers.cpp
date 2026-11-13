// Exercise the same strict parser and PortAudio display-name conversion used
// by the backend, without requiring a physical ALSA card.
#include "../lib/NrVolumeChangerLinux.cpp"
#include <cassert>
int main() {
  assert(shortPcm("USB Audio", "USB Audio") == "-");
  assert(shortPcm("USB Audio - Headset", "USB Audio") == "Headset");
  assert(shortPcm("USB Audio: Speaker", "USB Audio") == "Audio: Speaker");
  assert(shortPcm("HDMI 0", "Other Card") == "HDMI 0");
  size_t pos;
  int card, device;
  assert(parseHardwareUid("Card: PCM (hw:12,3)", pos, card, device));
  assert(card == 12 && device == 3);
  assert(parseHardwareUid("Card: PCM (plughw:12,3)", pos, card, device));
  assert(!parseHardwareUid("Card: PCM (hw:01,3)", pos, card, device));
  assert(!parseHardwareUid("Card: PCM (hw:+1,3)", pos, card, device));
  assert(!parseHardwareUid("Card: PCM (hw:999999999999999999,3)", pos, card, device));
  assert(!parseHardwareUid("Card: PCM (hw:1,3) trailing", pos, card, device));
}
