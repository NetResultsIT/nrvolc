#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
"$root/tests/linux-alsa-mock.sh"
tmp=$(mktemp -d)
trap 'test -z "${server_pid:-}" || kill "$server_pid" 2>/dev/null || :; rm -rf "$tmp"' EXIT HUP INT TERM
c++ -std=c++11 -Wall -Wextra -I"$root/lib" "$root/tests/linux_helpers.cpp" "$root/lib/VolumeChanger.cpp" $(pkg-config --cflags --libs alsa libpulse) -o "$tmp/helpers"
"$tmp/helpers"
c++ -std=c++11 -Wall -Wextra -I"$root/lib" "$root/tests/linux_integration.cpp" "$root/lib/NrVolumeChangerLinux.cpp" "$root/lib/VolumeChanger.cpp" $(pkg-config --cflags --libs alsa libpulse) -o "$tmp/test"
export PULSE_SERVER="unix:$tmp/native"
export ALSA_CONFIG_PATH="$tmp/asound.conf"
printf '%s\n' 'pcm.default { type pulse }' 'pcm.pulse { type pulse }' > "$ALSA_CONFIG_PATH"
# A private, anonymous local server: no host audio state is touched.
pulseaudio -n --daemonize=no --exit-idle-time=-1 --log-target=stderr \
  -L "module-native-protocol-unix socket=$tmp/native auth-anonymous=1" \
  -L "module-null-sink sink_name=nrvolc_a sink_properties=device.description=Duplicate" \
  -L "module-null-sink sink_name=nrvolc_b sink_properties=device.description=Duplicate" \
  -L "module-remap-source master=nrvolc_a.monitor source_name=nrvolc_source" \
  -L "module-remap-source master=nrvolc_b.monitor source_name=nrvolc_source_b" >"$tmp/server.log" 2>&1 &
server_pid=$!
for n in 1 2 3 4 5 6 7 8 9 10; do pactl info >/dev/null 2>&1 && break; sleep 1; done
pactl set-default-sink nrvolc_a
pactl set-default-source nrvolc_source
pactl set-sink-volume nrvolc_a 80% 40%
pactl set-sink-mute nrvolc_a 1
"$tmp/test"
printf '%s\n' 'pcm.default { type hw card 0 }' 'pcm.pulse { type pulse device "other" }' > "$ALSA_CONFIG_PATH"
"$tmp/test" aliasbad
printf '%s\n' 'pcm.default { type pulse }' 'pcm.pulse { type pulse }' > "$ALSA_CONFIG_PATH"
pactl get-sink-volume nrvolc_a | grep -q '40%.*20%'
pactl get-sink-mute nrvolc_a | grep -q 'yes'
pactl set-sink-volume nrvolc_b 125%
pactl set-source-volume nrvolc_source_b 65%
pactl set-default-sink nrvolc_b
pactl set-default-source nrvolc_source_b
# Unload the first sink (and its remapped source) to exercise stale IDs.
id=$(pactl list short modules | awk '$2 == "module-null-sink" && $0 ~ /sink_name=nrvolc_a/ {print $1}')
pactl unload-module "$id"
"$tmp/test" switched
kill "$server_pid"
wait "$server_pid" || :
server_pid=
"$tmp/test" offline

# Hold connections open without speaking the Pulse protocol. This tests the
# operation deadline rather than merely connection refusal.
rm -f "$tmp/native"
cat > "$tmp/stall.py" <<'PYTHON'
import socket, sys
sock = socket.socket(socket.AF_UNIX)
sock.bind(sys.argv[1])
sock.listen(20)
connections = []
while True:
    connection, _ = sock.accept()
    connections.append(connection)
PYTHON
python3 "$tmp/stall.py" "$tmp/native" &
server_pid=$!
for n in 1 2 3 4 5; do test -S "$tmp/native" && break; sleep 1; done
timeout 15 "$tmp/test" offline
kill "$server_pid"
wait "$server_pid" || :
server_pid=
rm -f "$tmp/native"
pulseaudio -n --daemonize=no --exit-idle-time=-1 --log-target=stderr \
  -L "module-native-protocol-unix socket=$tmp/native auth-anonymous=1" \
  -L "module-null-sink sink_name=reconnected" >"$tmp/server.log" 2>&1 &
server_pid=$!
for n in 1 2 3 4 5 6 7 8 9 10; do pactl info >/dev/null 2>&1 && break; sleep 1; done
pactl set-default-sink reconnected
"$tmp/test" reconnected
