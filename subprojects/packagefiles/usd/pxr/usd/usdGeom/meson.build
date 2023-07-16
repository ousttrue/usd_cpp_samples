cpp = meson.get_compiler('cpp')

usdGeom_lib = shared_library(
    'usd_usdGeom',
    [
        # PUBLIC_CLASSES
        'debugCodes.cpp',
        'tokens.cpp',
        'bboxCache.cpp',
        'constraintTarget.cpp',
        'xformCache.cpp',
        'basisCurves.cpp',
        'boundable.cpp',
        'boundableComputeExtent.cpp',
        'camera.cpp',
        'capsule.cpp',
        'cone.cpp',
        'cube.cpp',
        'curves.cpp',
        'cylinder.cpp',
        'hermiteCurves.cpp',
        'imageable.cpp',
        'gprim.cpp',
        'mesh.cpp',
        'metrics.cpp',
        'modelAPI.cpp',
        'motionAPI.cpp',
        'nurbsCurves.cpp',
        'nurbsPatch.cpp',
        'plane.cpp',
        'pointBased.cpp',
        'pointInstancer.cpp',
        'points.cpp',
        'primvar.cpp',
        'primvarsAPI.cpp',
        'scope.cpp',
        'sphere.cpp',
        'subset.cpp',
        'visibilityAPI.cpp',
        'xform.cpp',
        'xformable.cpp',
        'xformOp.cpp',
        'xformCommonAPI.cpp',

        # PRIVATE_CLASSES
        'samplingUtils.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DUSDGEOM_EXPORTS=1'],
    dependencies: [boost_dep, tbb_dep],
    link_with: [
        arch_lib,
        js_lib,
        tf_lib,
        plug_lib,
        vt_lib,
        trace_lib,
        work_lib,
        kind_lib,
        gf_lib,
        #
        sdf_lib,
        usd_lib,
    ],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_usdGeom.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/usdGeom/resources',
    configuration: cdata,
)
