# SIDFactoryII Development

## Building from source

### Windows

Prerequisites:

- Visual Studio

To build:

    build_windows.bat

### macOS

Prerequisites:

- XCode command line tools
- git
- gnu-sed
- pandoc

If [Homebrew](https://brew.sh) is installed, most prerequisites can be installed with:

    brew git gnu-sed pandoc

To build:

    cd macos
    make raw

This creates a DMG image in the `artifacts` folder.

### Linux (Ubuntu)

Install prerequisites:

    apt-get update
    apt-get install g++ make git libsdl2-dev

To build:

    make dist

Then look in the `artifacts` folder.

## External dependencies

### PicoPNG

PicoPNG files were copied from https://lodev.org/lodepng
A small change was made to `picopng.h` in relation to [issue 134](https://github.com/Chordian/sidfactory2/issues/134)

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
from the master branch should be useable and a potential candidate for an
official release.

Bugfixes and features are developed in separate branches. These branches are
tested and reviewed before they are merged into master. Only minor, low risk
changes are done directly in master (spelling errors etc.).

### Merge requests

Once approved, a MR is merged to master. It should be complete so that no
further work is neccesary in the master branch, so:

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
