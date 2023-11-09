pathfinder_sources = [
'CopyDialog.cpp',
'PathFinder.cpp',
'Preferences.cpp',
'PropertyDialog.cpp'
]

pathfinder_icons = [
  'bigicons.bmp',
  'blocks_gif.gif',
  'bookdel_gif.gif',
  'booknxt_gif.gif',
  'bookprv_gif.gif',
  'bookclr_gif.gif',
  'bookset_gif.gif',
  'bookmrk_gif.gif',
  'closepanel.gif',
  'copy_gif.gif',
  'config_gif.gif',
  'cut_gif.gif',
  'delete_gif.gif',
  'desktop.bmp',
  'details.bmp',
  'dirup_gif.gif',
  'enter.gif',
  'file_gif.gif',
  'fileshown.gif',
  'filehidden.gif',
  'foldernew_gif.gif',
  'foxbig.gif',
  'foxmini.gif',
  'goup_gif.gif',
  'godown_gif.gif',
  'goback_gif.gif',
  'goforw_gif.gif',
  'gotodir.bmp',
  'home_gif.gif',
  'hosts.bmp',
  'iconpath.gif',
  'link_gif.gif',
  'location.gif',
  'maphost.bmp',
  'miscellaneous_gif.gif',
  'move_gif.gif',
  'paste.gif',
  'pattern_gif.gif',
  'property_gif.gif',
  'quit_gif.gif',
  'newfolder.bmp',
  'rename_gif.gif',
  'rotateleft.gif',
  'rotateright.gif',
  'run_bmp.bmp',
  'search.gif',
  'setdir.gif',
  'sorting.bmp',
  'smallicons.bmp',
  'unmaphost.bmp',
  'warningicon_gif.gif',
  'work_gif.gif',
]

pathfinder_icons_h = custom_target('pathfinder-icons-h',
    input : pathfinder_icons,
    output : 'icons.h',
    command : [reswrap, '-h', '-o', '@OUTPUT@', '@INPUT@']
)

pathfinder_icons_cpp = custom_target('pathfinder-icons-cpp',
    input : pathfinder_icons,
    output : 'icons.cpp',
    command : [reswrap, '-i', pathfinder_icons_h, '-o', '@OUTPUT@', '@INPUT@']
)

pathfinder_sources += pathfinder_icons_cpp

# Because PathFinder includes xincs.h it needs to be compiled like the
# source code of the FOX libraries itself. Therefore libfox_dep
# dependency is not used like for other included FOX apllications.
executable('pathfinder', pathfinder_sources,
    include_directories : [fox_include, fox_topdir_include],
    dependencies : fox_deps,
    link_with : libfox,
    cpp_args : fox_defines,
    install : true, gui_app : true
)

install_man('PathFinder.1')
