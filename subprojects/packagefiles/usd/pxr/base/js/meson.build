cpp = meson.get_compiler('cpp')

js_lib = shared_library(
    'usd_js',
    [
        # PUBLIC_CLASSES
        'json.cpp',
        'utils.cpp',
        'value.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + [
        '-DJS_EXPORTS=1',
    ],
    dependencies: [boost_dep],
    link_with: [tf_lib],
)
