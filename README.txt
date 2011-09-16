This project implements a GPU-based global illumination solution based
on Photon Mapping and Screen-Space Ray Tracing.

REQUIREMENTS
============
The build system has only been configured for working on a UNIX system,
though support for Microsoft Windows could be easily added (using
pthread-win32).
Until now, the project has only been tested on Ubuntu Linux running an
NVIDIA GeForce 8600M GT with the drivers 256.35

A functional OpenGL 3.3+ implementation is required for this project
(for NVIDIA products, this means at least a GPU from the GeForce 8xxx
series is required). This corresponds to DirectX 10 level hardware.

The project relies on the following installed software:
- SCons build system (relies on Python)
- pthread
- GLFW: the version used is SVN r1202. GLFW 2.6 is not suitable for this
project as it does not support forward-compatible OpenGL context
creation. Latest SVN versions of GLFW are not suitable either because
of the removal of the support for the <GL3/gl3.h> header.

COMPILATION INSTRUCTIONS
========================
Just type "scons" in a terminal in the root of the project.
For spawning N tasks, add the "-jN" option (ex: "scons -j2" speeds
up the compilation for a dual-core CPU).

24th August 2010
Lionel FUENTES
funto66 -at- gmail -dot- com
