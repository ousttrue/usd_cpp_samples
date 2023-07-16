cpp = meson.get_compiler('cpp')

usdGeom_lib = shared_library(
    'usd_usdGeom',
    [
    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DUSDGEOM_EXPORTS=1'],
    dependencies: [boost_dep, tbb_dep],
    link_with: [arch_lib, js_lib, tf_lib, plug_lib, vt_lib, sdf_lib, trace_lib, usd_lib, work_lib],
)    

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        debugCodes
        tokens
        bboxCache
        constraintTarget
        xformCache
        basisCurves
        boundable
        boundableComputeExtent
        camera
        capsule
        cone
        cube
        curves
        cylinder
        hermiteCurves
        imageable
        gprim
        mesh
        metrics
        modelAPI
        motionAPI
        nurbsCurves
        nurbsPatch
        plane
        pointBased
        pointInstancer
        points
        primvar
        primvarsAPI
        scope
        sphere
        subset
        visibilityAPI
        xform
        xformable
        xformOp
        xformCommonAPI
        
    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PRIVATE_CLASSES
        samplingUtils


