# exasm

Assembler & emulator for pipelined processor I creating at experiment.

[![deploy](https://github.com/kofuk/exasm/actions/workflows/deploy.yaml/badge.svg)](https://github.com/kofuk/exasm/actions/workflows/deploy.yaml)

## Building

This project uses Meson to configure build system.
You need install meson regardless of the frontend you choose.

Ubuntu example:

```shell
$ sudo apt install python3 ninja-build
$ pip3 install meson
```

(On Ubuntu, APT version of Meson may be too old).

### Web interface (recommended)

```shell
$ meson . _build -Dhtml_install_dir=/opt/exasm --cross-file cross/wasm.txt
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

```shell
$ meson . _build && cd _build
$ ninja
```
