add_executable (curves
        main.cpp
        SceneDelegate.cpp SceneDelegate.h)

target_include_directories(curves
        PUBLIC
            ${USD_INC_DIR}
            ${PYTHON_INCLUDE_PATH}
        )

target_link_libraries(
        curves
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

install(TARGETS curves
        RUNTIME DESTINATION hydra-examples/bin
        )