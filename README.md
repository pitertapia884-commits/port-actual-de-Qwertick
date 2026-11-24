# Qwertickle

A Linux typewriter-sound utility inspired by **Qwertick for Windows**.

This repository contains the original Linux Qwertickle code and a modernized
Linux port. The actively developed port is maintained on the
[`gtk-modern-port` branch](https://github.com/pitertapia884-commits/port-actual-de-Qwertick/tree/gtk-modern-port).

The modern port updates the application for current Linux desktops:
GTK 3, Ayatana AppIndicator, GStreamer 1.x, and keyboard input through
Linux evdev. It is designed to work with Wayland compositors, including Niri.

## Sound behavior in the modern port

The modern port uses two audio samples recovered from the original Windows
Qwertick executable:

- `embedded-audio-1-ulaw-44100.wav`: standard key click.
- `embedded-audio-2-pcm-22050.wav`: distinct Enter sound.

All keys use the standard click, including Space and Backspace. Only Enter
(and keypad Enter) uses the distinct sound.

## Build (modern port)

Switch to the modern port branch:

```sh
git switch gtk-modern-port
```

On Arch Linux, install the dependencies:

```sh
sudo pacman -S --needed base-devel autoconf automake libtool pkgconf gtk3 \
  libayatana-appindicator gstreamer gst-plugins-base gst-plugins-good
```

Build and run:

```sh
./autogen.sh
./configure
make
./src/qwertickle
```

On first launch, select your own keyboard event device under `/dev/input/`.
Qwertickle needs permission to read that device so it can detect key presses
and play the corresponding sounds.

## Credits and acknowledgements

This project builds on the work of the original creators:

- **Nattyware** — creator of [Qwertick for Windows](http://www.nattyware.com/qwertick.php), the original application that inspired this project. The recovered audio samples also originate from its executable.
- **Wei-Ning Huang (AZ)** — creator of the original Linux **Qwertickle** project. The Linux port uses his code as its starting point and retains the applicable notices from that project.

### Port and contributions

The modern Linux port and the changes in the `gtk-modern-port` branch are
maintained by **@pitertapia884-commits**. Contributions include the GTK 3 and
Ayatana AppIndicator update, GStreamer 1.x integration, evdev keyboard input,
Wayland compatibility, and integration of the recovered original audio
samples with the key behavior described above.

The port is an independent community project and is not affiliated with or
endorsed by Nattyware.

## Copyright and licensing

The original Linux Qwertickle source files identify Wei-Ning Huang (AZ) as
their author and state that the program code is licensed under the
**GNU General Public License, version 2 or (at your option) any later version**.
Please retain the original copyright and license notices when redistributing
or modifying that code. See [`COPYING`](COPYING).

The audio samples are separate from the Linux program code. They were
recovered from the original Qwertick Windows executable; their copyright
and any distribution permissions remain with their respective rights holder.
This repository does not claim ownership of those sounds or grant a separate
license for them. Obtain permission from the rights holder before redistributing
the audio files.

## Contributing

Bug reports, fixes, and improvements are welcome. Please open an issue or
submit a pull request against the appropriate branch.
