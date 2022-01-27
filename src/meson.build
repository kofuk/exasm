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
  asm_runner = executable('exasm_test_runner', 'exasm_test.cc', link_with : asmio_lib)
  test('ASM register arith', asm_runner,
       args : ['../tests/asm/reg_arith_simple.in',
               '../tests/asm/reg_arith_simple.out'])
  test('ASM immediates', asm_runner,
       args : ['../tests/asm/imm_simple.in',
               '../tests/asm/imm_simple.out'])
  test('ASM signed immediates', asm_runner,
       args : ['../tests/asm/imm_sign.in',
               '../tests/asm/imm_sign.out'])
  test('ASM branch', asm_runner,
       args : ['../tests/asm/branch_simple.in',
               '../tests/asm/branch_simple.out'])
  test('ASM mem', asm_runner,
       args : ['../tests/asm/mem_simple.in',
               '../tests/asm/mem_simple.out'])

  if get_option('ex_inst_t').enabled()
    test('ASM extension T', asm_runner,
         args : ['../tests/asm/ex_t.in',
                 '../tests/asm/ex_t.out'])
  endif

  emu_runner = executable('exemu_test_runner', 'exemu_test.cc', link_with : [asmio_lib, emulator_lib])
  test('EMU register arith', emu_runner,
       args : ['../tests/emu/reg_arith.in',
               '../tests/emu/reg_arith.out'])
  test('EMU immediate arith', emu_runner,
       args : ['../tests/emu/imm_arith.in',
               '../tests/emu/imm_arith.out'])
  test('EMU branch', emu_runner,
       args : ['../tests/emu/branch.in',
               '../tests/emu/branch.out'])
  test('EMU mem', emu_runner,
       args : ['../tests/emu/mem.in',
               '../tests/emu/mem.out'])

  if get_option('ex_inst_t').enabled()
    test('EMU extension T', emu_runner,
         args : ['../tests/emu/ex_t.in',
                 '../tests/emu/ex_t.out'])
  endif
endif
