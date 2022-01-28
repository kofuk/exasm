asmio_lib = static_library('asmio', 'asmio.cc')
emulator_lib = static_library('emulator', 'emulator.cc')

if host_machine.system() == 'emscripten'
  fs = import('fs')

  if fs.is_absolute(get_option('html_install_dir'))
    html_install_dir = get_option('html_install_dir')
  else
    html_install_dir = meson.project_source_root() / get_option('html_install_dir')
  endif

  emcc = find_program('emcc')

  emcc_gen_html_flags = [
    '-s', 'WASM=1',
    '-s', 'NO_EXIT_RUNTIME=1',
    '-s', 'EXPORTED_RUNTIME_METHODS=["ccall"]',
    '-s', 'EXPORTED_FUNCTIONS=["_free"]',
    '-s', 'NO_DISABLE_EXCEPTION_CATCHING',
  ]

  custom_target(
    output : ['index.html', 'index.js', 'index.wasm'],
    input : ['wasm_shell_webemu.html', 'webemu.cc'],
    command : [
      emcc, emcc_gen_html_flags,
      '-o', '@OUTPUT0@', '--shell-file', '@INPUT@',
      asmio_lib, emulator_lib,
    ],
    install : true,
    install_dir : html_install_dir,
  )

  install_data('bcralnit.js', install_dir : html_install_dir)
else
  executable(
    'as', 'exasm.cc',
    link_with : asmio_lib,
    install : true,
  )
  executable(
    'emu', 'exemu.cc',
    link_with : [emulator_lib, asmio_lib],
    install: true,
  )
endif

if meson.can_run_host_binaries()
  asm_runner = executable('exasm_test_runner', 'exasm_test.cc', link_with : asmio_lib)
  test('ASM register arith', asm_runner,
       args : files('../tests/asm/y_reg_arith_simple.in',
                    '../tests/asm/y_reg_arith_simple.out'))
  test('ASM immediates', asm_runner,
       args : files('../tests/asm/y_imm_simple.in',
                    '../tests/asm/y_imm_simple.out'))
  test('ASM signed immediates', asm_runner,
       args : files('../tests/asm/y_imm_sign.in',
                    '../tests/asm/y_imm_sign.out'))
  test('ASM branch', asm_runner,
       args : files('../tests/asm/y_branch_simple.in',
                    '../tests/asm/y_branch_simple.out'))
  test('ASM mem', asm_runner,
       args : files('../tests/asm/y_mem_simple.in',
                    '../tests/asm/y_mem_simple.out'))
  test('ASM robustness', asm_runner,
       args : files('../tests/asm/y_syntax_robustness.in',
                    '../tests/asm/y_syntax_robustness.out'))
  test('ASM missing operand', asm_runner,
       should_fail : true,
       args : files('../tests/asm/n_missing_operand.in',
                    '../tests/asm/n_missing_operand.out'))
  test('ASM operand comment', asm_runner,
       should_fail : true,
       args : files('../tests/asm/n_operand_comment.in',
                    '../tests/asm/n_operand_comment.out'))
  test('ASM invalid operand 1', asm_runner,
       should_fail : true,
       args : files('../tests/asm/n_invalid_operand.in',
                    '../tests/asm/n_invalid_operand.out'))
  test('ASM invalid operand 2', asm_runner,
       should_fail : true,
       args : files('../tests/asm/n_invalid_operand_2.in',
                    '../tests/asm/n_invalid_operand_2.out'))
  test('ASM invalid immediate', asm_runner,
       should_fail : true,
       args : files('../tests/asm/n_invalid_imm.in',
                    '../tests/asm/n_invalid_imm.out'))

  if get_option('ex_inst_t').enabled()
    test('ASM extension T', asm_runner,
         args : files('../tests/asm/y_ex_t.in',
                      '../tests/asm/y_ex_t.out'))
  endif

  emu_runner = executable('exemu_test_runner', 'exemu_test.cc', link_with : [asmio_lib, emulator_lib])
  test('EMU register arith', emu_runner,
       args : files('../tests/emu/y_reg_arith.in',
                    '../tests/emu/y_reg_arith.out'))
  test('EMU immediate arith', emu_runner,
       args : files('../tests/emu/y_imm_arith.in',
                    '../tests/emu/y_imm_arith.out'))
  test('EMU branch', emu_runner,
       args : files('../tests/emu/y_branch.in',
                    '../tests/emu/y_branch.out'))
  test('EMU mem', emu_runner,
       args : files('../tests/emu/y_mem.in',
                    '../tests/emu/y_mem.out'))

  if get_option('ex_inst_t').enabled()
    test('EMU extension T', emu_runner,
         args : files('../tests/emu/y_ex_t.in',
                      '../tests/emu/y_ex_t.out'))
  endif
endif
