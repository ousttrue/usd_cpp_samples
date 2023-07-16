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



