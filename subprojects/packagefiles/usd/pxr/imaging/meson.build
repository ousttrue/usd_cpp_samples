gl_dep = dependency('gl')

subdir('pxOsd')
subdir('cameraUtil')
subdir('hf')
subdir('hd')
subdir('garch')
subdir('geomUtil')
subdir('hdar')
subdir('hio')
subdir('hgi')
subdir('glf')
subdir('hdsi')
subdir('hgiGL')
subdir('hgiInterop')
subdir('hdSt')
subdir('hdx')
subdir('plugin/hdStorm')

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
        hgi_lib,
        glf_lib,
        hdsi_lib,
        hgiGL_lib,
        hgiInterop_lib,
        hdSt_lib,
        hdx_lib,
        # hdStorm_lib,
    ],
    dependencies: [usd_base_dep],
    compile_args: args,
)
