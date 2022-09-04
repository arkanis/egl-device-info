EGL device info
===============

A small Linux command line utility to inspect the available headless EGL devices and their properties (EGL client extensions, EGL device extensions, EGL display extensions and OpenGL extensions).

Usage
-----

- Clone the repo and run `make egl-device-info` to compile the utility. It only needs `gcc` and `make` installed.
- Run `./egl-device-info`. This tries to create a headless OpenGL 4.5 context on all available devices and displays the properties of each one.
- In case you're getting `eglCreateContext()` errors you can change the OpenGL version via the `--opengl-version` command line argument, e.g. `./egl-device-info --opengl-version 3.3`.