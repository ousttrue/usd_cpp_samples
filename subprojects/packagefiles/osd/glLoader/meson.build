glew_dep = dependency('glew')
glloader_inc = include_directories('.')
glloader_args = ['-DOSD_USES_GLEW']
glloader_lib = static_library(
    'glloader',
    [
        'glApi.cpp',
        'glLoader.cpp',
    ],
    include_directories: glloader_inc,
    cpp_args: glloader_args,
    dependencies: [glew_dep],
)
glloader_dep = declare_dependency(
    link_with: glloader_lib,
    include_directories: glloader_inc,
    compile_args: glloader_args,
    dependencies: [glew_dep],
)
