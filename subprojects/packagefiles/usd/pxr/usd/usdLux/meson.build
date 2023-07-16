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
