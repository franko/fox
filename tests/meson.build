
test_apps = [
  ['gaugetest', 'gaugetest.cpp'],
  ['format', 'format.cpp'],
  ['timefmt', 'timefmt.cpp'],
  ['scan', 'scan.cpp'],
  ['console', 'console.cpp'],
  ['thread', 'thread.cpp'],
  ['calendar', 'calendar.cpp'],
  ['expression', 'expression.cpp'],
  ['wizard', 'wizard.cpp'],
  ['rex', 'rex.cpp'],
  ['layout', 'layout.cpp'],
  ['minheritance', 'minheritance.cpp'],
  ['memmap', 'memmap.cpp'],
  ['mditest', 'mditest.cpp'],
  ['table', 'table.cpp'],
  ['groupbox', 'groupbox.cpp'],
  ['foursplit', 'foursplit.cpp'],
  ['datatarget', 'datatarget.cpp'],
  ['bitmapviewer', 'bitmapviewer.cpp'],
  ['imageviewer', 'imageviewer.cpp'],
  ['scribble', 'scribble.cpp'],
  ['splitter', 'splitter.cpp'],
  ['shutter', 'shutter.cpp'],
  ['image', 'image.cpp'],
  ['header', 'header.cpp'],
  ['hello', 'hello.cpp'],
  ['hello2', 'hello2.cpp'],
  ['switcher', 'switcher.cpp'],
  ['process', 'process.cpp'],
  ['iconlist', 'iconlist.cpp'],
  ['tabbook', 'tabbook.cpp'],
  ['ratio', 'ratio.cpp'],
  ['dialog', 'dialog.cpp'],
  ['dirlist', 'dirlist.cpp'],
  ['button', 'button.cpp'],
  ['codecs', 'codecs.cpp'],
  ['dctest', 'dctest.cpp'],
  ['half', 'half.cpp'],
  ['match', 'match.cpp'],
  ['unicode', 'unicode.cpp'],
  ['parallel', 'parallel.cpp'],
  ['variant', 'variant.cpp'],
  ['math', 'math.cpp'],
  ['xml', 'xml.cpp'],
  ['dictest', 'dictest.cpp'],
]


opengl_test_apps = [
    ['gltest', 'gltest.cpp'],
]

foreach app : test_apps
    executable(app[0], app[1], dependencies: libfox_dep, gui_app : true, install : false)
endforeach

if get_option('opengl')
    foreach app : opengl_test_apps
        executable(app[0], app[1], dependencies: libfox_dep, gui_app : true, install : false)
    endforeach
endif
