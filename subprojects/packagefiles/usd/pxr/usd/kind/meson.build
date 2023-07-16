cpp = meson.get_compiler('cpp')

kind_lib = shared_library(
    'usd_kind',
    [
        'registry.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DKIND_EXPORTS=1'],
    dependencies: [boost_dep],
    link_with: [arch_lib, tf_lib, js_lib, plug_lib],
)
