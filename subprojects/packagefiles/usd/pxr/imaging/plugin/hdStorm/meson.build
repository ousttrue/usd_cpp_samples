cpp = meson.get_compiler('cpp')

hdStorm_lib = shared_library(
    'usd_hdStorm',
    [
        # PUBLIC_CLASSES
        'rendererPlugin.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DHDSTORM_EXPORTS=1'],
    dependencies: [usd_dep],
    link_with: [hd_lib, hdSt_lib, hf_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_hdStorm.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/hdStorm/resources',
    configuration: cdata,
)

