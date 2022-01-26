asmio_lib = static_library('asmio', 'asmio.cc')
emulator_lib = static_library('emulator', 'emulator.cc')

executable('as', 'exasm.cc', link_with : asmio_lib)
executable('emu', 'exemu.cc', link_with : [emulator_lib, asmio_lib])

emcc = find_program('emcc')
webas = custom_target(
  output : ['webas.html'],
  input : ['wasm_shell_webas.html', 'webas.cc', 'asmio.cc'],
  command : [
    emcc, '-s', 'WASM=1', '-s', 'NO_EXIT_RUNTIME=1',
    '-s', 'EXPORTED_RUNTIME_METHODS=["ccall"]',
    '-s', 'EXPORTED_FUNCTIONS=["_free"]',
    '-s', 'NO_DISABLE_EXCEPTION_CATCHING',
    '-o', '@OUTPUT@', '--shell-file', '@INPUT@',
  ],
  install : true,
  install_dir : '/opt/exasm',
)
