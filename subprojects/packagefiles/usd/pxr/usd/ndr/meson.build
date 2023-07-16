cpp = meson.get_compiler('cpp')

ndr_lib = shared_library(
    'usd_ndr',
    [
        # PUBLIC_CLASSES
        'debugCodes.cpp',
        'declare.cpp',
        'discoveryPlugin.cpp',
        'filesystemDiscovery.cpp',
        'filesystemDiscoveryHelpers.cpp',
        'node.cpp',
        'parserPlugin.cpp',
        'property.cpp',
        'registry.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DNDR_EXPORTS=1'],
    dependencies: [boost_dep, tbb_dep],
    link_with: [
        arch_lib,
        tf_lib,
        plug_lib,
        vt_lib,
        trace_lib,
        work_lib,
        ar_lib,
        sdf_lib,
    ],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_ndr.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/ndr/resources',
    configuration: cdata,
)
