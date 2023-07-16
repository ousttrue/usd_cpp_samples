cpp = meson.get_compiler('cpp')

pxOsd_lib = shared_library(
    'usd_pxOsd',
    [
        # PUBLIC_CLASSES
        'meshTopology.cpp',
        'meshTopologyValidation.cpp',
        'refinerFactory.cpp',
        'subdivTags.cpp',
        'tokens.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DPXOSD_EXPORTS=1'],
    dependencies: [boost_dep, tbb_dep],
    link_with: [tf_lib, gf_lib, vt_lib, trace_lib, arch_lib],
)
