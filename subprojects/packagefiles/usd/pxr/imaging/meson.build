subdir('pxOsd')
subdir('cameraUtil')
subdir('hf')
subdir('hd')

usd_imaging_dep = declare_dependency(
    link_with: [pxOsd_lib, hd_lib, cameraUtil_lib, hf_lib],
    dependencies: [usd_dep],
    compile_args: args,
)
