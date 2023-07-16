cpp = meson.get_compiler('cpp')

ar_lib = shared_library(
    'usd_ar',
    [
        # PUBLIC_CLASSES
        'asset.cpp',
        'assetInfo.cpp',
        'defaultResolver.cpp',
        'defaultResolverContext.cpp',
        'definePackageResolver.cpp',
        'defineResolver.cpp',
        'filesystemAsset.cpp',
        'filesystemWritableAsset.cpp',
        'inMemoryAsset.cpp',
        'notice.cpp',
        'packageResolver.cpp',
        'packageUtils.cpp',
        'resolvedPath.cpp',
        'resolver.cpp',
        'resolverContext.cpp',
        'resolverContextBinder.cpp',
        'resolverScopedCache.cpp',
        'timestamp.cpp',
        'writableAsset.cpp',

        # CPPFILES
        'debugCodes.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DAR_EXPORTS=1'],
    dependencies: [boost_dep, tbb_dep],
    link_with: [arch_lib, js_lib, tf_lib, plug_lib, vt_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_ar.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/ar/resources',
    configuration: cdata,
)
