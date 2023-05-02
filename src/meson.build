bd2_p2_library = library('bd2_p2',
                           'attribute.cpp',
                           'base_relation.cpp',
                           'database.cpp',
                           'file_relation.cpp',
                           'memory_relation.cpp',
                           'predicates.cpp',
                           link_with : [],
                           include_directories : inc)

parser_library = library('parser',
                     'parser.cpp',
                     link_with : [],
                     include_directories : inc)

main_exe = executable('main', 'main.cpp',
                      link_with : [bd2_p2_library, parser_library],
                      include_directories : inc)
