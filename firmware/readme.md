
[Doxygen](https://rusefi.com/docs/html/)

[Q&A on source code](https://rusefi.com/forum/viewtopic.php?f=5&t=10)

See also [../unit_tests](../unit_tests)

This directory contains the source code for the RusEFI firmware.

The ideal is that typical end users should be able to use pre-built
firmware.  They should not need to modify or even rebuild from the
source code for basic use, but building from the source code provides
the opportunity for optimization, supporting unexpected engine
configurations, and specialized enhancements.


TL;DR

make PROJECT_BOARD=microrusefi PROJECT_CPU=ARCH_STM32F4


Environment

Rebuilding from source code requires this firmware, a modern C/C++
compiler for embedded ARM systems, and a platform that supports 'make'
based builds.

While many compilers have the potential to work, we suggest using the
official ARM version of GCC available at launchpad.net.

Linux and MacOS systems should have the software development tools,
primarily 'make', pre-installed or readily installed.  MS-Windows
requires selecting and installing a Unix-compatible system environment.

Note that the developers are volunteers, with varied motivations.
These motivations often include using leading-edge language and build
system concepts, requiring recent versions of tools.  Should you
encounter build problems, review the latest version of this document.


Expected Future Changes

The firmware build is moving toward a system that separates board
features from processor features.  This will require specifying both
the board type and specific processor.

The existing system evolved based on the original RusEFI boards.  Those
used 'STM32 Discovery' development boards plugged into base boards
that held the ECU-specific chips.  That approach resulted in hard-coded
assumption about pin assignments, and associations of hardware with a
specific processor variant.  That legacy is slowly being cleaned up,
but is still evident in some settings and limitations.
