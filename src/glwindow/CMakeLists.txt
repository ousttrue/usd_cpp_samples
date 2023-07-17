set(TARGET_NAME glwindow)
add_library(${TARGET_NAME}
    glWindow.cpp
    glPlatformDebugWindowWindows.cpp
)
target_include_directories(${TARGET_NAME}
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(${TARGET_NAME}
PUBLIC
    opengl32
)
