set(TARGET_NAME testHdxIdRender)
add_executable(
  ${TARGET_NAME} ${USD_SOURCE_DIR}/pxr/imaging/hdx/testenv/testHdxIdRender.cpp
                 ${USD_SOURCE_DIR}/pxr/imaging/hdx/unitTestDelegate.cpp)
target_include_directories(${TARGET_NAME} PRIVATE ${USD_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE usd)
target_compile_options(${TARGET_NAME} PRIVATE /wd4267)
