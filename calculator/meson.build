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
    command : [reswrap, '-i', '-o', '@OUTPUT@', '@INPUT@']
)

calc_icons_cpp = custom_target('calc-icons-cpp',
    input : calc_icons,
    output : 'icons.cpp',
    command : [reswrap, '-e', '-o', '@OUTPUT@', '@INPUT@']
)

calc_sources += calc_icons_cpp

executable('calculator', calc_sources,
    dependencies: libfox_dep, install : true, gui_app : true
)

install_man('calculator.1')
