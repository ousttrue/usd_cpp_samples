cpp = meson.get_compiler('cpp')

usdImagingGL_lib = shared_library(
    'usd_imagingGL',
    [
        # PUBLIC_CLASSES
        'engine.cpp',
        'renderParams.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DIMAGINGGL_EXPORTS=1'],
    dependencies: [usd_dep, usd_imaging_dep],
    link_with: [usdImaging_lib],
)

