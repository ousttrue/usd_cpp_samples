cpp = meson.get_compiler('cpp')

geomUtil_lib = shared_library(
    'usd_geomUtil',
    [
        # PUBLIC_CLASSES
        'capsuleMeshGenerator.cpp',
        'coneMeshGenerator.cpp',
        'cuboidMeshGenerator.cpp',
        'cylinderMeshGenerator.cpp',
        'meshGeneratorBase.cpp',
        'sphereMeshGenerator.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DGEOMUTIL_EXPORTS=1'],
    dependencies: [usd_base_dep],
    link_with: [pxOsd_lib],
)
