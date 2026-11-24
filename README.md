# Qwertickle

A typewriter sound scheme for your keyboard.

This branch modernizes the original Qwertickle project for current Linux
systems. The interface uses GTK 3 and Ayatana AppIndicator, audio playback
uses GStreamer 1.x, and keyboard events are read from a user-selected Linux
input event device (evdev). This allows it to work under Wayland compositors
such as Niri; it no longer relies on XRecord/X11.

## Sound behavior

Qwertickle uses the two original audio samples recovered from the original
Windows Qwertick executable:

- `embedded-audio-1-ulaw-44100.wav`: the standard key-click sound.
- `embedded-audio-2-pcm-22050.wav`: the Enter key sound.

All keys use the standard click, including Space and Backspace. Only Enter
(and keypad Enter) plays the distinct Enter sound.

## Dependencies on Arch Linux

```sh
sudo pacman -S --needed base-devel autoconf automake libtool pkgconf gtk3 \
  libayatana-appindicator gstreamer gst-plugins-base gst-plugins-good
```

## Build

```sh
./autogen.sh
./configure
make
```

Run the program with:

```sh
./src/qwertickle
```

On first launch, choose the keyboard event device (usually a path under
`/dev/input/`). The application needs read permission for that device.
Choose only your own keyboard device: while enabled, Qwertickle can observe
the keys pressed on that device in order to play the corresponding sounds.

## Credits and license

Original Qwertickle by Wei-Ning Huang (AZ). The program code is distributed
under the GNU General Public License, version 2 or (at your option) any later
version, as stated in the source files.
