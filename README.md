# nrvolchanger

A small C++ library for reading and setting input and output device volume on
macOS, Windows and Linux. The public API is in `lib/VolumeChanger.h`.

## Linux

Ubuntu 24.04 or later requires a PulseAudio-compatible sound server (PulseAudio
or PipeWire's PulseAudio service) for server-routed devices. Install the build
dependencies with:

```sh
sudo apt install g++ pkg-config libasound2-dev libpulse-dev qt6-base-dev
mkdir -p /tmp/nrvolc-build && cd /tmp/nrvolc-build
qmake6 /path/to/nrvolchanger/lib/lib.pro CONFIG+=release
make -j4
```

The core library does not use Qt at runtime. qmake copies the library and public
headers to `lib/last_build`. Build the optional Qt tester with qmake against
`tester/tester.pro` after building the library; build both in debug mode for the
debug tester. The tester may need `LD_LIBRARY_PATH` set to `lib/last_build`.

Default volume methods follow the current Pulse server default sink or source.
Device enumeration returns server sinks and non-monitor sources. Its map keys
are display labels with the unique ID appended, and values are opaque IDs of
form `pulse:sink:<server-name>` and `pulse:source:<server-name>`. Re-enumerate
after audio devices change. Explicit IDs select that server endpoint even if the
default changes. Removing a device causes operations on its old ID to fail.

VDK's ALSA PortAudio backend also supplies names of the form
`<card name>: <PCM name> (hw:C,D)` or `(plughw:C,D)`. These streams can bypass
the sound server, so changing a Pulse endpoint would not necessarily change
what the stream hears. For these identifiers the library verifies the current
ALSA card name, PCM name and direction, and uses an ALSA mixer volume only if
there is exactly one PCM in that direction on the card and exactly one active
volume control in that direction. It returns an error for ambiguous or missing
controls. Card indexes can change; re-enumerate after hardware changes. Hardware
volume does not guarantee control of a particular audio path on every driver.
Direct hardware behavior requires validation on the target device.

The PortAudio aliases `default` and `pulse` are accepted only when the evaluated
ALSA PCM configuration specifies a direct Pulse plugin with no explicit server
or device and the direction's `PULSE_SINK`/`PULSE_SOURCE` override is unset.
Other aliases, nested plugins, and native PipeWire ALSA routes are unsupported;
they return an error instead of assuming a default route. ALSA config hooks are
executed by ALSA when evaluating aliases and are outside the Pulse operation
timeout. Default-volume API calls refer to Pulse defaults regardless of ALSA
configuration.

Setters accept finite percentages from 0 to 100, preserve mute and relative
channel levels, and return an error when a device or control is unavailable.
Getters report the largest channel as a percentage; Pulse can report values
above 100. Failed getters leave their output unchanged. Pulse operations use a
fresh connection and a two-second overall deadline. No physical hardware is
required for the isolated server integration test:

```sh
sudo apt install pulseaudio pulseaudio-utils
./tests/linux-integration.sh
```

The integration test starts a private PulseAudio server and virtual endpoints;
it does not validate physical ALSA controls or audio output.

To test the Pulse-compatible PipeWire service in a private session, install
`pipewire`, `pipewire-pulse`, `wireplumber`, `pulseaudio-utils`, and `dbus-x11`,
then run `dbus-run-session -- ./tests/pipewire-integration.sh`.
