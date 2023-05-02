catch2_tests_exe = executable('main',
                              'main.cpp',
                              'parser.cpp',
                              'potpourri.cpp',
                              link_with : [bd2_p2_library,
                                           parser_library],
                              include_directories : inc,
                              dependencies : [])
test('BD2-P2 tests', catch2_tests_exe)
