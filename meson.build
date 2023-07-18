project(
    'usd_cpp_samples',
    'cpp',
    default_options: ['cpp_std=c++latest'],
)

usd_usd_dep = dependency(
    'usd_usd',
    include_type: 'system',
    default_options: ['cpp_std=c++17'],
)
usd_imaging_dep = dependency(
    'usd_imaging',
    include_type: 'system',
    default_options: ['cpp_std=c++17'],
)
usd_usdImaging_dep = dependency(
    'usd_usdImaging',
    include_type: 'system',
    default_options: ['cpp_std=c++17'],
)
grapho_dep = dependency('grapho')
directxmath_dep = dependency('directxmath')

subdir('src')
