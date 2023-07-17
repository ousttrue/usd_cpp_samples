set(TARGET_NAME UsdImagingGLEngine)
add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE glfw usd OpenGL32.lib)
