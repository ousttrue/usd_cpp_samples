set(TARGET_NAME hd_sample)
add_executable(${TARGET_NAME}
    main.cpp
    testHdxIdRender.cpp
    unitTestDelegate.cpp
    SceneManager.cpp
)
target_link_libraries(${TARGET_NAME}
PRIVATE
    usd
)
target_compile_options(${TARGET_NAME}
PRIVATE
    /wd4267
)
