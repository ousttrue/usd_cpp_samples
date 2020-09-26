add_library(hdTiny SHARED
mesh.cpp
renderDelegate.cpp
rendererPlugin.cpp
renderPass.cpp
)
target_link_libraries(hdTiny
PRIVATE
    usd
)
set_target_properties(hdTiny
PROPERTIES
RUNTIME_OUTPUT_DIRECTORY_DEBUG "$ENV{USD_BUILD_DEBUG}/plugin/usd"
RUNTIME_OUTPUT_DIRECTORY_RELEASE "$ENV{USD_BUILD_RELEASE}/plugin/usd"
)
add_custom_command(TARGET hdTiny POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:hdTiny>/hdTiny/resources/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/plugInfo.json" "$<TARGET_FILE_DIR:hdTiny>/hdTiny/resources/"
)

add_executable(testHdTiny
    testenv/testHdTiny.cpp
)
target_link_libraries(testHdTiny
PRIVATE
    usd
)

# pxr_install_test_dir(
#     SRC testenv/testHdTiny
#     DEST testHdTiny
# )

# # This test only runs on MacOS and Linux for now.
# if (APPLE)
#     pxr_register_test(testHdTiny
#         ENV
#             DYLD_INSERT_LIBRARIES=${CMAKE_INSTALL_PREFIX}/share/usd/examples/plugin/hdTiny.dylib
#             PXR_PLUGINPATH_NAME=${CMAKE_INSTALL_PREFIX}/share/usd/examples/plugin/hdTiny/resources
#         COMMAND "${CMAKE_INSTALL_PREFIX}/tests/testHdTiny" 
#         STDOUT_REDIRECT output.txt
#         DIFF_COMPARE output.txt
#     )
# elseif (UNIX)
#     pxr_register_test(testHdTiny
#         ENV
#             LD_PRELOAD=${CMAKE_INSTALL_PREFIX}/share/usd/examples/plugin/hdTiny.so
#             PXR_PLUGINPATH_NAME=${CMAKE_INSTALL_PREFIX}/share/usd/examples/plugin/hdTiny/resources
#         COMMAND "${CMAKE_INSTALL_PREFIX}/tests/testHdTiny" 
#         STDOUT_REDIRECT output.txt
#         DIFF_COMPARE output.txt
#     )
# elseif (WIN32)
#     pxr_register_test(testHdTiny
#         ENV
#             PXR_PLUGINPATH_NAME=${CMAKE_INSTALL_PREFIX}/share/usd/examples/plugin/hdTiny/resources
#         PRE_PATH
#             ${CMAKE_INSTALL_PREFIX}/share/usd/examples/plugin/hdTiny
#         COMMAND "${CMAKE_INSTALL_PREFIX}/tests/testHdTiny" 
#         STDOUT_REDIRECT output.txt
#         DIFF_COMPARE output.txt
#     )
# endif()
