args = [
    '-DNOMINMAX',
    '/wd4244',
    '/wd4305',
    '/wd4251',
    '/wd4267',
    '/wd4804',
    '/wd4005',
    '/wd4273',
    '/wd4828',
]

subdir('arch')
subdir('tf')
subdir('js')
subdir('work')
subdir('gf')
subdir('trace')
subdir('vt')
subdir('plug')

usd_base_dep = declare_dependency(
    include_directories: pxr_inc,
    link_with: [
        arch_lib,
        tf_lib,
        js_lib,
        work_lib,
        gf_lib,
        trace_lib,
        vt_lib,
        plug_lib,
    ],
    dependencies: [boost_dep, tbb_dep],
    compile_args: args,
)
