shutterbug_sources = [
'ShutterBug.cpp',
'Snapper.cpp'
]

shutterbug_icons = [
'line_0.gif',
'line_1.gif',
'line_2.gif',
'line_3.gif',
'line_4.gif',
'line_5.gif',
'line_6.gif',
'line_7.gif',
'line_8.gif',
'shutterbug.gif',
'tinyshutterbug.gif',
]

shutterbug_icons_h = custom_target('shutterbug-icons-h',
    input : shutterbug_icons,
    output : 'icons.h',
    command : [reswrap, '-h', '-o', '@OUTPUT@', '@INPUT@']
)

shutterbug_icons_cpp = custom_target('shutterbug-icons-cpp',
    input : shutterbug_icons,
    output : 'icons.cpp',
    command : [reswrap, '-i', shutterbug_icons_h, '-o', '@OUTPUT@', '@INPUT@']
)

shutterbug_sources += shutterbug_icons_cpp

executable('shutterbug', shutterbug_sources,
    dependencies: libfox_dep,
    include_directories: fox_topdir_include,
    install : true, gui_app : true
)

install_man('shutterbug.1')
