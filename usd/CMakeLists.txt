add_library(boost INTERFACE)
target_include_directories(
  boost INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../build_usd/include/boost-1_70)

add_library(usd INTERFACE)
target_include_directories(
  usd INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../build_usd/include)
target_link_libraries(
  usd
  INTERFACE boost
            tbb.lib
            #
            usd_sdf.lib
            usd_tf.lib
            usd_usd.lib
            #
            usd_cameraUtil.lib
            usd_glf.lib
            usd_plug.lib
            usd_hf.lib
            usd_arch.lib
            usd_hd.lib
            usd_hdx.lib
            usd_trace.lib
            usd_gf.lib
            usd_garch.lib
            usd_hgi.lib
            usd_hgiGL.lib
            usd_usdGeom.lib
            usd_usdImaging.lib
            usd_usdImagingGL.lib
            usd_usdHydra.lib
            usd_hdSt.lib
            usd_vt.lib
            usd_pxOsd.lib
            #
)
target_compile_definitions(usd INTERFACE NOMINMAX)
target_compile_options(usd INTERFACE /wd4244 /wd4305)
target_link_directories(usd INTERFACE
                        ${CMAKE_CURRENT_LIST_DIR}/../build_usd/lib)
