# CMakeLists.txt 解読

```cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/cmake/defaults
                      ${CMAKE_SOURCE_DIR}/cmake/modules
                      ${CMAKE_SOURCE_DIR}/cmake/macros)

include(Options)
include(ProjectDefaults)
include(Packages)
```

## pxr_library

`cmake/macros/Public.cmake` で定義されている。

```cmake
function(pxr_library NAME)

```

`cmake/macros/Private.cmake` で定義されている。

```cmake
function(_pxr_python_module NAME)
```
