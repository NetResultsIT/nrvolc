#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'kill ${pw_pid:-} ${pulse_pid:-} ${wp_pid:-} 2>/dev/null || :; rm -rf "$tmp"' EXIT HUP INT TERM
export XDG_RUNTIME_DIR="$tmp/runtime"
mkdir -m 700 "$XDG_RUNTIME_DIR"
export PULSE_SERVER="unix:$XDG_RUNTIME_DIR/pulse/native"
export ALSA_CONFIG_PATH="$tmp/asound.conf"
printf '%s\n' 'pcm.default { type pulse }' 'pcm.pulse { type pulse }' > "$ALSA_CONFIG_PATH"
c++ -std=c++11 -Wall -Wextra -I"$root/lib" "$root/tests/linux_integration.cpp" "$root/lib/NrVolumeChangerLinux.cpp" "$root/lib/VolumeChanger.cpp" $(pkg-config --cflags --libs alsa libpulse) -o "$tmp/test"
pipewire >"$tmp/pipewire.log" 2>&1 & pw_pid=$!
wireplumber >"$tmp/wireplumber.log" 2>&1 & wp_pid=$!
pipewire-pulse >"$tmp/pulse.log" 2>&1 & pulse_pid=$!
for n in 1 2 3 4 5 6 7 8 9 10; do pactl info >/dev/null 2>&1 && break; sleep 1; done
pactl load-module module-null-sink sink_name=nrvolc_a sink_properties=device.description=Duplicate
pactl load-module module-null-sink sink_name=nrvolc_b sink_properties=device.description=Duplicate
pactl load-module module-remap-source master=nrvolc_a.monitor source_name=nrvolc_source
pactl load-module module-remap-source master=nrvolc_b.monitor source_name=nrvolc_source_b
pactl set-default-sink nrvolc_a
pactl set-default-source nrvolc_source
pactl set-sink-volume nrvolc_a 80% 40%
pactl set-sink-mute nrvolc_a 1
"$tmp/test"
pactl get-sink-volume nrvolc_a | grep -q '40%.*20%'
pactl get-sink-mute nrvolc_a | grep -q 'yes'
pactl set-sink-volume nrvolc_b 125%
pactl set-source-volume nrvolc_source_b 65%
pactl set-default-sink nrvolc_b
pactl set-default-source nrvolc_source_b
id=$(pactl list short modules | awk '$2 == "module-null-sink" && $0 ~ /sink_name=nrvolc_a/ {print $1}')
pactl unload-module "$id"
"$tmp/test" switched
