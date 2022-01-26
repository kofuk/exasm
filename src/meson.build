asmio_lib = static_library('asmio', 'asmio.cc')

executable('as', 'exasm.cc', link_with : asmio_lib)

emcc = find_program('emcc')
webas = custom_target(
  output : ['webas.html'],
  input : ['wasm_shell.html', 'webas.cc', 'asmio.cc'],
  command : [
    emcc, '-s', 'WASM=1', '-s', 'NO_EXIT_RUNTIME=1',
    '-s', 'EXPORTED_RUNTIME_METHODS=["ccall"]',
    '-s', 'EXPORTED_FUNCTIONS=["_free"]',
    '-o', '@OUTPUT@', '--shell-file', '@INPUT@',
  ],
  install : true,
  install_dir : '/opt/exasm',
)
