
adie_sources = [
  'Adie.cpp',
  'Commands.cpp',
  'FindInFiles.cpp',
  'HelpWindow.cpp',
  'main.cpp',
  'Modeline.cpp',
  'Preferences.cpp',
  'ShellCommand.cpp',
  'ShellDialog.cpp',
  'Syntax.cpp',
  'SyntaxParser.cpp',
  'TextWindow.cpp',
]

adie_icons = [
  'adie_gif.gif',
  'backward_gif.gif',
  'big_gif.gif',
  'bookclr2_gif.gif',
  'bookdel2_gif.gif',
  'bookmrk2_gif.gif',
  'booknxt2_gif.gif',
  'bookprv2_gif.gif',
  'bookset2_gif.gif',
  'browser.gif',
  'capitalize.gif',
  'close_gif.gif',
  'colors_gif.gif',
  'command_bmp.bmp',
  'config_gif.gif',
  'copy_gif.gif',
  'cut_gif.gif',
  'delete_gif.gif',
  'delimit_gif.gif',
  'docs_gif.gif',
  'export_gif.gif',
  'express_gif.gif',
  'filehidden_gif.gif',
  'fileshown_gif.gif',
  'filter_gif.gif',
  'fonts_gif.gif',
  'forward_gif.gif',
  'goto_gif.gif',
  'help_gif.gif',
  'import_gif.gif',
  'indent_gif.gif',
  'info_gif.gif',
  'lang_gif.gif',
  'logger.gif',
  'lowercase.gif',
  'new_gif.gif',
  'newfile_gif.gif',
  'nobrowser.gif',
  'nologger.gif',
  'nowrap_gif.gif',
  'miscellaneous_gif.gif',
  'open_gif.gif',
  'opensel_gif.gif',
  'palette_gif.gif',
  'pattern_gif.gif',
  'paste_gif.gif',
  'point_gif.gif',
  'point2_gif.gif',
  'print_gif.gif',
  'quit_gif.gif',
  'redo_gif.gif',
  'reload_gif.gif',
  'replace_gif.gif',
  'save_gif.gif',
  'saveall_gif.gif',
  'saveas_gif.gif',
  'saveto_gif.gif',
  'script_gif.gif',
  'search_gif.gif',
  'search_rex.gif',
  'search_norex.gif',
  'search_case.gif',
  'search_nocase.gif',
  'search_word.gif',
  'search_noword.gif',
  'search_up.gif',
  'search_dn.gif',
  'searchicon_gif.gif',
  'searchnext_gif.gif',
  'searchprev_gif.gif',
  'searchfiles_gif.gif',
  'shiftleft_gif.gif',
  'shiftright_gif.gif',
  'small_gif.gif',
  'styles_gif.gif',
  'syntax_gif.gif',
  'terminal_gif.gif',
  'undo_gif.gif',
  'uppercase.gif',
  'wordwrap_gif.gif',
  'switch_gif.gif',
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
    include_directories : [fox_include, fox_topdir_include],
    dependencies : fox_deps,
    link_with : libfox,
    cpp_args : fox_defines,
    install : true, gui_app : true
)

install_man('adie.1')
install_data('Adie.stx', install_dir : join_paths(get_option('bindir')))
