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
    cpp_args: args + ['-DSDR_EXPORTS=1'],
    dependencies: [usd_base_dep],
    link_with: [ar_lib, sdf_lib, ndr_lib],
)
