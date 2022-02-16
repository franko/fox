calc_sources = [
'Calculator.cpp',
'HelpWindow.cpp',
'Preferences.cpp',
'help.cpp',
'main.cpp',
]

calc_icons = [
'colors.gif',
'information.gif',
'bigcalc.gif',
'constmem.bmp',
'question.gif',
'tinycalc.gif',
]

calc_icons_h = custom_target('calc-icons-h',
    input : calc_icons,
    output : 'icons.h',
    command : [reswrap, '-h', '-o', '@OUTPUT@', '@INPUT@']
)

calc_icons_cpp = custom_target('calc-icons-cpp',
    input : calc_icons,
    output : 'icons.cpp',
    command : [reswrap, '-i', calc_icons_h, '-o', '@OUTPUT@', '@INPUT@']
)

calc_sources += calc_icons_cpp

executable('calculator', calc_sources,
    dependencies: libfox_dep,
    include_directories: fox_topdir_include,
    install : true, gui_app : true
)

install_man('calculator.1')
