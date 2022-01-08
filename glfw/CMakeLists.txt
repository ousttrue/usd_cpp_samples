set(GLFW_DIR ${CMAKE_CURRENT_LIST_DIR}/../_external/glfw)
set(SRC_DIR ${GLFW_DIR}/src)
set(TARGET_NAME glfw)
add_library(
  ${TARGET_NAME} SHARED
  ${SRC_DIR}/internal.h
  ${SRC_DIR}/platform.h
  ${SRC_DIR}/mappings.h
  ${SRC_DIR}/context.c
  ${SRC_DIR}/init.c
  ${SRC_DIR}/input.c
  ${SRC_DIR}/monitor.c
  ${SRC_DIR}/platform.c
  ${SRC_DIR}/vulkan.c
  ${SRC_DIR}/window.c
  ${SRC_DIR}/egl_context.c
  ${SRC_DIR}/osmesa_context.c
  ${SRC_DIR}/null_platform.h
  ${SRC_DIR}/null_joystick.h
  ${SRC_DIR}/null_init.c
  ${SRC_DIR}/null_monitor.c
  ${SRC_DIR}/null_window.c
  ${SRC_DIR}/null_joystick.c
  # win32
  ${SRC_DIR}/win32_platform.h
  ${SRC_DIR}/win32_joystick.h
  ${SRC_DIR}/win32_init.c
  ${SRC_DIR}/win32_joystick.c
  ${SRC_DIR}/win32_monitor.c
  ${SRC_DIR}/win32_window.c
  ${SRC_DIR}/wgl_context.c
  ${SRC_DIR}/win32_time.h
  ${SRC_DIR}/win32_thread.h
  ${SRC_DIR}/win32_module.c
  ${SRC_DIR}/win32_time.c
  ${SRC_DIR}/win32_thread.c)
target_compile_definitions(${TARGET_NAME} PRIVATE _GLFW_WIN32 _GLFW_BUILD_DLL
                                                  _CRT_SECURE_NO_WARNINGS)
target_include_directories(${TARGET_NAME} PUBLIC ${GLFW_DIR}/include)
