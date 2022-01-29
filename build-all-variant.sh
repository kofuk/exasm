#!/usr/bin/env bash
set -eu

_dir="$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)"

set -x
    meson setup "${_dir}/_build/default" --cross-file cross/wasm.txt \
          -Dhtml_install_dir="public/"
    meson install -C "${_dir}/_build/default"
set +x

for variant in $(meson configure | sed -n '/Project options/,$p' |
                     grep ex_inst | cut -d\  -f3); do
    variant_short_name="${variant#ex_inst_}"
    set -x
    meson setup "_build/${variant}" --cross-file cross/wasm.txt \
          -Dhtml_install_dir="public/${variant_short_name}" \
          -D"${variant}"=enabled
    meson install -C "${_dir}/_build/${variant}"
    set +x
done
