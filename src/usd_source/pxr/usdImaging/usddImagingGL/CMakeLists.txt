set(TARGET_NAME testUsdImagingGLBasicDrawing)
add_executable(${TARGET_NAME}
    unitTestGLDrawing.cpp
    testenv/testUsdImagingGLBasicDrawing.cpp
)
target_link_libraries(${TARGET_NAME}
PRIVATE
    usd
)
target_compile_options(${TARGET_NAME}
PRIVATE
    /wd4267
)
