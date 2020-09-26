add_library(usd INTERFACE)
target_include_directories(usd INTERFACE
"$<$<CONFIG:Debug>:$ENV{USD_BUILD_DEBUG}/include>"
"$<$<CONFIG:RelWithDebInfo>:$ENV{USD_BUILD_RELEASE}/include>"
"$<$<CONFIG:Release>:$ENV{USD_BUILD_RELEASE}/include>"
"$<$<CONFIG:MinSizeRel>:$ENV{USD_BUILD_RELEASE}/include>"
"$<$<CONFIG:Debug>:$ENV{USD_BUILD_DEBUG}/include/boost-1_70>"
"$<$<CONFIG:RelWithDebInfo>:$ENV{USD_BUILD_RELEASE}/include/boost-1_70>"
"$<$<CONFIG:Release>:$ENV{USD_BUILD_RELEASE}/include/boost-1_70>"
"$<$<CONFIG:MinSizeRel>:$ENV{USD_BUILD_RELEASE}/include/boost-1_70>"
)
target_compile_definitions(usd INTERFACE
NOMINMAX
)
target_compile_options(usd INTERFACE
/wd4244
/wd4305
)
target_link_libraries(usd INTERFACE
"$<$<CONFIG:Debug>:$ENV{USD_BUILD_DEBUG}/lib/tbb_debug.lib>"
"$<$<CONFIG:RelWithDebInfo>:$ENV{USD_BUILD_RELEASE}/lib/tbb.lib>"
"$<$<CONFIG:Release>:$ENV{USD_BUILD_RELEASE}/lib/tbb.lib>"
"$<$<CONFIG:MinSizeRel>:$ENV{USD_BUILD_RELEASE}/lib/tbb.lib>"

"$<$<CONFIG:Debug>:$ENV{USD_BUILD_DEBUG}/lib/usd_ms.lib>"
"$<$<CONFIG:RelWithDebInfo>:$ENV{USD_BUILD_RELEASE}/lib/usd_ms.lib>"
"$<$<CONFIG:Release>:$ENV{USD_BUILD_RELEASE}/lib/usd_ms.lib>"
"$<$<CONFIG:MinSizeRel>:$ENV{USD_BUILD_RELEASE}/lib/usd_ms.lib>"

"$<$<CONFIG:Debug>:$ENV{USD_BUILD_DEBUG}/lib/glew32.lib>"
"$<$<CONFIG:RelWithDebInfo>:$ENV{USD_BUILD_RELEASE}/lib/glew32.lib>"
"$<$<CONFIG:Release>:$ENV{USD_BUILD_RELEASE}/lib/glew32.lib>"
"$<$<CONFIG:MinSizeRel>:$ENV{USD_BUILD_RELEASE}/lib/glew32.lib>"

OpenGL32
)
