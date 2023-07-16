project('OpenUSD', 'cpp', version: '23.05')
add_project_arguments('-DNOMINMAX', language: 'cpp')
boost_dep = dependency('boost')
tbb_dep = subproject('tbb').get_variable('tbb_dep')
pxr_inc = include_directories('.', is_system: true)

# cpp = meson.get_compiler('cpp')
# shlwapi_lib = cpp.find_library('Shlwapi', required: true)
# ws2_32_lib = cpp.find_library('Ws2_32', required: true)
# dbghelp_lib = cpp.find_library('Dbghelp', required: true)
args = ['-DNOMINMAX']
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
    ],
    dependencies: [boost_dep, tbb_dep],
    compile_args: args,
)
