cpp = meson.get_compiler('cpp')

work_lib = shared_library(
    'usd_work',
    [
        # PUBLIC_CLASSES
        'detachedTask.cpp',
        'dispatcher.cpp',
        'loops.cpp',
        'reduce.cpp',
        'singularTask.cpp',
        'threadLimits.cpp',
        'utils.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DWORK_EXPORTS=1'],
    dependencies: [boost_dep, tbb_dep],
    link_with: [tf_lib, arch_lib],
)
