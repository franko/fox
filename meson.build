project('fox', 'cpp', version : '1.6.57', default_options : 'cpp_std=c++03')

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

fox_deps = []
if host_machine.system() != 'windows'
    fox_deps += dependency('x11')
    fox_deps += dependency('xext')
else
    add_global_arguments('-DWIN32', language : 'cpp')
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

if get_option('enable-opengl')
    fox_deps += dependency('gl')
    fox_deps += dependency('glu')
endif

conf = configuration_data()
# Surround the version in quotes to make it a C string
conf.set_quoted('FOX_MAJOR_VERSION', fox_major)
conf.set_quoted('FOX_MINOR_VERSION', fox_minor)
conf.set_quoted('FOX_LEVEL_VERSION', fox_level)
conf.set_quoted('FOX_BYTEORDER', fox_byteorder)

configure_file(input: 'include/fxver.h.in', output : 'include/fxver.h', configuration : conf)

subdir('src')
