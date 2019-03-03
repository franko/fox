libchart_sources = ['FXChart.cpp']

libchart = library('CHART-1.6', libchart_sources, dependencies : libfox_dep, install : false)

chart_sources = ['chart.cpp']
chart_icons = ['marble.bmp']

chart_icons_h = custom_target('chart-icons-h',
    input : chart_icons,
    output : 'icons.h',
    command : [reswrap, '-i', '-o', '@OUTPUT@', '@INPUT@']
)

chart_icons_cpp = custom_target('chart-icons-cpp',
    input : chart_icons,
    output : 'icons.cpp',
    command : [reswrap, '-e', '-o', '@OUTPUT@', '@INPUT@']
)

chart_sources += chart_icons_cpp

executable('chart', chart_sources,
    dependencies: libfox_dep, link_with : libchart, install : true, gui_app : true
)
