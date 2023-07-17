cpp = meson.get_compiler('cpp')

cameraUtil_lib = shared_library(
    'usd_cameraUtil',
    [
        'conformWindow.cpp',
        'framing.cpp',
        'screenWindowParameters.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DCAMERAUTIL_EXPORTS=1'],
    dependencies: [usd_dep],
    link_with: [],
)
