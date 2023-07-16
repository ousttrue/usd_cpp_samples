cpp = meson.get_compiler('cpp')

hgiGL_lib = shared_library(
    'usd_hgiGL',
    [
        # PRIVATE_CLASSES
        'conversions.cpp',
        'diagnostic.cpp',
        'scopedStateHolder.cpp',
        'shaderGenerator.cpp',
        'shaderSection.cpp',

        # PUBLIC_CLASSES
        'blitCmds.cpp',
        'buffer.cpp',
        'capabilities.cpp',
        'computeCmds.cpp',
        'computePipeline.cpp',
        'contextArena.cpp',
        'debugCodes.cpp',
        'device.cpp',
        'garbageCollector.cpp',
        'graphicsCmds.cpp',
        'graphicsPipeline.cpp',
        'hgi.cpp',
        'ops.cpp',
        'resourceBindings.cpp',
        'sampler.cpp',
        'shaderFunction.cpp',
        'shaderProgram.cpp',
        'texture.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DHGIGL_EXPORTS=1'],
    dependencies: [usd_dep],
    link_with: [garch_lib, hgi_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_hgiGL.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/hgiGL/resources',
    configuration: cdata,
)


