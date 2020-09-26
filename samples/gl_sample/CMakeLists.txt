set(TARGET_NAME gl_sample)
add_executable(${TARGET_NAME}
    main.cpp
    unitTestGLDrawing.cpp
    My_TestGLDrawing.cpp
)
target_link_libraries(${TARGET_NAME}
PRIVATE
    usd
)
target_compile_options(${TARGET_NAME}
PRIVATE
    /wd4267
)
