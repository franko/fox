project('fox', 'cpp', version : '1.6.57', default_options : 'cpp_std=c++11')

pkg = import('pkgconfig')

fox_version = meson.project_version()
fox_version_array = fox_version.split('.')

if build_machine.endian() == 'big'
    fox_byteorder = 1
else
    fox_byteorder = 0
endif

cc = meson.get_compiler('cpp')
host_system = host_machine.system()

use_x_window = host_system != 'windows'

# public, external dependencies and defines
fox_ext_deps = []
fox_ext_defines = []

# private FOX library's dependencies and defines
fox_deps = []
fox_defines = []

if host_system != 'windows'
    fox_deps += dependency('x11')
    fox_deps += dependency('xext')
    fox_defines += [
        '-DTIME_WITH_SYS_TIME',
# TODO: X11 extentions should be detected.
        '-DHAVE_X11_EXTENSIONS_XSHM_H',
        '-DHAVE_X11_EXTENSIONS_SHAPE_H',
        '-DHAVE_X11_XCURSOR_XCURSOR_H',
        '-DHAVE_X11_EXTENSIONS_XRENDER_H',
        '-DHAVE_X11_EXTENSIONS_XRANDR_H',
        '-DHAVE_X11_EXTENSIONS_XFIXES_H',
        '-DHAVE_X11_EXTENSIONS_XINPUT_H'
    ]
else
    fox_defines += ['-DDLL_EXPORT']
    fox_ext_defines += '-DWIN32'
    foreach lib : ['comctl32', 'wsock32', 'winspool', 'mpr', 'gdi32', 'imm32']
        fox_ext_deps += cc.find_library(lib, required : true)
    endforeach
endif

if host_system == 'linux'
    fox_ext_deps += cc.find_library('dl', required : false)
    fox_defines += ['-D_GNU_SOURCE', '-D_POSIX_PTHREAD_SEMANTICS']
endif

fox_package_name = meson.project_name()
fox_bugreport = 'jeroen@fox-toolkit.com'

fox_defines += [
    '-DPACKAGE_NAME="@0@"'.format(fox_package_name),
    '-DPACKAGE_TARNAME="@0@"'.format(fox_package_name),
    '-DPACKAGE_VERSION="@0@"'.format(fox_version),
    '-DPACKAGE_STRING="@0@ @1@"'.format(fox_package_name, fox_version),
    '-DPACKAGE_BUGREPORT="@0@"'.format(fox_bugreport),
    '-DPACKAGE_URL=""',
    '-DPACKAGE="@0@"'.format(fox_package_name),
    '-DVERSION="@0@"'.format(fox_version)
]

if get_option('opengl')
    gl = dependency('gl', required : true)
    fox_ext_defines += '-DHAVE_GL_H'
    fox_ext_deps += gl

    if host_system == 'windows'
        if cc.has_header('GL/glu.h')
            fox_ext_defines += '-DHAVE_GLU_H'
            fox_ext_deps += cc.find_library('glu32', required : true)
        endif
    else
        glu = dependency('glu', required : false)
        if glu.found()
            fox_ext_defines += '-DHAVE_GLU_H'
            fox_ext_deps += glu
        endif
    endif
endif

if get_option('jpeg')
    libjpeg = dependency('libjpeg', required : false)
    if libjpeg.found()
        fox_defines += '-DHAVE_JPEG_H'
        fox_deps += libjpeg
    endif
endif

if get_option('png')
    libpng = dependency('libpng', required : false)
    if libpng.found()
        fox_defines += '-DHAVE_PNG_H'
        fox_deps += libpng
    endif
endif

if get_option('zlib')
    zlib = dependency('zlib', required : false)
    if zlib.found()
        fox_defines += '-DHAVE_ZLIB_H'
        fox_deps += zlib
    endif
endif

if use_x_window and get_option('xft')
    fox_deps += dependency('freetype2')
    fox_deps += dependency('fontconfig')
    fox_deps += dependency('xft')
    fox_defines += '-DHAVE_XFT_H'
endif

conf = configuration_data()
conf.set('FOX_MAJOR_VERSION', fox_version_array[0])
conf.set('FOX_MINOR_VERSION', fox_version_array[1])
conf.set('FOX_PATCH_LEVEL', fox_version_array[2])
conf.set('FOX_BYTEORDER', fox_byteorder)

check_headers = [
  ['HAVE_UNISTD_H', 'unistd.h'],
  ['HAVE_DIRENT_H', 'dirent.h'],
  ['HAVE_SYS_WAIT_H', 'sys/wait.h'],
  ['HAVE_MMAP', 'sys/mman.h'],
]

foreach h : check_headers
    if cc.has_header(h[1])
        fox_defines += '-D@0@'.format(h[0])
    endif
endforeach

strtoll_code = '''#include<stdlib.h>
long long int func() { char s[2] = {'1', '\0'}, *tail; return strtoll(s, &tail, 10); }'''

if cc.compiles(strtoll_code)
    fox_defines += '-DHAVE_STRTOLL'
endif

strtoull_code = '''#include<stdlib.h>
unsigned long long int func() { char s[2] = {'1', '\0'}, *tail; return strtoull(s, &tail, 10); }'''

if cc.compiles(strtoull_code)
    fox_defines += '-DHAVE_STRTOULL'
endif

vsnprintf_code = '''#include<stdio.h>
#include<stdarg.h>
int func(va_list ap) { char b[8]; return vsnprintf(b, 8, "%d", ap); }'''

if cc.compiles(vsnprintf_code)
    fox_defines += '-DHAVE_VSNPRINTF'
endif

vsscanf_code = '''#include<stdio.h>
#include<stdarg.h>
int func(const char *s, va_list ap) { return vsscanf(s, "%d", ap); }'''

if cc.compiles(vsscanf_code)
    fox_defines += '-DHAVE_VSSCANF'
endif

fox_ext_deps += dependency('threads')

fox_include = include_directories('include')
install_subdir('include', install_dir : '', exclude_files : 'meson.build')

fox_defines += fox_ext_defines
fox_deps += fox_ext_deps

subdir('include')
subdir('utils')
subdir('src')
subdir('tests')
subdir('adie')
