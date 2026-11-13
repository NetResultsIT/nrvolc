#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
wraps='snd_mixer_open snd_mixer_close snd_mixer_attach snd_mixer_get_hctl snd_hctl_ctl snd_ctl_card_info snd_ctl_card_info_get_name snd_ctl_pcm_info snd_pcm_info_get_name snd_ctl_pcm_next_device snd_mixer_selem_register snd_mixer_load snd_mixer_first_elem snd_mixer_elem_next snd_mixer_selem_is_active snd_mixer_selem_has_playback_volume snd_mixer_selem_has_capture_volume snd_mixer_selem_has_playback_channel snd_mixer_selem_has_capture_channel snd_mixer_selem_get_playback_volume_range snd_mixer_selem_get_capture_volume_range snd_mixer_selem_get_playback_volume snd_mixer_selem_get_capture_volume snd_mixer_selem_set_playback_volume snd_mixer_selem_set_capture_volume'
flags=''
for symbol in $wraps; do flags="$flags -Wl,--wrap=$symbol"; done
c++ -std=c++11 -Wall -Wextra -I"$root/lib" "$root/tests/linux_alsa_mock.cpp" "$root/lib/VolumeChanger.cpp" $flags $(pkg-config --cflags --libs alsa libpulse) -o "$tmp/test"
"$tmp/test"
