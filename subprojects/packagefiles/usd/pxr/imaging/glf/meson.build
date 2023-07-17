cpp = meson.get_compiler('cpp')

glf_lib = shared_library(
    'usd_glf',
    [
        # PUBLIC_CLASSES
        'bindingMap.cpp',
        'contextCaps.cpp',
        'diagnostic.cpp',
        'drawTarget.cpp',
        'glContext.cpp',
        'glRawContext.cpp',
        'info.cpp',
        'simpleLight.cpp',
        'simpleLightingContext.cpp',
        'simpleMaterial.cpp',
        'simpleShadowArray.cpp',
        'texture.cpp',
        'uniformBlock.cpp',
        'utils.cpp',

        # PRIVATE_CLASSES
        'debugCodes.cpp',
        'glContextRegistry.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DGLF_EXPORTS=1'],
    dependencies: [usd_dep, gl_dep],
    link_with: [garch_lib, hf_lib, hio_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_glf.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/glf/resources',
    configuration: cdata,
)

install_data(
    'shaders/pcfShader.glslfx',
    install_dir: 'bin/pluginfo/glf/resources/shaders',
)
install_data(
    'shaders/simpleLighting.glslfx',
    install_dir: 'bin/pluginfo/glf/resources/shaders',
)

