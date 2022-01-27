project('exasm', 'cpp', default_options : ['warning_level=3'])

if get_option('ex_inst_t').enabled()
  add_project_arguments('-DEXTEND_T', language : 'cpp')
endif

subdir('src')
