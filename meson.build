project(
  'exasm', 'cpp',
  default_options : ['warning_level=3', 'cpp_std=c++20'],
  version : '1.0.0',
)

if get_option('cpp_std') != 'c++20'
  error('Exasm requires C++20')
endif

subdir('src')
