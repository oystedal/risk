project(
  'risk',
  'cpp',
  version : '0.0.1',
  default_options : ['cpp_std=c++17'],
)

includes = include_directories(
)

gtest = dependency('gtest', required : false)

compiler = meson.get_compiler('cpp')

# executable(
#   'risk_server', 
#   'main.cpp',
#   
#   include_directories: [
#   ],
#   dependencies: [
#   ],
#   cpp_args: [
#     '-std=c++17',
#     '-Wno-unused-parameter',
#     '-Wno-unused-function',
#     '-Wno-unused-label',
#     '-Wpointer-arith',
#     '-Wformat',
#     '-Wreturn-type',
#     '-Wsign-compare',
#     '-Wmultichar',
#     # '-Wformat-nonliteral',
#     '-Winit-self',
#     '-Wuninitialized',
#     '-Wno-deprecated',
#     '-Wformat-security',
#   ],
# )

test_exe = executable(
  'tests',
  'test/test_main.cpp',
  'test/test.cpp',
  dependencies : [
    gtest,
  ]
)
test('tests', test_exe)
