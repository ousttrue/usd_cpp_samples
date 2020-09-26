add_executable (simpleObject
        main.cpp
        SceneDelegate.cpp SceneDelegate.h tiny_obj_loader.h)

target_include_directories(simpleObject
        PUBLIC
            ${USD_INC_DIR}
            ${PYTHON_INCLUDE_PATH}
        )

target_link_libraries(
        simpleObject
                ${USD_LIB_DIR}/libhd.so
                ${USD_LIB_DIR}/libhdStream.so
                ${USD_LIB_DIR}/libhdSt.so
                ${USD_LIB_DIR}/libhdx.so
                ${USD_LIB_DIR}/libtf.so
                ${USD_LIB_DIR}/libusd.so
                ${USD_LIB_DIR}/libsdf.so
                ${USD_LIB_DIR}/libgarch.so
                ${USD_LIB_DIR}/libtrace.so
                ${USD_LIB_DIR}/libcameraUtil.so
                ${USD_LIB_DIR}/libgf.so
                ${USD_LIB_DIR}/libglf.so
                ${USD_LIB_DIR}/libvt.so
                ${USD_LIB_DIR}/libpxOsd.so
                ${USD_LIB_DIR}/libosdCPU.so
                ${USD_LIB_DIR}/libboost_python.so 
                ${PYTHON_LIBRARY}
                ${OPENGL_LIBRARIES}
        )

install(FILES data/teapot/default.mtl data/teapot/default.png data/teapot/teapot.obj DESTINATION hydra-examples/bin)


install(TARGETS simpleObject
        RUNTIME DESTINATION hydra-examples/bin
        )