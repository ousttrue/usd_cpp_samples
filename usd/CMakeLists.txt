add_library(boost INTERFACE)
set(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/../build_debug)
target_include_directories(boost INTERFACE ${BUILD_DIR}/include/boost-1_77)

add_library(usd INTERFACE)
target_include_directories(usd INTERFACE ${BUILD_DIR}/include)
target_link_libraries(
  usd
  INTERFACE boost
            tbb.lib
            usd_ar.lib
            usd_arch.lib
            usd_cameraUtil.lib
            usd_garch.lib
            usd_gf.lib
            usd_glf.lib
            usd_hd.lib
            usd_hdSt.lib
            usd_hdx.lib
            usd_hf.lib
            usd_hgi.lib
            usd_hgiGL.lib
            usd_hgiInterop.lib
            usd_hio.lib
            usd_js.lib
            usd_kind.lib
            usd_ndr.lib
            usd_pcp.lib
            usd_plug.lib
            usd_pxOsd.lib
            usd_sdf.lib
            usd_sdr.lib
            usd_tf.lib
            usd_trace.lib
            usd_usd.lib
            usd_usdAppUtils.lib
            usd_usdGeom.lib
            usd_usdHydra.lib
            usd_usdImaging.lib
            usd_usdImagingGL.lib
            usd_usdLux.lib
            usd_usdMedia.lib
            usd_usdPhysics.lib
            usd_usdRender.lib
            usd_usdRi.lib
            usd_usdRiImaging.lib
            usd_usdShade.lib
            usd_usdSkel.lib
            usd_usdSkelImaging.lib
            usd_usdUI.lib
            usd_usdUtils.lib
            usd_usdVol.lib
            usd_usdVolImaging.lib
            usd_vt.lib
            usd_work.lib)
target_compile_definitions(usd INTERFACE NOMINMAX)
target_compile_options(usd INTERFACE /wd4244 /wd4305 /wd4275)
target_link_directories(usd INTERFACE ${BUILD_DIR}/lib)
