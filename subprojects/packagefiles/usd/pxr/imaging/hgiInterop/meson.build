cpp = meson.get_compiler('cpp')

hgiInterop_lib = shared_library(
    'usd_hgiInterop',
    [
        # CPPFILES
        'opengl.cpp',
        'hgiInterop.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DHGIINTEROP_EXPORTS=1'],
    dependencies: [usd_dep],
    link_with: [garch_lib, hgi_lib],
)
