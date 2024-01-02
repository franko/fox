glviewer_sources = [
  'FXGLCone.cpp',
  'FXGLCube.cpp',
  'FXGLCylinder.cpp',
  'FXGLLine.cpp',
  'FXGLPoint.cpp',
  'FXGLShape.cpp',
  'FXGLSphere.cpp',
  'FXGLTriangleMesh.cpp',
  'glviewer.cpp',
]

if get_option('opengl') and have_glu
    executable('glviewer', glviewer_sources,
        dependencies : [ libfox_dep ] + fox_ext_deps,
        cpp_args : fox_ext_defines,
        include_directories: fox_topdir_include,
        install : true, gui_app : true
    )
endif
