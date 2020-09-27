set(TARGET_NAME tiny)
# add_library(${TARGET_NAME} SHARED
# mesh.cpp
# renderDelegate.cpp
# rendererPlugin.cpp
# renderPass.cpp
# )
# target_link_libraries(${TARGET_NAME}
# PRIVATE
#     usd
# )
# set_target_properties(${TARGET_NAME}
# PROPERTIES
# RUNTIME_OUTPUT_DIRECTORY_DEBUG "$ENV{USD_BUILD_DEBUG}/plugin/usd"
# RUNTIME_OUTPUT_DIRECTORY_RELEASE "$ENV{USD_BUILD_RELEASE}/plugin/usd"
# )
# add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:hdTiny>/hdTiny/resources/"
#     COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/plugInfo.json" "$<TARGET_FILE_DIR:hdTiny>/hdTiny/resources/"
# )

add_executable(${TARGET_NAME}
    main.cpp
)
target_link_libraries(${TARGET_NAME}
PRIVATE
    usd
)
