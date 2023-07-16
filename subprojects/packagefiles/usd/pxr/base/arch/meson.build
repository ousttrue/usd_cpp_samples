cpp = meson.get_compiler('cpp')
ws2_32 = cpp.find_library('Ws2_32', required: true)
dbghelp = cpp.find_library('Dbghelp', required: true)

arch_lib = shared_library(
    'usd_arch',
    [
        # PUBLIC_CLASSES
        'align.cpp',
        'attributes.cpp',
        'daemon.cpp',
        'debugger.cpp',
        'demangle.cpp',
        'env.cpp',
        'error.cpp',
        'errno.cpp',
        'fileSystem.cpp',
        'function.cpp',
        'hash.cpp',
        'library.cpp',
        'mallocHook.cpp',
        'regex.cpp',
        'stackTrace.cpp',
        'symbols.cpp',
        'systemInfo.cpp',
        'threads.cpp',
        'timing.cpp',
        'virtualMemory.cpp',
        'vsnprintf.cpp',

        # CPPFILES
        'assumptions.cpp',
        'initConfig.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DARCH_EXPORTS=1'],
    dependencies: [ws2_32, dbghelp],
)
