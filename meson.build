project('fox', 'cpp', version : '1.6.57', default_options : 'cpp_std=c++11')

fox_version = meson.project_version()
ver_arr = fox_version.split('.')

fox_major = ver_arr[0]
fox_minor = ver_arr[1]
fox_level = ver_arr[2]
if build_machine.endian() == 'big'
    fox_byteorder = 1
else
    fox_byteorder = 0
endif

use_x_window = host_machine.system() != 'windows'

fox_deps = []
if host_machine.system() != 'windows'
    fox_deps += dependency('x11')
    fox_deps += dependency('xext')
    add_project_arguments(
        '-DTIME_WITH_SYS_TIME',
        '-DHAVE_STRTOLL',
        '-DHAVE_STRTOULL',
        '-DHAVE_VSSCANF',
        '-DHAVE_VSNPRINTF',

#        '-DSTDC_HEADERS',
#        '-DHAVE_SYS_TYPES_H',
#        '-DHAVE_SYS_STAT_H',
#        '-DHAVE_STDLIB_H',
#        '-DHAVE_STRING_H',
#        '-DHAVE_MEMORY_H',
#        '-DHAVE_STRINGS_H',
#        '-DHAVE_INTTYPES_H',
#        '-DHAVE_STDINT_H',
#        '-D__EXTENSIONS__',
#        '-D_ALL_SOURCE',
#        '-D_GNU_SOURCE',
#        '-D_POSIX_PTHREAD_SEMANTICS',
#        '-D_TANDEM_SOURCE',
#        '-DHAVE_DLFCN_H',

# TODO: X11 extentions should be detected.
        '-DHAVE_X11_EXTENSIONS_XSHM_H',
        '-DHAVE_X11_EXTENSIONS_SHAPE_H',
        '-DHAVE_X11_XCURSOR_XCURSOR_H',
        '-DHAVE_X11_EXTENSIONS_XRENDER_H',
        '-DHAVE_X11_EXTENSIONS_XRANDR_H',
        '-DHAVE_X11_EXTENSIONS_XFIXES_H',
        '-DHAVE_X11_EXTENSIONS_XINPUT_H',
        language : 'cpp')
else
    add_global_arguments('-DWIN32', '-DDLL_EXPORT', language : 'cpp')
endif

fox_package_name = meson.project_name()
fox_bugreport = 'jeroen@fox-toolkit.com'

add_project_arguments(
    '-DPACKAGE_NAME="@0@"'.format(fox_package_name),
    '-DPACKAGE_TARNAME="@0@"'.format(fox_package_name),
    '-DPACKAGE_VERSION="@0@"'.format(fox_version),
    '-DPACKAGE_STRING="@0@ @1@"'.format(fox_package_name, fox_version),
    '-DPACKAGE_BUGREPORT="@0@"'.format(fox_bugreport),
    '-DPACKAGE_URL=""',
    '-DPACKAGE="@0@"'.format(fox_package_name),
    '-DVERSION="@0@"'.format(fox_version),
    language : 'cpp'
)

if get_option('opengl')
    fox_deps += dependency('gl')
    fox_deps += dependency('glu')
endif

if use_x_window and get_option('xft')
    fox_deps += dependency('freetype2')
    fox_deps += dependency('fontconfig')
    fox_deps += dependency('xft')
    add_project_arguments('-DHAVE_XFT_H', language : 'cpp')
endif

conf = configuration_data()
# Surround the version in quotes to make it a C string
conf.set('FOX_MAJOR_VERSION', fox_major)
conf.set('FOX_MINOR_VERSION', fox_minor)
conf.set('FOX_PATCH_LEVEL', fox_level)
conf.set('FOX_BYTEORDER', fox_byteorder)

compiler = meson.get_compiler('cpp')

check_headers = [
  ['HAVE_UNISTD_H', 'unistd.h'],
  ['HAVE_DIRENT_H', 'dirent.h'],
  ['HAVE_SYS_WAIT_H', 'sys/wait.h'],
  ['HAVE_MMAP', 'sys/mman.h'],
]

foreach h : check_headers
    if compiler.has_header(h.get(1))
        add_project_arguments('-D@0@'.format(h.get(0)), language: 'cpp')
    endif
endforeach

fox_include = include_directories('include')

fox_deps += dependency('threads')

subdir('include')
subdir('utils')
subdir('src')
subdir('adie')
