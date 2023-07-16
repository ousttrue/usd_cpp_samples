subdir('usdImaging')
subdir('usdImagingGL')

usd_usdImaging_dep = declare_dependency(
    link_with: [usdImaging_lib, usdImagingGL_lib],
    dependencies: usd_imaging_dep,
)
