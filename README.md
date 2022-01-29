# exasm

Assembler & emulator for pipelined processor I creating at experiment.

[![deploy](https://github.com/kofuk/exasm/actions/workflows/deploy.yaml/badge.svg)](https://github.com/kofuk/exasm/actions/workflows/deploy.yaml)

## Building

This project uses Meson to configure build system.
You need install Meson and Ninja regardless of the frontend you choose.

**Note**: On Ubuntu, APT package of Meson is too old and fails to configure
this project. If you encountered the problem, try installing Meson with `pip`.

### Web interface (recommended)

Web interface is implemented by calling C++ code (compiled to WebAssembly) from JavaScript.
You need install Emscripten on your system.

```shell
$ meson . _build -Dhtml_install_dir=/opt/exasm --cross-file cross/wasm.txt && cd _build
$ ninja install
```

You can change `html_install_dir` to somewhere for your preference.

You'll need HTTP server which serves installed directory to run
on web browser (https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing).

### Command line tool

Since developing useful command line tool is not goal of this project,
command line interface has limited functionality.

If you'd like to use assembler in automated build, command line assembler
may be a good choice. Otherwise, you should build web interface instead.

You need C++ compiler installed on your system.

```shell
$ meson . _build && cd _build
$ ninja
```
