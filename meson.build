project('fox', 'cpp', version : '1.7.84', default_options : 'cpp_std=c++11')

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

if get_option('default_library') != 'static'
    fox_defines += '-DFOXDLL'
endif

if not host_is_window
    fox_deps += dependency('x11')
    fox_deps += dependency('xext')
    foreach libname : ['xrandr', 'xrender', 'xfixes', 'xcursor', 'xi']
        fox_deps += dependency(libname, required: false)
    endforeach
else
    fox_ext_defines += '-DWIN32'
    if cc.get_id() == 'msvc'
        fox_defines += ['-D_WINDOWS', '-DUNICODE', '-D_USRDLL', '-D_CRT_SECURE_NO_WARNINGS']
    endif
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

have_glu = false
if get_option('opengl')
    gl = dependency('gl', required : true)
    fox_ext_defines += '-DHAVE_GL_H'
    fox_ext_deps += gl

    if use_x_window
        if cc.has_header('GL/glx.h')
            fox_ext_defines += '-DHAVE_GLX_H'
            fox_ext_deps += dependency('glx')
        else
            error('Cannot find GL/glx.h headers: required by opengl')
        endif
    endif

    if host_system == 'windows'
        if cc.has_header('GL/glu.h')
            fox_ext_defines += '-DHAVE_GLU_H'
            fox_ext_deps += cc.find_library('glu32', required : true)
            have_glu = true
        endif
    else
        glu = dependency('glu', required : false)
        if glu.found()
            fox_ext_defines += '-DHAVE_GLU_H'
            fox_ext_deps += glu
            have_glu = true
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
    # the bzlib is provided without a pkg-config file.
    if cc.has_header('bzlib.h')
        bzlib = cc.find_library('bz2', required : false)
        if bzlib.found()
            fox_defines += '-DHAVE_BZ2LIB_H'
            fox_deps += bzlib
        endif
    endif
endif

if get_option('cups')
    libcups_dep = dependency('cups', method : 'config-tool')
    # if not cc.has_function('cupsGetDests', dependencies: libcups_dep)
    fox_defines += '-DHAVE_CUPS_H'
    fox_deps += libcups_dep
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
  ['HAVE_SYS_SELECT_H', 'sys/select.h'],
  ['HAVE_SYS_EPOLL_H', 'sys/epoll.h'],
  ['HAVE_SYS_TIMERFD_H', 'sys/timerfd.h'],
  ['HAVE_SYS_IPC_H', 'sys/ipc.h'],
  ['HAVE_SYS_SHM_H', 'sys/shm.h'],
  ['HAVE_SYS_MMAN_H', 'sys/mman.h'],
  ['HAVE_SYS_PSTAT_H', 'sys/pstat.h'],
  ['HAVE_SYS_MOUNT_H', 'sys/mount.h'],
  ['HAVE_SYS_WAIT_H', 'sys/wait.h'],
  ['HAVE_SYS_STATVFS_H', 'sys/statvfs.h'],
  ['HAVE_SYS_INOTIFY_H', 'sys/inotify.h'],
  ['HAVE_SYS_RESOURCE_H', 'sys/resource.h'],
  ['HAVE_SYS_SYSCTL_H', 'sys/sysctl.h'],
  ['HAVE_DLFCN_H', 'dlfcn.h'],
  ['HAVE_STDLIB_H', 'stdlib.h'],
  ['HAVE_STRING_H', 'string.h'],
  ['HAVE_MEMORY_H', 'memory.h'],
  ['HAVE_STRINGS_H', 'strings.h'],
  ['HAVE_INTTYPES_H', 'inttypes.h'],
  ['HAVE_UNISTD_H', 'unistd.h'],
  ['HAVE_DIRENT_H', 'dirent.h'],
  ['HAVE_SEMAPHORE_H', 'semaphore.h'],
  ['HAVE_IMMINTRIN_H', 'immintrin.h'],
  # The valus below are not actually used.
  ['HAVE_STDINT_H', 'stdint.h'],
  ['HAVE_SYS_TYPES_H', 'sys/types.h'],
  ['HAVE_SYS_STAT_H', 'sys/stat.h'],
]

x11_check_headers = [
    ['HAVE_XSHM_H',    'X11/extensions/XShm.h'],
    ['HAVE_SHAPE_H',   'X11/extensions/shape.h'],
    ['HAVE_XCURSOR_H', 'X11/Xcursor/Xcursor.h'],
    ['HAVE_XRENDER_H', 'X11/extensions/Xrender.h'],
    ['HAVE_XRANDR_H',  'X11/extensions/Xrandr.h'],
    ['HAVE_XFIXES_H',  'X11/extensions/Xfixes.h'],
    ['HAVE_XINPUT2_H', 'X11/extensions/XInput2.h'],
]

foreach h : check_headers
    if cc.has_header(h[1])
        fox_defines += '-D@0@'.format(h[0])
    endif
endforeach

freetype_dep   = dependency('freetype2', required: false)
fontconfig_dep = dependency('fontconfig', required: false)
xft_dep        = dependency('xft', required: false)

has_xft_deps = freetype_dep.found() and fontconfig_dep.found() and xft_dep.found()

if use_x_window and get_option('xft') and has_xft_deps
    foreach h : x11_check_headers
        if cc.has_header(h[1])
            fox_defines += '-D@0@'.format(h[0])
        endif
    endforeach
    fox_deps += [freetype_dep, fontconfig_dep, xft_dep]
    fox_defines += '-DHAVE_XFT_H'
endif

check_functions = [
    ['localtime_r',    'time.h',        '#include<time.h>'],
    ['gmtime_r',       'time.h',        '#include<time.h>'],
    ['getpwnam_r',     'pwd.h',         '#include<sys/types.h>\n#include<pwd.h>'],
    ['getpwuid_r',     'pwd.h',         '#include<sys/types.h>\n#include<pwd.h>'],
    ['getgrgid_r',     'grp.h',         '#include<sys/types.h>\n#include<grp.h>'],
    ['sched_getcpu',   'sched.h',       '#include<sched.h>'],
    ['epoll_create1',  'sys/epoll.h',   '#include<sys/epoll.h>'],
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
    subdir('controlpanel')
    subdir('glviewer')
    subdir('adie')
    subdir('calculator')
    subdir('chart')
    subdir('pathfinder')
    subdir('shutterbug')
endif
if get_option('tests')
    subdir('tests')
endif

