subdir('pxOsd')
subdir('cameraUtil')
subdir('hf')
subdir('hd')
subdir('garch')
subdir('geomUtil')
subdir('hdar')
subdir('hio')

usd_imaging_dep = declare_dependency(
    link_with: [
        pxOsd_lib,
        hd_lib,
        cameraUtil_lib,
        hf_lib,
        garch_lib,
        geomUtil_lib,
        hdar_lib,
        hio_lib,
    ],
    dependencies: [usd_base_dep],
    compile_args: args,
)
