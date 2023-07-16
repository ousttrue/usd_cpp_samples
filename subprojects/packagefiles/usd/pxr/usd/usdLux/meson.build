cpp = meson.get_compiler('cpp')

usdLux_lib = shared_library(
    'usd_usdLux',
    [
        # PUBLIC_CLASSES
        'blackbody.cpp',
        'boundableLightBase.cpp',
        'cylinderLight.cpp',
        'diskLight.cpp',
        'distantLight.cpp',
        'domeLight.cpp',
        'geometryLight.cpp',
        'lightAPI.cpp',
        'lightFilter.cpp',
        'lightListAPI.cpp',
        'listAPI.cpp',
        'meshLightAPI.cpp',
        'nonboundableLightBase.cpp',
        'pluginLight.cpp',
        'pluginLightFilter.cpp',
        'portalLight.cpp',
        'rectLight.cpp',
        'shadowAPI.cpp',
        'shapingAPI.cpp',
        'sphereLight.cpp',
        'tokens.cpp',
        'volumeLightAPI.cpp',

        # PRIVATE_CLASSES
        'discoveryPlugin.cpp',
        'lightDefParser.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DUSDLUX_EXPORTS=1'],
    dependencies: [usd_base_dep],
    link_with: [ndr_lib, sdr_lib, sdf_lib, usd_lib, usdGeom_lib, usdShade_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_usdLux.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/usdLux/resources',
    configuration: cdata,
)

install_data(
    'generatedSchema.usda',
    install_dir: 'bin/pluginfo/usdLux/resources',
)
install_data(
    'schema.usda',
    install_dir: 'bin/pluginfo/usdLux/resources/usdLux',
)
