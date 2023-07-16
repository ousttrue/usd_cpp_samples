subdir('ar')
subdir('sdf')
subdir('pcp')
subdir('kind')
subdir('usd')
subdir('usdGeom')
subdir('ndr')
subdir('sdr')
subdir('usdShade')
subdir('usdLux')
subdir('usdVol')
subdir('usdRender')

usd_dep = declare_dependency(
    link_with: [
        ar_lib,
        sdf_lib,
        pcp_lib,
        kind_lib,
        usd_lib,
        usdGeom_lib,
        ndr_lib,
        sdr_lib,
        usdShade_lib,
        usdLux_lib,
        usdVol_lib,
        usdRender_lib,
    ],
    dependencies: [usd_base_dep],
    compile_args: args,
)
