libchart_sources = [
    'FX2DChart.cpp',
    'FX2DPlot.cpp',
    'FXAxis.cpp',
    'FXChart.cpp',
    'FXCurve.cpp',
    'chartutils.cpp'
]

libchart = library('CHART-1.6', libchart_sources,
    include_directories : fox_include,
    cpp_args : fox_defines + ['-DCHARTDLL_EXPORTS'],
    dependencies : libfox_dep,
    install : false
)

chart_icons = ['marble.bmp']

chart_icons_h = custom_target('chart-icons-h',
    input : chart_icons,
    output : 'icons.h',
    command : [reswrap, '-h', '-o', '@OUTPUT@', '@INPUT@']
)

chart_icons_cpp = custom_target('chart-icons-cpp',
    input : chart_icons,
    output : 'icons.cpp',
    command : [reswrap, '-i', chart_icons_h, '-o', '@OUTPUT@', '@INPUT@']
)

libchart_sources += chart_icons_cpp

executable('charttest', libchart_sources + ['charttest.cpp'],
    dependencies: libfox_dep,
    include_directories: fox_topdir_include,
    link_with : libchart, install : true, gui_app : true
)
