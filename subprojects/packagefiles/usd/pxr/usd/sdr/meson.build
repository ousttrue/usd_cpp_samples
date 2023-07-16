cpp = meson.get_compiler('cpp')

sdr_lib = shared_library(
    'usd_sdr',
    [
        # PUBLIC_CLASSES
        'debugCodes.cpp',
        'registry.cpp',
        'shaderMetadataHelpers.cpp',
        'shaderNode.cpp',
        'shaderProperty.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DSDR_EXPORTS=1'],
    dependencies: [boost_dep, tbb_dep],
    link_with: [
        gf_lib,
        trace_lib,
        arch_lib,
        tf_lib,
        vt_lib,
        ar_lib,
        sdf_lib,
        ndr_lib,
    ],
)
