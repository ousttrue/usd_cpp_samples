glfw_dep = dependency('glfw3', default_options: ['install=true'],)
executable(
    'MiniUsdImagingGLEngine',
    [
        'main.cpp',
    ],
    install: true,
    dependencies: [
        usd_usd_dep,
        usd_imaging_dep,
        usd_usdImaging_dep,
        glwindow_dep,
        glfw_dep,
    ],
)
