osd_lib = static_library(
    'osd',
    [
        # core
        'sdc/crease.cpp',
        'sdc/typeTraits.cpp',
        # vtr
        'vtr/fvarLevel.cpp',
        'vtr/fvarRefinement.cpp',
        'vtr/level.cpp',
        'vtr/quadRefinement.cpp',
        'vtr/refinement.cpp',
        'vtr/sparseSelector.cpp',
        'vtr/triRefinement.cpp',
        # far
        'far/bilinearPatchBuilder.cpp',
        'far/catmarkPatchBuilder.cpp',
        'far/error.cpp',
        'far/loopPatchBuilder.cpp',
        'far/patchBasis.cpp',
        'far/patchBuilder.cpp',
        'far/patchDescriptor.cpp',
        'far/patchMap.cpp',
        'far/patchTable.cpp',
        'far/patchTableFactory.cpp',
        'far/ptexIndices.cpp',
        'far/stencilTable.cpp',
        'far/stencilTableFactory.cpp',
        'far/stencilBuilder.cpp',
        'far/topologyDescriptor.cpp',
        'far/topologyRefiner.cpp',
        'far/topologyRefinerFactory.cpp',
        # osd
        'osd/cpuEvaluator.cpp',
        'osd/cpuKernel.cpp',
        'osd/cpuPatchTable.cpp',
        'osd/cpuVertexBuffer.cpp',
        # osd:gl
        'osd/cpuGLVertexBuffer.cpp',
        'osd/glLegacyGregoryPatchTable.cpp',
        'osd/glPatchTable.cpp',
        'osd/glVertexBuffer.cpp',
        'osd/glslPatchShaderSource.cpp',
    ],
    include_directories: osd_inc,
    cpp_args: ['-D_USE_MATH_DEFINES'],
    dependencies: [glloader_dep],
)

osd_dep = declare_dependency(
    include_directories: osd_inc,
    link_with: osd_lib,
    compile_args: ['-DOPENSUBDIV_VERSION_STRING="3.5"'],
)
