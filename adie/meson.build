
adie_sources = [
'Adie.cpp',
'Commands.cpp',
'help.cpp',
'HelpWindow.cpp',
'Hilite.cpp',
'icons.cpp',
'main.cpp',
'parsesyntax.cpp',
'Preferences.cpp',
'TextWindow.cpp'
]

executable('adie', adie_sources,
    dependencies: libfox_dep,
    install : true,
    gui_app: true
)

install_man('adie.1')
install_data('Adie.stx', install_dir : join_paths(get_option('bindir')))
