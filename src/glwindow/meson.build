gl_dep = dependency('gl')
glwindow_lib = static_library(
    'glwindow',
    [
        'glwindow.cpp',
        'glPlatformDebugWindowWindows.cpp',
    ],
)
glwindow_dep = declare_dependency(
    include_directories: include_directories('.'),
    link_with: glwindow_lib,
    dependencies: [gl_dep],
)
