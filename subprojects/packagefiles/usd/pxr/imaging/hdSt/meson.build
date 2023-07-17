cpp = meson.get_compiler('cpp')

hdSt_lib = shared_library(
    'usd_hdSt',
    [
        # PUBLIC_CLASSES
        'basisCurves.cpp',
        'binding.cpp',
        'bufferArrayRange.cpp',
        'bufferResource.cpp',
        'commandBuffer.cpp',
        'copyComputation.cpp',
        'debugCodes.cpp',
        'dependencyForwardingSceneIndexPlugin.cpp',
        'dependencySceneIndexPlugin.cpp',
        'dispatchBuffer.cpp',
        'domeLightComputations.cpp',
        'drawItem.cpp',
        'drawItemInstance.cpp',
        'drawTarget.cpp',
        'drawTargetRenderPassState.cpp',
        'dynamicUvTextureObject.cpp',
        'extComputation.cpp',
        'extCompGpuComputation.cpp',
        'extCompGpuComputationResource.cpp',
        'extCompGpuPrimvarBufferSource.cpp',
        'fallbackLightingShader.cpp',
        'field.cpp',
        'fieldSubtextureIdentifier.cpp',
        'flatNormals.cpp',
        'geometricShader.cpp',
        'glConversions.cpp',
        'glslfxShader.cpp',
        'glslProgram.cpp',
        'bufferUtils.cpp',
        'hgiConversions.cpp',
        'hioConversions.cpp',
        'imageShaderRenderPass.cpp',
        'implicitSurfaceSceneIndexPlugin.cpp',
        'instancer.cpp',
        'interleavedMemoryManager.cpp',
        'light.cpp',
        'lightingShader.cpp',
        'material.cpp',
        'materialNetwork.cpp',
        'mesh.cpp',
        'package.cpp',
        'points.cpp',
        'primUtils.cpp',
        'ptexTextureObject.cpp',
        'renderBuffer.cpp',
        'renderDelegate.cpp',
        'renderParam.cpp',
        'renderPass.cpp',
        'renderPassShader.cpp',
        'renderPassState.cpp',
        'resourceRegistry.cpp',
        'samplerObject.cpp',
        'samplerObjectRegistry.cpp',
        'shaderCode.cpp',
        'simpleLightingShader.cpp',
        'smoothNormals.cpp',
        'stagingBuffer.cpp',
        'subtextureIdentifier.cpp',
        'textureBinder.cpp',
        'textureCpuData.cpp',
        'textureHandle.cpp',
        'textureHandleRegistry.cpp',
        'textureIdentifier.cpp',
        'textureObject.cpp',
        'textureObjectRegistry.cpp',
        'textureUtils.cpp',
        'tokens.cpp',
        'udimTextureObject.cpp',
        'unitTestGLDrawing.cpp',
        'vboMemoryManager.cpp',
        'vboSimpleMemoryManager.cpp',
        'volume.cpp',

        # PRIVATE_CLASSES
        'assetUvTextureCpuData.cpp',
        'basisCurvesComputations.cpp',
        'basisCurvesShaderKey.cpp',
        'basisCurvesTopology.cpp',
        'codeGen.cpp',
        'cullingShaderKey.cpp',
        'drawBatch.cpp',
        'drawItemsCache.cpp',
        'extCompComputeShader.cpp',
        'fieldTextureCpuData.cpp',
        'imageShaderShaderKey.cpp',
        'indirectDrawBatch.cpp',
        'materialNetworkShader.cpp',
        'materialBindingResolvingSceneIndexPlugin.cpp',
        'materialParam.cpp',
        'meshShaderKey.cpp',
        'meshTopology.cpp',
        'nodeIdentifierResolvingSceneIndex.cpp',
        'nodeIdentifierResolvingSceneIndexPlugin.cpp',
        'pipelineDrawBatch.cpp',
        'pointsShaderKey.cpp',
        'quadrangulate.cpp',
        'resourceBinder.cpp',
        'resourceLayout.cpp',
        'shaderKey.cpp',
        'subdivision.cpp',
        'terminalsResolvingSceneIndexPlugin.cpp',
        'triangulate.cpp',
        'unitTestHelper.cpp',
        'volumeShader.cpp',
        'volumeShaderKey.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + [
        '-DHDST_EXPORTS=1',
        '-DMFB_PACKAGE_NAME=hdSt',
        '-DMFB_ALT_PACKAGE_NAME=hdSt',
    ],
    dependencies: [usd_dep, osd_dep],
    link_with: [
        glf_lib,
        hgiGL_lib,
        # hdMtlx_lib,
        hgi_lib,
        hf_lib,
        cameraUtil_lib,
        hio_lib,
        garch_lib,
        glf_lib,
        hd_lib,
        hdsi_lib,
        hgiGL_lib,
        hgiInterop_lib,
        pxOsd_lib,
    ],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_hdSt.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/hdSt/resources',
    configuration: cdata,
)

install_data(
    'shaders/basisCurves.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/compute.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/domeLight.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/edgeId.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/fallbackLighting.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/fallbackLightingShader.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/fallbackMaterialNetwork.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/fallbackVolume.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/frustumCull.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/imageShader.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/instancing.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/mesh.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/meshFaceCull.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/meshNormal.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/meshWire.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/points.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/pointId.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/ptexTexture.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/renderPass.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/renderPassShader.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/simpleLightingShader.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/terminals.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/visibility.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'shaders/volume.glslfx',
    install_dir: 'bin/pluginfo/hdSt/resources/shaders',
)
install_data(
    'textures/fallbackBlackDomeLight.png',
    install_dir: 'bin/pluginfo/hdSt/resources/textures',
)
#
#
