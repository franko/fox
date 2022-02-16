project('fox', 'cpp', version : '1.7.77', default_options : 'cpp_std=c++11')

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

supported_host_systems = ['windows', 'cygwin', 'linux', 'netbsd', 'freebsd', 'gnu']

if host_system not in supported_host_systems
    error('Host system \'' + host_system + '\' is not supported.')
endif

host_is_window = (host_system == 'windows') or (host_system == 'cygwin')
use_x_window = not host_is_window

# public, external dependencies and defines
fox_ext_deps = []
fox_ext_defines = []

# private FOX library's dependencies and defines
fox_deps = []
fox_defines = []

if not host_is_window
    fox_deps += dependency('x11')
    fox_deps += dependency('xext')
    fox_defines += [
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
    fox_ext_defines += ['-DWIN32', '-D_WINDOWS']
    foreach lib : ['comctl32', 'wsock32', 'winspool', 'mpr', 'gdi32', 'imm32']
        fox_ext_deps += cc.find_library(lib, required : true)
    endforeach
endif

time_msys_code = '''#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
int main () { if ((struct tm *) 0) { return 0; } return 0; }'''

if cc.compiles(time_msys_code)
    fox_defines += '-DTIME_WITH_SYS_TIME'
endif

if cc.get_id() in ['gcc', 'clang']
    fox_defines += ['-D_ALL_SOURCE', '-D_GNU_SOURCE', '-D_POSIX_PTHREAD_SEMANTICS']
endif

if host_system == 'linux'
    fox_ext_deps += cc.find_library('dl', required : false)
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

if get_option('bzlib')
    bzlib = dependency('bzlib', required : false)
    if bzlib.found()
        fox_defines += ['-DHAVE_BZLIB_H', '-DHAVE_BZ2LIB_H']
        fox_deps += bzlib
    endif
endif

freetype_dep   = dependency('freetype2', required: false)
fontconfig_dep = dependency('fontconfig', required: false)
xft_dep        = dependency('xft', required: false)

has_xft_deps = freetype_dep.found() and fontconfig_dep.found() and xft_dep.found()

if use_x_window and get_option('xft') and has_xfs_deps
    fox_deps += [freetype_dep, fontconfig_dep, xft_dep]
    fox_defines += '-DHAVE_XFT_H'
endif

conf = configuration_data()
conf.set('FOX_MAJOR_VERSION', fox_version_array[0])
conf.set('FOX_MINOR_VERSION', fox_version_array[1])
conf.set('FOX_PATCH_LEVEL', fox_version_array[2])
conf.set('FOX_BYTEORDER', fox_byteorder)

# Not actually used '-DSTDC_HEADERS'
check_headers = [
  ['HAVE_SYS_TYPES_H', 'sys/types.h'],
  ['HAVE_SYS_STAT_H', 'sys/stat.h'],
  ['HAVE_STDLIB_H', 'stdlib.h'],
  ['HAVE_STRING_H', 'string.h'],
  ['HAVE_MEMORY_H', 'memory.h'],
  ['HAVE_STRINGS_H', 'strings.h'],
  ['HAVE_INTTYPES_H', 'inttypes.h'],
  ['HAVE_UNISTD_H', 'unistd.h'],
  ['HAVE_DIRENT_H', 'dirent.h'],
  ['HAVE_SYS_WAIT_H', 'sys/wait.h'],
  ['HAVE_SEMAPHORE_H', 'semaphore.h'],
  ['HAVE_IMMINTRIN_H', 'immintrin.h'],
  # The valus below are not actually used.
  ['HAVE_STDINT_H', 'stdint.h'],
  ['HAVE_SYS_TYPES_H', 'sys/types.h'],
  ['HAVE_SYS_STAT_H', 'sys/stat.h'],
]

foreach h : check_headers
    if cc.has_header(h[1])
        fox_defines += '-D@0@'.format(h[0])
    endif
endforeach

check_functions = [
    ['localtime_r',    'time.h',        '#include<time.h>'],
    ['gmtime_r',       'time.h',        '#include<time.h>'],
    ['getpwnam_r',     'pwd.h',         '#include<sys/types.h>\n#include<pwd.h>'],
    ['getpwuid_r',     'pwd.h',         '#include<sys/types.h>\n#include<pwd.h>'],
    ['getgrgid_r',     'grp.h',         '#include<sys/types.h>\n#include<grp.h>'],
    ['sched_getcpu',   'sched.h',       '#include<sched.h>'],
    ['epoll_create1',  'epoll.h',       '#include<epoll.h>'],
    ['timerfd_create', 'sys/timerfd.h', '#include<sys/timerfd.h>'],
    ['pipe2',          'unistd.h',      '#include<unistd.h>'],
    ['statvfs',        'sys/statvfs.h', '#include<sys/statvfs.h>'],
    ['getrlimit',      'sys/resource.h','#include<sys/resource.h>'],
    ['inotify_init1',  'sys/inotify.h', '#include<sys/inotify.h>'],
]


foreach f : check_functions
    if cc.has_header(f[1]) and cc.has_function(f[0], prefix: f[2])
        fox_defines += '-DHAVE_@0@'.format(f[0].to_upper())
    endif
endforeach

fox_ext_deps += dependency('threads')

# fox_defines += '-DHAVE_LIBPTHREAD' # not used
if host_is_window
    pthread_prefix='''#include <pthread.h>'''
else
    pthread_prefix = '''
        #ifndef _GNU_SOURCE
        # define _GNU_SOURCE
        #endif
        #include <pthread.h>'''
endif
if cc.has_header_symbol('pthread.h', 'pthread_getname_np', prefix : pthread_prefix)
    fox_defines += [
        '-DHAVE_PTHREAD_GETNAME_NP',
        '-DHAVE_PTHREAD_SETNAME_NP',
    ]
endif
if cc.has_header_symbol('pthread.h', 'pthread_setaffinity_np', prefix : pthread_prefix)
    fox_defines += [
        # '-DHAVE_PTHREAD_GETAFFINITY_NP', # not used
        '-DHAVE_PTHREAD_SETAFFINITY_NP',
    ]
endif

fox_include = include_directories('include')
fox_topdir_include = include_directories('.')
install_subdir('include', install_dir : '', exclude_files : 'meson.build')

fox_defines += fox_ext_defines
fox_deps += fox_ext_deps

subdir('include')
subdir('utils')
subdir('lib')
if get_option('apps')
    # subdir('tests')
    subdir('adie')
    # subdir('calculator')
    # subdir('chart')
    # subdir('pathfinder')
    # subdir('shutterbug')
endif
