
adie_sources = [
'Adie.cpp',
'Commands.cpp',
'help.cpp',
'HelpWindow.cpp',
'Hilite.cpp',
'main.cpp',
'parsesyntax.cpp',
'Preferences.cpp',
'TextWindow.cpp'
]

icons = [
'adie_gif.gif',
'big_gif.gif',
'bookdel_gif.gif',
'booknext_gif.gif',
'bookprev_gif.gif',
'bookset_gif.gif',
'close_gif.gif',
'colors_gif.gif',
'copy_gif.gif',
'cut_gif.gif',
'delete_gif.gif',
'delimit_gif.gif',
'fonts_gif.gif',
'help_gif.gif',
'indent_gif.gif',
'info_gif.gif',
'lang_gif.gif',
'new_gif.gif',
'open_gif.gif',
'palette_gif.gif',
'pattern_gif.gif',
'paste_gif.gif',
'print_gif.gif',
'quit_gif.gif',
'redo_gif.gif',
'reload_gif.gif',
'saveall_gif.gif',
'saveas_gif.gif',
'save_gif.gif',
'search_gif.gif',
'searchnext_gif.gif',
'searchprev_gif.gif',
'shiftleft_gif.gif',
'shiftright_gif.gif',
'small_gif.gif',
'styles_gif.gif',
'syntax_gif.gif',
'undo_gif.gif'
]

icons_h = custom_target('icons.h',
    input : icons,
    output : 'icons.h',
    command : [reswrap, '-i', '-o', '@OUTPUT@', '@INPUT@']
)

icons_cpp = custom_target('icons.cpp',
    input : icons,
    output : 'icons.cpp',
    command : [reswrap, '-e', '-o', '@OUTPUT@', '@INPUT@']
)

adie_sources += icons_cpp

executable('adie', adie_sources,
    dependencies: libfox_dep,
    install : true,
    gui_app: true
)

install_man('adie.1')
install_data('Adie.stx', install_dir : join_paths(get_option('bindir')))
