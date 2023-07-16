project(
    'OpenSubdiv',
    'cpp',
    version: '3.5.0',
)

osd_inc = include_directories('.')

osd_lib = static_library(
    'osd',
    [
        'opensubdiv/sdc/crease.cpp',
        'opensubdiv/sdc/typeTraits.cpp',
        'opensubdiv/vtr/fvarLevel.cpp',
        'opensubdiv/vtr/fvarRefinement.cpp',
        'opensubdiv/vtr/level.cpp',
        'opensubdiv/vtr/quadRefinement.cpp',
        'opensubdiv/vtr/refinement.cpp',
        'opensubdiv/vtr/sparseSelector.cpp',
        'opensubdiv/vtr/triRefinement.cpp',
        'opensubdiv/far/bilinearPatchBuilder.cpp',
        'opensubdiv/far/catmarkPatchBuilder.cpp',
        'opensubdiv/far/error.cpp',
        'opensubdiv/far/loopPatchBuilder.cpp',
        'opensubdiv/far/patchBasis.cpp',
        'opensubdiv/far/patchBuilder.cpp',
        'opensubdiv/far/patchDescriptor.cpp',
        'opensubdiv/far/patchMap.cpp',
        'opensubdiv/far/patchTable.cpp',
        'opensubdiv/far/patchTableFactory.cpp',
        'opensubdiv/far/ptexIndices.cpp',
        'opensubdiv/far/stencilTable.cpp',
        'opensubdiv/far/stencilTableFactory.cpp',
        'opensubdiv/far/stencilBuilder.cpp',
        'opensubdiv/far/topologyDescriptor.cpp',
        'opensubdiv/far/topologyRefiner.cpp',
        'opensubdiv/far/topologyRefinerFactory.cpp',
        'opensubdiv/osd/cpuEvaluator.cpp',
        'opensubdiv/osd/cpuKernel.cpp',
        'opensubdiv/osd/cpuPatchTable.cpp',
        'opensubdiv/osd/cpuVertexBuffer.cpp',
    ],
    # install: true,
    include_directories: osd_inc,
    cpp_args: [
        '-DNOMINMAX',
        '-D_USE_MATH_DEFINES',
    ],
)

osd_dep = declare_dependency(
    link_with: osd_lib,
    include_directories: osd_inc,
)
