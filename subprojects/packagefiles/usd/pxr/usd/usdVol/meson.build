cpp = meson.get_compiler('cpp')

usdVol_lib = shared_library(
    'usd_usdVol',
    [
        # PUBLIC_CLASSES
        'volume.cpp',
        'fieldBase.cpp',
        'fieldAsset.cpp',
        'field3DAsset.cpp',
        'openVDBAsset.cpp',
        'tokens.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DUSDVOL_EXPORTS=1'],
    dependencies: [usd_base_dep],
    link_with: [usd_lib, usdGeom_lib, sdf_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_usdVol.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/usdVol/resources',
    configuration: cdata,
)

install_data(
    'generatedSchema.usda',
    install_dir: 'bin/pluginfo/usdVol/resources',
)
install_data(
    'schema.usda',
    install_dir: 'bin/pluginfo/usdVol/resources/usdVol',
)
