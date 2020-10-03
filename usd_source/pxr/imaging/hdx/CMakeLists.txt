set(TARGET_NAME testHdxIdRender)
add_executable(${TARGET_NAME}
    testenv/testHdxIdRender.cpp
    unitTestDelegate.cpp
)
target_link_libraries(${TARGET_NAME}
PRIVATE
    usd
)
target_compile_options(${TARGET_NAME}
PRIVATE
    /wd4267
)
