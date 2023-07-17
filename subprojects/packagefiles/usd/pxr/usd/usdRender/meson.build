cpp = meson.get_compiler('cpp')

usdRender_lib = shared_library(
    'usd_usdRender',
    [
        # PUBLIC_CLASSES
        'denoisePass.cpp',
        'pass.cpp',
        'product.cpp',
        'settings.cpp',
        'settingsBase.cpp',
        'spec.cpp',
        'tokens.cpp',
        'var.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DUSDRENDER_EXPORTS=1'],
    dependencies: [usd_base_dep],
    link_with: [sdf_lib, usd_lib, usdGeom_lib, usdShade_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_usdRender.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/usdRender/resources',
    configuration: cdata,
)

install_data(
    'generatedSchema.usda',
    install_dir: 'bin/pluginfo/usdRender/resources',
)
install_data(
    'schema.usda',
    install_dir: 'bin/pluginfo/usdRender/resources/usdRender',
)

