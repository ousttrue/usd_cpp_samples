project('tbb', 'cpp', version: '20190410oss')

cc = meson.get_compiler('cpp')

if get_option(
    'buildtype',
).startswith('debug') # debug or debugoptimized
    tbb_lib = cc.find_library(
        'tbb_debug',
        dirs: meson.current_source_dir() + '/lib/intel64/vc14',
    )
    install_data('bin/intel64/vc14/tbb_debug.dll', install_dir: 'bin')
else
    tbb_lib = cc.find_library(
        'tbb',
        dirs: meson.current_source_dir() + '/lib/intel64/vc14',
    )
    install_data('bin/intel64/vc14/tbb.dll', install_dir: 'bin')
endif

tbb_dep = declare_dependency(
    include_directories: include_directories('include'),
    dependencies: tbb_lib,
)
meson.override_dependency('tbb', tbb_dep)
