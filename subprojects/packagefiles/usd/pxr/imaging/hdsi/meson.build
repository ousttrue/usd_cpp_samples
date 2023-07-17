cpp = meson.get_compiler('cpp')

hdsi_lib = shared_library(
    'usd_hdsi',
    [
        # PUBLIC_CLASSES
        'extComputationPrimvarPruningSceneIndex.cpp',
        'implicitSurfaceSceneIndex.cpp',
        'materialBindingResolvingSceneIndex.cpp',
        'materialPrimvarTransferSceneIndex.cpp',
        'pinnedCurveExpandingSceneIndex.cpp',
        'terminalsResolvingSceneIndex.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DHDSI_EXPORTS=1'],
    dependencies: [usd_dep],
    link_with: [cameraUtil_lib, geomUtil_lib, hf_lib, hd_lib, pxOsd_lib],
)
