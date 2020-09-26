add_executable(testUsdImagingGLBasicDrawing
    unitTestGLDrawing.cpp
    testenv/testUsdImagingGLBasicDrawing.cpp
)
target_link_libraries(testUsdImagingGLBasicDrawing
PRIVATE
    usd
)
