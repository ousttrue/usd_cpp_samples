cpp = meson.get_compiler('cpp')

usdShade_lib = shared_library(
    'usd_usdShade',
    [
        # PUBLIC_CLASSES
        'connectableAPI.cpp',
        'connectableAPIBehavior.cpp',
        'coordSysAPI.cpp',
        'input.cpp',
        'material.cpp',
        'materialBindingAPI.cpp',
        'nodeDefAPI.cpp',
        'output.cpp',
        'shader.cpp',
        'shaderDefParser.cpp',
        'shaderDefUtils.cpp',
        'nodeGraph.cpp',
        'tokens.cpp',
        'udimUtils.cpp',
        'utils.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DUSDSHADE_EXPORTS=1'],
    dependencies: [usd_base_dep],
    link_with: [
        sdr_lib,
        usdGeom_lib,
        ndr_lib,
        usd_lib,
        pcp_lib,
        sdf_lib,
        ar_lib,
    ],
)
