asmio_lib = static_library('asmio', 'asmio.cc', cpp_args : '-DEXTEND_T')
emulator_lib = static_library('emulator', 'emulator.cc', cpp_args : '-DEXTEND_T')

if host_machine.system() == 'emscripten'
  empp = find_program('em++')

  empp_gen_html_flags = [
    '-s', 'WASM=1',
    '-s', 'NO_EXIT_RUNTIME=1',
    '-s', 'EXPORTED_RUNTIME_METHODS=["ccall"]',
    '-s', 'EXPORTED_FUNCTIONS=["_free"]',
    '-s', 'NO_DISABLE_EXCEPTION_CATCHING',
  ]

  custom_target(
    output : ['webas.html'],
    input : ['wasm_shell_webas.html', 'webas.cc'],
    command : [
      empp, empp_gen_html_flags,
      '-o', '@OUTPUT@', '--shell-file', '@INPUT@',
      asmio_lib,
    ],
    install : true,
    install_dir : '/opt/exasm',
  )

  custom_target(
    output : ['webemu.html'],
    input : ['wasm_shell_webemu.html', 'webemu.cc'],
    command : [
      empp, empp_gen_html_flags,
      '-o', '@OUTPUT@', '--shell-file', '@INPUT@',
      asmio_lib, emulator_lib,
    ],
    install : true,
    install_dir : '/opt/exasm',
  )

  install_data('bcralnit.js', install_dir : '/opt/exasm')
else
  executable('as', 'exasm.cc', link_with : asmio_lib)
  executable('emu', 'exemu.cc', link_with : [emulator_lib, asmio_lib])
endif
