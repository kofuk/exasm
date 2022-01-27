asmio_lib = static_library('asmio', 'asmio.cc')
emulator_lib = static_library('emulator', 'emulator.cc')

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

if meson.can_run_host_binaries()
  test_runner = executable('exasm_test_runner', 'exasm_test.cc', link_with : asmio_lib)

  test('register arith', test_runner,
       args : ['../tests/asm/reg_arith_simple.in',
               '../tests/asm/reg_arith_simple.out'])
  test('immediates', test_runner,
       args : ['../tests/asm/imm_simple.in',
               '../tests/asm/imm_simple.out'])
  test('signed immediates', test_runner,
       args : ['../tests/asm/imm_sign.in',
               '../tests/asm/imm_sign.out'])
  test('branch', test_runner,
       args : ['../tests/asm/branch_simple.in',
               '../tests/asm/branch_simple.out'])
  test('mem', test_runner,
       args : ['../tests/asm/mem_simple.in',
               '../tests/asm/mem_simple.out'])

  if get_option('ex_inst_t').enabled()
    test('extension T', test_runner,
         args : ['../tests/asm/ex_t.in',
                 '../tests/asm/ex_t.out'])
  endif
endif
