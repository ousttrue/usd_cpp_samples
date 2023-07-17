cpp = meson.get_compiler('cpp')

hdMtlx_lib = shared_library(
    'usd_hdMtlx',
    [
        # PUBLIC_CLASSES
    LIBRARIES
        gf
        hd
        sdf
        sdr
        tf
        trace
        usdMtlx
        vt
        MaterialXCore
        MaterialXFormat

    PUBLIC_CLASSES
        hdMtlx

    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DHDMTLX_EXPORTS=1'],
    dependencies: [usd_dep],
    link_with: [],
)
