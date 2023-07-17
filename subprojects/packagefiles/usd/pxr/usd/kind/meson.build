cpp = meson.get_compiler('cpp')

kind_lib = shared_library(
    'usd_kind',
    [
        'registry.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DKIND_EXPORTS=1'],
    dependencies: [usd_base_dep],
)

