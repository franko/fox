
adie_sources = [
  'Adie.cpp',
  'Commands.cpp',
  'FindInFiles.cpp',
  'HelpWindow.cpp',
  'main.cpp',
  'Modeline.cpp',
  'Preferences.cpp',
  'ShellCommand.cpp',
  'Syntax.cpp',
  'SyntaxParser.cpp',
  'TextWindow.cpp',
]

adie_icons = [
  'adie_gif.gif',
  'backward_gif.gif',
  'big_gif.gif',
  'bookdel_gif.gif',
  'booknext_gif.gif',
  'bookprev_gif.gif',
  'bookset_gif.gif',
  'browser.gif',
  'capitalize.gif',
  'close_gif.gif',
  'colors_gif.gif',
  'config_gif.gif',
  'copy_gif.gif',
  'cut_gif.gif',
  'delete_gif.gif',
  'delimit_gif.gif',
  'filehidden.gif',
  'fileshown.gif',
  'fonts_gif.gif',
  'forward_gif.gif',
  'goto_gif.gif',
  'help_gif.gif',
  'indent_gif.gif',
  'info_gif.gif',
  'lang_gif.gif',
  'logger.gif',
  'lowercase.gif',
  'miscellaneous_gif.gif',
  'new_gif.gif',
  'nobrowser.gif',
  'nologger.gif',
  'open_gif.gif',
  'palette_gif.gif',
  'pattern_gif.gif',
  'paste_gif.gif',
  'print_gif.gif',
  'quit_gif.gif',
  'redo_gif.gif',
  'reload_gif.gif',
  'replace_gif.gif',
  'saveall_gif.gif',
  'saveas_gif.gif',
  'saveto_gif.gif',
  'save_gif.gif',
  'search_gif.gif',
  'searchicon_gif.gif',
  'searchnext_gif.gif',
  'searchprev_gif.gif',
  'shiftleft_gif.gif',
  'shiftright_gif.gif',
  'small_gif.gif',
  'styles_gif.gif',
  'syntax_gif.gif',
  'undo_gif.gif',
  'uppercase.gif',
]

adie_icons_h = custom_target('adie-icons-h',
    input : adie_icons,
    output : 'icons.h',
    command : [reswrap, '-h', '-o', '@OUTPUT@', '@INPUT@']
)

adie_icons_cpp = custom_target('adie-icons-cpp',
    input : adie_icons,
    output : 'icons.cpp',
    command : [reswrap, '-i', adie_icons_h, '-o', '@OUTPUT@', '@INPUT@']
)

adie_sources += adie_icons_cpp

adie_help_h = custom_target('adie-help-h',
    input : 'help.txt',
    output : 'help.h',
    command : [reswrap, '-h', '-a', '-o', '@OUTPUT@', '@INPUT@']
)

adie_help_cpp = custom_target('adie-help-cpp',
    input : 'help.txt',
    output : 'help.cpp',
    command : [reswrap, '-i', adie_help_h, '-a', '-o', '@OUTPUT@', '@INPUT@']
)

adie_sources += adie_help_cpp

executable('adie', adie_sources,
    dependencies: libfox_dep,
    include_directories: fox_topdir_include,
    install : true, gui_app : true
)

install_man('adie.1')
install_data('Adie.stx', install_dir : join_paths(get_option('bindir')))
