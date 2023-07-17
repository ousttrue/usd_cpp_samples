cpp = meson.get_compiler('cpp')

hio_lib = shared_library(
    'usd_hio',
    [
        # PUBLIC_CLASSES
        'fieldTextureData.cpp',
        'glslfx.cpp',
        'glslfxConfig.cpp',
        'image.cpp',
        'imageRegistry.cpp',
        'types.cpp',

        # PRIVATE_CLASSES
        'debugCodes.cpp',
        'dictionary.cpp',

        # CPPFILES
        'stbImage.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DHIO_EXPORTS=1'],
    dependencies: [usd_dep],
    link_with: [hf_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_hio.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/hio/resources',
    configuration: cdata,
)


