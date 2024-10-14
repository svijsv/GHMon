uHAL
======

uHAL is a small-ish hardware abstraction layer written in C99. It doesn't do
much and it doesn't do it very well, but it's mine.

At present it supports the STM32F103, STM32F401, the ATTiny402 but can
be fairly easily extended to support other devices in those families.

API documentation can be found in `Documentation/html/index.html` if I remembered
to build it. Otherwise it can be built with `./tools/build_docs.sh` if I remembered
to write that.


Configuration
======
There are template configuration files in `./config`.

Two configuration files are required:
* The core configuration file, named `config_uHAL.h`
* The platform configuration file, the name of which is defined in the macro
`uHAL_PLATFORM_CONFIG` (e.g. with `-D uHAL_PLATFORM_CONFIG=\"config_STM32F103.h\"`)


Building
======
Building requires ulib, my more-poorly-documented general-purpose library.

FatFS (http://elm-chan.org/fsw/ff/) is an optional dependency.

The uHAL root directory, the location(s) of the configuration files, and the
parent of the ulib directory must be in the C include search path.

The macro `uHAL_PLATFORM` must be set to the name of the platform - that is, the
correct subdirectory of `uHAL/src/platform` (e.g. `AVR_XMEGA3`).

The subdirectory of `uHAL/src/platform` matching `uHAL_PLATFORM` must be included
in the build and all the others must be excluded. If using PlatformIO, this is
handled by the included `library.json`.

As far as getting the tool chain set up, you're on your own.

Usage
======
Include the file `uHAL/include/interface.h` in the main program. This will pull in
the configuration files and any enabled subsystems.

There's a test program in `./test` which may be of help.
