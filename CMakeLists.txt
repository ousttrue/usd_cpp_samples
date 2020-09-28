project(usd_cpp_samples)
cmake_minimum_required(VERSION 3.10.0)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin)

subdirs(
    usd
    samples/hello
    samples/gl_sample
    samples/tiny
    samples/tinyUSDHyrda
    usd_source/pxr/usdImaging/usddImagingGL
    usd_source/extras/imaging/examples/hdTiny
    IETutorials/hydra/helloWorld
)
