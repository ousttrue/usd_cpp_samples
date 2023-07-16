cpp = meson.get_compiler('cpp')

vt_lib = shared_library(
    'usd_vt',
    [
        # PUBLIC_CLASSES
        'array.cpp',
        'dictionary.cpp',
        'functions.cpp',
        'hash.cpp',
        'streamOut.cpp',
        'types.cpp',
        'value.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DVT_EXPORTS=1'],
    dependencies: [boost_dep, tbb_dep],
    link_with: [arch_lib, tf_lib, gf_lib, trace_lib],
)
