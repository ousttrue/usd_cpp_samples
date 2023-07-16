osd_dep = dependency('osd')

subdir('pxOsd')
subdir('hd')

usd_imaging_dep = declare_dependency(
    link_with: [pxOsd_lib, hd_lib],
    dependencies: [usd_dep, osd_dep],
    compile_args: args,
)
