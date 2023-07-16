cdata = configuration_data()

major_version = 0
minor_version = 23
patch_version = 5

cdata.set_quoted('PXR_MAJOR_VERSION', major_version.to_string())
cdata.set_quoted('PXR_MINOR_VERSION', minor_version.to_string())
cdata.set_quoted('PXR_PATCH_VERSION', patch_version.to_string())
pxr_version = major_version * 10000 + minor_version * 100 + patch_version
cdata.set_quoted('PXR_VERSION', pxr_version.to_string())
cdata.set('PXR_USE_NAMESPACES', 1)
cdata.set('PXR_EXTERNAL_NAMESPACE', 'pxr')
cdata.set(
    'PXR_INTERNAL_NAMESPACE',
    'pxrInternal_v' + major_version.to_string() + '_' + minor_version.to_string(),
)
cdata.set('PXR_PYTHON_SUPPORT_ENABLED', 0)
cdata.set('PXR_PREFER_SAFETY_OVER_SPEED', 1)

configure_file(
    input: 'pxr.h.in',
    output: 'pxr.h',
    configuration: cdata,
)

subdir('base')
subdir('usd')
subdir('imaging')
subdir('usdImaging')
