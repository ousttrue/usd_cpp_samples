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
    cpp_args: args + ['-DPXOSD_EXPORTS=1'],
    dependencies: [usd_dep, osd_dep],
    link_with: [],
)
