cpp = meson.get_compiler('cpp')

hdar_lib = shared_library(
    'usd_hdar',
    [
        # PUBLIC_CLASSES
        'systemSchema.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DHDAR_EXPORTS=1'],
    dependencies: [usd_base_dep, usd_dep],
    link_with: [hd_lib],
)
