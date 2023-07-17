cpp = meson.get_compiler('cpp')

usd_lib = shared_library(
    'usd_usd',
    [
        # PUBLIC_CLASSES
        'apiSchemaBase.cpp',
        'attribute.cpp',
        'clipsAPI.cpp',
        'attributeQuery.cpp',
        'collectionAPI.cpp',
        'collectionMembershipQuery.cpp',
        'common.cpp',
        'crateInfo.cpp',
        'debugCodes.cpp',
        'editContext.cpp',
        'editTarget.cpp',
        'errors.cpp',
        'flattenUtils.cpp',
        'inherits.cpp',
        'interpolation.cpp',
        'modelAPI.cpp',
        'notice.cpp',
        'object.cpp',
        'payloads.cpp',
        'prim.cpp',
        'primCompositionQuery.cpp',
        'primData.cpp',
        'primDataHandle.cpp',
        'primDefinition.cpp',
        'primFlags.cpp',
        'primRange.cpp',
        'primTypeInfo.cpp',
        'property.cpp',
        'references.cpp',
        'relationship.cpp',
        'resolveInfo.cpp',
        'resolveTarget.cpp',
        'resolver.cpp',
        'schemaBase.cpp',
        'schemaRegistry.cpp',
        'specializes.cpp',
        'stage.cpp',
        'stageCache.cpp',
        'stageCacheContext.cpp',
        'stageLoadRules.cpp',
        'stagePopulationMask.cpp',
        'timeCode.cpp',
        'tokens.cpp',
        'typed.cpp',
        'usdFileFormat.cpp',
        'usdaFileFormat.cpp',
        'usdcFileFormat.cpp',
        'usdzFileFormat.cpp',
        'variantSets.cpp',
        'zipFile.cpp',

        # PRIVATE_CLASSES
        'clip.cpp',
        'clipCache.cpp',
        'clipSet.cpp',
        'clipSetDefinition.cpp',
        'crateData.cpp',
        'crateFile.cpp',
        'instanceCache.cpp',
        'instanceKey.cpp',
        'integerCoding.cpp',
        'interpolators.cpp',
        'primTypeInfoCache.cpp',
        'usdzResolver.cpp',
        'valueUtils.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DUSD_EXPORTS=1', '/bigobj'],
    dependencies: [usd_base_dep],
    link_with: [ar_lib, kind_lib, pcp_lib, sdf_lib],
)

cdata = configuration_data()
cdata.set('PLUG_INFO_LIBRARY_PATH', '../../usd_usd.dll')
cdata.set('PLUG_INFO_RESOURCE_PATH', 'resources')
cdata.set('PLUG_INFO_ROOT', '..')
configure_file(
    input: 'plugInfo.json',
    output: 'plugInfo.json',
    install: true,
    install_dir: 'bin/pluginfo/usd/resources',
    configuration: cdata,
)

install_data(
    'generatedSchema.usda',
    install_dir: 'bin/pluginfo/usd/resources',
)
install_data(
    'schema.usda',
    install_dir: 'bin/pluginfo/usd/resources/usd',
)
