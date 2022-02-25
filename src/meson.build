insts = files('../isa/basic.json', '../isa/extra.json')
if get_option('ex_inst_t').enabled()
  insts += files('../isa/ex_t.json')
endif
if get_option('ex_inst_12').enabled()
  insts += files('../isa/ex_12.json')
endif

python = find_program('python3')

inst_type_enum_inc = custom_target(
  output : ['inst_type_enum.inc'],
  input : ['generate_inst_type_enum.py', insts],
  command : [python, '@INPUT@', '@OUTPUT@']
)
inst_name_to_enum_inc = custom_target(
  output : ['inst_name_to_enum.inc'],
  input : ['generate_inst_name_to_enum.py', insts],
  command : [python, '@INPUT@', '@OUTPUT@']
)
asm_parser_inc = custom_target(
  output : ['asm_parser.inc'],
  input : ['generate_asm_parser.py', insts],
  command : [python, '@INPUT@', '@OUTPUT@']
)
executor_inc = custom_target(
  output : ['executor.inc'],
  input : ['generate_executor.py', insts],
  command : [python, '@INPUT@', '@OUTPUT@']
)
asm_writer_inc = custom_target(
  output : ['asm_writer.inc'],
  input : ['generate_asm_writer.py', insts],
  command : [python, '@INPUT@', '@OUTPUT@']
)
inst_traits_inc = custom_target(
  output : ['inst_traits.inc'],
  input : ['generate_inst_traits.py', insts],
  command : [python, '@INPUT@', '@OUTPUT@']
)
decoder_inc = custom_target(
  output : ['decoder.inc'],
  input : ['generate_decoder.py', insts],
  command : [python, '@INPUT@', '@OUTPUT@']
)
encoder_inc = custom_target(
  output : ['encoder.inc'],
  input : ['generate_encoder.py', insts],
  command : [python, '@INPUT@', '@OUTPUT@']
)

asmio_lib = static_library(
  'asmio', 'asmio.cc',
  inst_type_enum_inc, inst_name_to_enum_inc,
  asm_parser_inc, asm_writer_inc, inst_traits_inc,
  decoder_inc, encoder_inc,
)
emulator_lib = static_library(
  'emulator', 'emulator.cc',
  inst_type_enum_inc, executor_inc,
)

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

  if get_option('cpp_std') != 'none'
    emcc_gen_html_flags += '-std=@0@'.format(get_option('cpp_std'))
  endif

  custom_target(
    output : ['index.html', 'index.js', 'index.wasm'],
    input : ['web/wasm_shell_webemu.html', 'webemu.cc'],
    command : [
      emcc, emcc_gen_html_flags,
      '-o', '@OUTPUT0@', '--shell-file', '@INPUT@',
      asmio_lib, emulator_lib,
    ],
    install : true,
    install_dir : html_install_dir,
  )

  install_data('web/driver.js', install_dir : html_install_dir)
  install_data('web/style.css', install_dir : html_install_dir)
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
  asm_testcases = [
    'y_reg_arith_simple', 'y_imm_simple', 'y_branch_simple', 'y_mem_simple', 'y_syntax_robustness',
    'n_missing_operand', 'n_operand_comment', 'n_invalid_operand', 'n_invalid_operand_2',
    'n_invalid_imm', 'n_label_duplicate', 'y_label_simple', 'y_jump_to_label', 'y_long_jump_forward',
    'y_long_jump_forward_2', 'y_long_jump_forward_3', 'y_long_jump_forward_4',
    'y_long_jump_backward', 'y_long_jump_backward_2', 'y_long_jump_backward_3',
    'y_long_jump_backward_4', 'y_missing_delay_slot', 'y_pseudo_li_simple', 'y_pseudo_li_arith',
    'y_pseudo_li_arith_2', 'y_pseudo_li_large_num', 'y_raw_data',
  ]
  emu_testcases = [
    'y_reg_arith', 'y_imm_arith', 'y_branch', 'y_mem', 'y_break_simple', 'n_unaligned_word_access',
    'n_unaligned_word_access', 'y_reverse_after_branch'
  ]

  if get_option('ex_inst_t').enabled()
    asm_testcases += ['y_ex_t']
    emu_testcases += ['y_ex_t']
  endif

  asm_runner = executable('exasm_test_runner', 'exasm_test.cc', link_with : asmio_lib)
  foreach t : asm_testcases
    test('ASM @0@'.format(t), asm_runner,
         should_fail : t.startswith('n_'),
         args : files('../tests/asm/@0@.in'.format(t),
                      '../tests/asm/@0@.out'.format(t)))
  endforeach

  emu_runner = executable('exemu_test_runner', 'exemu_test.cc', link_with : [asmio_lib, emulator_lib])
  foreach t : emu_testcases
    test('EMU @0@'.format(t), emu_runner,
         should_fail : t.startswith('n_'),
         args : files('../tests/emu/@0@.in'.format(t),
                      '../tests/emu/@0@.op'.format(t),
                      '../tests/emu/@0@.out'.format(t)))
  endforeach
endif
