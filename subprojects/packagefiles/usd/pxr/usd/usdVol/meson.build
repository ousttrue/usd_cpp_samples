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
