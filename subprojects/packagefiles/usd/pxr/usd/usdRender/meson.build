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
