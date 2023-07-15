project('OpenUSD', 'cpp', version: '23.05')
add_project_arguments('-DNOMINMAX', language: 'cpp')
boost_dep = dependency('boost')
tbb_dep = subproject('tbb').get_variable('tbb_dep')
pxr_inc = include_directories('.')

# cpp = meson.get_compiler('cpp')
# shlwapi_lib = cpp.find_library('Shlwapi', required: true)
# ws2_32_lib = cpp.find_library('Ws2_32', required: true)
# dbghelp_lib = cpp.find_library('Dbghelp', required: true)
subdir('pxr')
args = ['-DNOMINMAX']
#     '-DPXR_STATIC',
#     '/wd4244',
#     '/wd4305',
#     '/wd4267',
#     '/wd4804',
#     '/wd4005',
#     '/wd5033',
#     '/bigobj',
#     '-DYY_NO_UNISTD_H',
# ]
# pxr_lib = static_library(
#     'usd',
#     base_src + usd_src,
#     include_directories: pxr_inc,
#     dependencies: [boost_dep, tbb_dep],
#     cpp_args: args,
# )

usd_dep = declare_dependency(
    include_directories: pxr_inc,
    # link_with: pxr_lib,
    dependencies: [boost_dep, tbb_dep],
    compile_args: args,
)
