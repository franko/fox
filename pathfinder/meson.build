pathfinder_sources = [
'CopyDialog.cpp',
'PathFinder.cpp',
'Preferences.cpp',
'PropertyDialog.cpp'
]

pathfinder_icons = [
'bigicons.bmp',
'closepanel.gif',
'clrbook.gif',
'copy.bmp',
'copyit.gif',
'cut.bmp',
'deleteit.bmp',
'desktop.bmp',
'details.bmp',
'dirup.bmp',
'enter.gif',
'file_gif.gif',
'foxbig.gif',
'foxmini.gif',
'goback.bmp',
'goforw.bmp',
'gotodir.bmp',
'home.gif',
'hosts.bmp',
'iconpath.gif',
'linkit.gif',
'location.gif',
'maphost.bmp',
'mimetype.gif',
'moveit.gif',
'paste.bmp',
'pattern_gif.gif',
'properties.bmp',
'quit_gif.gif',
'renameit.gif',
'rotateleft.gif',
'rotateright.gif',
'setbook.gif',
'setdir.gif',
'smallicons.bmp',
'unmaphost.bmp',
'work.gif',
]

pathfinder_icons_h = custom_target('pathfinder-icons-h',
    input : pathfinder_icons,
    output : 'icons.h',
    command : [reswrap, '-i', '-o', '@OUTPUT@', '@INPUT@']
)

pathfinder_icons_cpp = custom_target('pathfinder-icons-cpp',
    input : pathfinder_icons,
    output : 'icons.cpp',
    command : [reswrap, '-e', '-o', '@OUTPUT@', '@INPUT@']
)

pathfinder_sources += pathfinder_icons_cpp

# Because PathFinder includes xincs.h it needs to be compiled like the
# source code of the FOX libraries itself. Therefore libfox_dep
# dependency is not used like for other included FOX apllications.
executable('pathfinder', pathfinder_sources,
    include_directories : fox_include,
    dependencies : fox_deps,
    link_with : libfox,
    cpp_args : fox_defines,
    install : true, gui_app : true
)

install_man('PathFinder.1')
