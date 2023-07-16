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
