project('OpenUSD', 'cpp', version: '23.05')
add_project_arguments('-DNOMINMAX', language: 'cpp')
boost_dep = dependency('boost')
tbb_dep = subproject('tbb').get_variable('tbb_dep')
pxr_inc = include_directories('.', is_system: true)

args = [
    '/wd4244',
    '/wd4305',
    '/wd4251',
]
subdir('pxr')
usd_dep = declare_dependency(
    include_directories: pxr_inc,
    link_with: [
        # base
        arch_lib,
        tf_lib,
        js_lib,
        work_lib,
        plug_lib,
        # usd
        ar_lib,
        sdf_lib,
        pcp_lib,
        kind_lib,
        usd_lib,
        usdGeom_lib,
    ],
    dependencies: [boost_dep, tbb_dep],
    compile_args: ['-DNOMINMAX'],
)
