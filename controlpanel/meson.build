
control_panel_sources = [
  'ControlPanel.cpp',
]

control_panel_icons = [
  'controlpanel_gif.gif',
  'colors_gif.gif',
  'filebinding_gif.gif',
  'settings_gif.gif',
]

control_panel_icons_h = custom_target('control_panel-icons-h',
    input : control_panel_icons,
    output : 'icons.h',
    command : [reswrap, '-h', '-o', '@OUTPUT@', '@INPUT@']
)

control_panel_icons_cpp = custom_target('control_panel-icons-cpp',
    input : control_panel_icons,
    output : 'icons.cpp',
    command : [reswrap, '-i', control_panel_icons_h, '-o', '@OUTPUT@', '@INPUT@']
)

control_panel_sources += control_panel_icons_cpp

executable('ControlPanel', control_panel_sources,
    dependencies: libfox_dep,
    include_directories: fox_topdir_include,
    install : true, gui_app : true
)

