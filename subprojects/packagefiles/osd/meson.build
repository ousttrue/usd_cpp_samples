project(
    'OpenSubdiv',
    'cpp',
    version: '3.5.0',
)
add_project_arguments('-DNOMINMAX', language: 'cpp')

glew_dep = dependency('glew')

osd_inc = include_directories('.')

subdir('tools/stringify')
subdir('glLoader')
subdir('opensubdiv')

# subdir('regression/common')
# subdir('examples/common')
# glfw_dep = dependency('glfw3', default_options: ['install=true'])
# subdir('examples/glViewer')
