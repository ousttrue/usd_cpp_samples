cpp = meson.get_compiler('cpp')

usdHydra_lib = shared_library(
    'usd_usdHydra',
    [
        # PUBLIC_CLASSES
        'tokens.cpp',
        'discoveryPlugin.cpp',
        'generativeProceduralAPI.cpp',


    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + [
        '-DUSDHYDRA_EXPORTS=1',
        '-DMFB_PACKAGE_NAME=usdHydra',
        '-DMFB_ALT_PACKAGE_NAME=usdHydra',
    ],
    dependencies: [usd_base_dep],
    link_with: [usd_lib, usdShade_lib, sdf_lib, ndr_lib, ar_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_usdHydra.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/usdHydra/resources',
    configuration: cdata,
)

install_data(
    'generatedSchema.usda',
    install_dir: 'bin/pluginfo/usdHydra/resources',
)
install_data(
    'schema.usda',
    install_dir: 'bin/pluginfo/usdHydra/resources/usdHydra',
)
install_data(
    'shaders/shaderDefs.usda',
    install_dir: 'bin/pluginfo/usdHydra/resources/shaders',
)
install_data(
    'shaders/empty.glslfx',
    install_dir: 'bin/pluginfo/usdHydra/resources/shaders',
)
