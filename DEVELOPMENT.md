# SIDFactoryII Development

## Building from source

Clone the repository with git: the build number shown in the editor is made from the date and
hash of the last commit.

    git clone https://github.com/Chordian/sidfactory2.git
    cd sidfactory2

Every platform puts its build in an `artifacts` folder (`macos/artifacts` on macOS). The executable needs the `config`
(or `config.ini`), `drivers`, `overlay` and `color_schemes` resources in its folder, the build
copies them there.

### Windows

Prerequisites:

- [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) (the Community edition is
  fine) or the Build Tools for Visual Studio 2022, with the workload "Desktop development with
  C++" (MSVC v143 toolset and a Windows 10 or 11 SDK)
- git

SDL2 (`libs/SDL2-2.32.10`) and libusb (`libs/libusb`) come with the repository, nothing else
needs to be installed.

To build from the command line, open the "Developer Command Prompt for VS 2022" (it puts
`MSBuild.exe` on the path), go to the root folder of the repository and run:

    build_windows.bat

This builds the `Release|x86` configuration of `SIDFactoryII.sln` and collects the executable,
`SDL2.dll`, `config.ini`, the drivers, music, overlays, color schemes and documentation in
`artifacts`. Start `artifacts\SIDFactoryII.exe`. The script stops when the `artifacts` folder
already exists: remove or rename it before building again.

To build in the Visual Studio IDE, open `SIDFactoryII.sln`, select the `Release` (or `Debug`)
configuration and the `x86` platform and build the solution. The executable lands in
`Release\` (or `Debug\`); run `build_windows.bat` once to collect everything in `artifacts`, or copy
`SDL2.dll`, `config.ini` and the `drivers`, `overlay` and `color_schemes` folders from
`SIDFactoryII\` into the folder of the executable.

USBSID-Pico output needs the WinUSB driver on the "USBSID-Pico Data" interface of the board.
Install it once with [Zadig](https://zadig.akeo.ie): Options, List All Devices, select
"USBSID-Pico Data", select WinUSB and press Install Driver (or Replace Driver).

### macOS

Prerequisites:

- Xcode command line tools: `xcode-select --install`
- [Homebrew](https://brew.sh)
- git, gnu-sed, librsvg (`rsvg-convert` for the icon) and pandoc (the keys document):

      brew install git gnu-sed librsvg pandoc

SDL2 (`macos/App/Contents/Frameworks/SDL2.framework`) and libusb (`libs/libusb`) come with the
repository. To build:

    cd macos
    make raw

This compiles a universal binary (x86_64 for macOS 10.11 and up, arm64 for macOS 11 and up),
packages `SIDFactoryII.app` and writes a read-write disk image
`artifacts/SIDFactoryII_macOS_<build>-RAW.dmg`. For a release, mount it, lay out the window by
hand, unmount it and run `make dmg` for the final compressed image. Other targets in
`macos/Makefile`:

- `make app`: only the universal `artifacts/SIDFactoryII.app`
- `make universal`: only the universal executable `artifacts/SIDFactoryII`
- `make clean`: remove the objects and the `artifacts` folder

USBSID-Pico output needs no driver installation on macOS.

A 'linux-style' binary for debugging is built from the root folder with the Homebrew SDL2 and
libusb instead:

    brew install sdl2 libusb pkg-config
    make PLATFORM=MACOS

Add `TARGET=DEBUG` for a build without optimization. `make PLATFORM=MACOS debug` starts the
binary in `lldb`, `make PLATFORM=MACOS run` starts it with a demo tune.

### Linux

Install the compiler, SDL2, ALSA, libusb and pkg-config.

Debian, Ubuntu and derivatives:

    sudo apt-get update
    sudo apt-get install g++ make git libsdl2-dev libasound2-dev libusb-1.0-0-dev pkg-config

Fedora:

    sudo dnf install gcc-c++ make git SDL2-devel alsa-lib-devel libusb1-devel pkgconf-pkg-config

Arch Linux and derivatives:

    sudo pacman -S --needed gcc make git sdl2 alsa-lib libusb pkgconf

openSUSE:

    sudo zypper install gcc-c++ make git SDL2-devel alsa-devel libusb-1_0-devel pkgconf-pkg-config

To build and run from the root folder:

    make
    cd artifacts && ./SIDFactoryII

`make` builds `artifacts/SIDFactoryII` with the resources in the same folder, `make run` starts it with a
demo tune. For a distribution folder `artifacts/SIDFactoryII_LINUX_ALSA_<build>` with the
stripped executable, music and documentation:

    make dist

Options, passed on the `make` command line:

- `TARGET=DEBUG`: no optimization, for debugging (`make debug` starts the binary in `lldb`)
- `LINUXAUDIO=JACK`: MIDI (ASID) through JACK instead of ALSA. Install the JACK development
  package instead of the ALSA one (`libjack-jackd2-dev` on Debian/Ubuntu,
  `pipewire-jack-audio-connection-kit-devel` on Fedora,
  `pipewire-jack` or `jack2` on Arch, `libjack-devel` on openSUSE):

      make LINUXAUDIO=JACK dist

Run `make clean` after changing `TARGET` or `LINUXAUDIO`, objects are not rebuilt for a changed
option.

`make ubuntu` builds `make dist` in an Ubuntu 26.04 Docker container (see `Dockerfile`) and
copies the `artifacts` folder out of it.

USBSID-Pico output needs read and write access to the USB device of the board. Install the udev
rule [`69-usbsid-permissions.rules`](https://github.com/LouDnl/USBSID-Pico/blob/master/examples/udev-rules/69-usbsid-permissions.rules)
in `/etc/udev/rules.d` and reload the rules:

    sudo udevadm control --reload-rules && sudo udevadm trigger

## External dependencies

### PicoPNG

PicoPNG files were copied from https://lodev.org/lodepng
A small change was made to `picopng.h` in relation to [issue 134](https://github.com/Chordian/sidfactory2/issues/134)

### USBSID-Pico driver

`SIDFactoryII/source/libraries/usbsid` holds unmodified copies of `USBSID.cpp`,
`USBSID.h`, `USBSID_Manager.cpp` and `USBSID_Manager.h` from
https://github.com/LouDnl/USBSID-Pico-driver (`src/`). Update by copying the
files again, never patch them in place.

The driver needs libusb-1.0 and POSIX threads:

- Linux: system libusb-1.0 through pkg-config.
- macOS: `libs/libusb` (vendored libusb 1.0.30 subset) is compiled into the
  binary by `macos/Makefile`.
- Windows: `libs/libusb` is compiled into the binary by the Visual Studio
  project, `libs/usbsid-windows/pthread.h` maps the pthread calls of the
  driver to Win32. The board needs the WinUSB driver on its
  "USBSID-Pico Data" interface (install with [Zadig](https://zadig.akeo.ie)).

### Multi SID drivers

`SIDFactoryII/drivers/sf2driver11_05_2sid.prg`, `_3sid.prg` and `_4sid.prg` are built from one
KickAssembler source derived from driver 11.05 (`sf2driver11_05_multisid.a`, argument `:sids=2`,
`3` or `4`). The editor takes the number of SIDs from the track count of the driver (three tracks
per SID) and expects SID n at `$D400 + n * $20`.

### Stereo panning and SID v5 export

`utils/sidpanning` computes the stereo position of every SID from the panning layout and mode of
the SID file format v5, as listed in the tables of the format description. The panning of a tune is
stored in the hardware preferences block of the `.sf2` (version 2, three bytes appended to the two
of version 1). `PSIDFile` writes the v5 header and the song length table. The song lengths come from
`DriverUtils::GetSongLengthInMilliseconds`: it plays the song on a copy of the memory until every
track has fetched an order list entry a second time, or the driver state reports a stop.

### Licences of bundled code

| Code | Location | Licence |
|---|---|---|
| reSID-fp | `SIDFactoryII/source/libraries/residfp` | GPL-2.0-or-later |
| RtMidi | `SIDFactoryII/source/libraries/rtmidi` | MIT style |
| USBSID-Pico driver | `SIDFactoryII/source/libraries/usbsid` | GPL-3.0-or-later, see `LICENSE` and `LICENSE-EXCEPTION` there |
| libusb (macOS, Windows) | `libs/libusb` | LGPL-2.1-or-later, see `COPYING` there |
| pthread layer for Windows | `libs/usbsid-windows` | GPL-3.0-or-later |

SID Factory II ships the GPL version 2 text in `SIDFactoryII/COPYING` without naming a
version in its sources. Section 9 of that licence then allows any published GPL version. A
build that contains the USBSID-Pico driver is distributed under GPL version 3 or later.

## Releases and nightly builds

There are two sets of binaries:

- Official releases
- Nightly automatic builds of the master branch

Official releases:

- Are publicly announced
- Are supported (bugs and feature requests are welcomed)
- Are tested
- Are manually prepared
- Have an up-to-date manual

Nightly builds:

- Are not publicly announced
- Are not tested extensively
- Are "beta" versions
- Are automatically built from the current master branch
- Are not supported; if someone experiences issues with a nightly build, he/she
  could be asked to revert back to an official release.
- Could have an out of date manual

Nightly builds can be downloaded by anyone from the Github project. They can
be used by beta testers, for example by people that requested a certain feature
or bugfix and can evaluate it before it is being officially released.

The dev team can also be beta testers by making sure they use the latest nightly
version ("eat your own dogfood").

Apart from releases and nightly builds, an 'alpha' version can be built by the
dev team from a particular branch, to be tested before it becomes part of the
nightly builds.

## Branching and merging

The 'master' branch should be a stable branch. The nightly 'beta' versions built
from the master branch should be usable and a potential candidate for an
official release.

Bugfixes and features are developed in separate branches. These branches are
tested and reviewed before they are merged into master. Only minor, low risk
changes are done directly in master (spelling errors etc.).

### Merge requests

Once approved, a MR is merged to master. It should be complete so that no
further work is necessary in the master branch, so:

- Basic functionality is tested (with the latest master branch merged into it)
- The changelog in `README.md` is updated
- Config file changes are documented in `config.ini` and `user.default.ini`
- Key mappings that have changed are documented in `notes.txt`
- The user manual (Word document) is **not** updated, because it is a binary
  file that can not be automatically merged.
- Build files (Makefiles/Windows batch file) are updated when needed.

## Releases

For official releases

- The manual is updated in the master branch
- The release candidate is more thorougly tested
- The release is officially announced
- The release is given a git tag

## Issue tracking

Anyone can file a feature request or bug. The team will evaluate if it should go
on the "to do" list and at what priority. There are no guarantees if and when an
issue will be addressed.
