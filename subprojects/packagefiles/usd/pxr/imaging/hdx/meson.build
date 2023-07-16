cpp = meson.get_compiler('cpp')

hdx_lib = shared_library(
    'usd_hdx',
    [
        # PUBLIC_CLASSES
        'aovInputTask.cpp',
        'boundingBoxTask.cpp',
        'colorizeSelectionTask.cpp',
        'colorChannelTask.cpp',
        'colorCorrectionTask.cpp',
        'drawTargetTask.cpp',
        'freeCameraSceneDelegate.cpp',
        'fullscreenShader.cpp',
        'hgiConversions.cpp',
        'oitBufferAccessor.cpp',
        'oitRenderTask.cpp',
        'oitResolveTask.cpp',
        'oitVolumeRenderTask.cpp',
        'package.cpp',
        'pickTask.cpp',
        'pickFromRenderBufferTask.cpp',
        'presentTask.cpp',
        'renderTask.cpp',
        'renderSetupTask.cpp',
        'selectionSceneIndexObserver.cpp',
        'selectionTask.cpp',
        'selectionTracker.cpp',
        'shadowTask.cpp',
        'simpleLightTask.cpp',
        'skydomeTask.cpp',
        'task.cpp',
        'taskController.cpp',
        'tokens.cpp',
        'types.cpp',
        'visualizeAovTask.cpp',

        # PRIVATE_CLASSES
        'debugCodes.cpp',
        'unitTestDelegate.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DHDX_EXPORTS=1'],
    dependencies: [usd_dep],
    link_with: [
        garch_lib,
        glf_lib,
        pxOsd_lib,
        hd_lib,
        hdSt_lib,
        hgi_lib,
        hgiInterop_lib,
        hio_lib,
        hgiGL_lib,
        cameraUtil_lib,
    ],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_hdx.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/hdx/resources',
    configuration: cdata,
)

install_data(
    'shaders/boundingBox.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/colorChannel.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/colorCorrection.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/fullscreen.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/oitResolveImageShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/outline.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPass.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPassColorShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPassColorAndSelectionShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPassColorWithOccludedSelectionShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPassIdShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPassPickingShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPassOitShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPassOitOpaqueShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPassOitVolumeShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/renderPassShadowShader.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/selection.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/skydome.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'shaders/visualize.glslfx',
    install_dir: 'bin/pluginfo/hdx/resources/shaders',
)
install_data(
    'textures/StinsonBeach.hdr',
    install_dir: 'bin/pluginfo/hdx/resources/textures',
)
install_data(
    'textures/StinsonBeach.tex',
    install_dir: 'bin/pluginfo/hdx/resources/textures',
)

