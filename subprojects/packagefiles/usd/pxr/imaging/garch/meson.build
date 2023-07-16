cpp = meson.get_compiler('cpp')

gl_dep = dependency('gl')

garch_lib = shared_library(
    'usd_garch',
    [
        'glDebugWindow.cpp',
        'glPlatformDebugContext.cpp',
        # Windows
        'glPlatformContextWindows.cpp',
        'glPlatformDebugWindowWindows.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DGARCH_EXPORTS=1'],
    dependencies: [usd_dep, gl_dep],
    link_with: [],
)
