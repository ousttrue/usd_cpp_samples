cpp = meson.get_compiler('cpp')

hf_lib = shared_library(
    'usd_hf',
    [
        'pluginBase.cpp',
        'pluginRegistry.cpp',
        'pluginEntry.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DHF_EXPORTS=1'],
    dependencies: [usd_dep],
    link_with: [],
)
