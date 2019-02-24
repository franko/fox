
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
    include_directories: fox_include,
    dependencies: thread_dep,
    link_with: libfox,
    gui_app: true)
