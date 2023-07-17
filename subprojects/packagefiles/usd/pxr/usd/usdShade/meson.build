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

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_usdShade.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/usdShade/resources',
    configuration: cdata,
)

install_data(
    'generatedSchema.usda',
    install_dir: 'bin/pluginfo/usdShade/resources',
)
install_data(
    'schema.usda',
    install_dir: 'bin/pluginfo/usdShade/resources/usdShade',
)

