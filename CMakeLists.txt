cmake_minimum_required(VERSION 3.10.0)
project(usd_cpp_samples)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

subdirs(glfw usd)

subdirs(glwindow samples
        # usd_source
        IETutorials/hydra/helloWorld)

set(UT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/_external/usdtweak)

add_library(fontawesme INTERFACE)
target_include_directories(fontawesme
                           INTERFACE ${UT_DIR}/src/3rdparty/fontawesome)
add_library(iconfontcppheaders INTERFACE)
target_include_directories(iconfontcppheaders
                           INTERFACE ${UT_DIR}/src/3rdparty/iconfontcppheaders)
add_library(filesystem INTERFACE)
target_include_directories(filesystem
                           INTERFACE ${UT_DIR}/src/3rdparty/filesystem)

subdirs(${UT_DIR}/src/3rdparty/imgui)
add_executable(
  usdtweak
  ""
  ${UT_DIR}/src/Constants.h
  ${UT_DIR}/src/CommandLineOptions.h
  ${UT_DIR}/src/CommandLineOptions.cpp
  ${UT_DIR}/src/Editor.cpp
  ${UT_DIR}/src/Editor.h
  ${UT_DIR}/src/EditorSettings.cpp
  ${UT_DIR}/src/EditorSettings.h
  ${UT_DIR}/src/GeometricFunctions.h
  ${UT_DIR}/src/Gui.h
  ${UT_DIR}/src/ImGuiHelpers.h
  ${UT_DIR}/src/ProxyHelpers.h
  ${UT_DIR}/src/Selection.cpp
  ${UT_DIR}/src/Selection.h
  ${UT_DIR}/src/main.cpp
  ${UT_DIR}/src/window.cpp
  ${UT_DIR}/src/Dockspace.cpp
  ${UT_DIR}/src/commands/Commands.cpp
  ${UT_DIR}/src/commands/Commands.h
  ${UT_DIR}/src/commands/Shortcuts.h
  ${UT_DIR}/src/commands/SdfCommandGroup.cpp
  ${UT_DIR}/src/commands/SdfCommandGroup.h
  ${UT_DIR}/src/commands/SdfLayerInstructions.cpp
  ${UT_DIR}/src/commands/SdfLayerInstructions.h
  ${UT_DIR}/src/commands/SdfUndoRecorder.cpp
  ${UT_DIR}/src/commands/SdfUndoRecorder.h
  ${UT_DIR}/src/commands/UndoLayerStateDelegate.cpp
  ${UT_DIR}/src/commands/UndoLayerStateDelegate.h
  ${UT_DIR}/src/resources/glsl/GridVert.cpp
  ${UT_DIR}/src/resources/glsl/GridFrag.cpp
  ${UT_DIR}/src/resources/glsl/ManipulatorVert.cpp
  ${UT_DIR}/src/resources/glsl/ManipulatorFrag.cpp
  ${UT_DIR}/src/resources/ResourcesLoader.cpp
  ${UT_DIR}/src/resources/ResourcesLoader.h
  ${UT_DIR}/src/viewport/CameraManipulator.cpp
  ${UT_DIR}/src/viewport/CameraManipulator.h
  ${UT_DIR}/src/viewport/CameraRig.cpp
  ${UT_DIR}/src/viewport/CameraRig.h
  ${UT_DIR}/src/viewport/Grid.cpp
  ${UT_DIR}/src/viewport/Grid.h
  ${UT_DIR}/src/viewport/Manipulator.cpp
  ${UT_DIR}/src/viewport/Manipulator.h
  ${UT_DIR}/src/viewport/MouseHoverManipulator.cpp
  ${UT_DIR}/src/viewport/MouseHoverManipulator.h
  ${UT_DIR}/src/viewport/PositionManipulator.cpp
  ${UT_DIR}/src/viewport/PositionManipulator.h
  ${UT_DIR}/src/viewport/RotationManipulator.cpp
  ${UT_DIR}/src/viewport/RotationManipulator.h
  ${UT_DIR}/src/viewport/ScaleManipulator.cpp
  ${UT_DIR}/src/viewport/ScaleManipulator.h
  ${UT_DIR}/src/viewport/SelectionManipulator.cpp
  ${UT_DIR}/src/viewport/SelectionManipulator.h
  ${UT_DIR}/src/viewport/Viewport.cpp
  ${UT_DIR}/src/viewport/Viewport.h
  ${UT_DIR}/src/widgets/TextEditor.cpp
  ${UT_DIR}/src/widgets/TextEditor.h
  ${UT_DIR}/src/widgets/CompositionEditor.cpp
  ${UT_DIR}/src/widgets/CompositionEditor.h
  ${UT_DIR}/src/widgets/FileBrowser.cpp
  ${UT_DIR}/src/widgets/FileBrowser.h
  ${UT_DIR}/src/widgets/LayerEditor.cpp
  ${UT_DIR}/src/widgets/LayerEditor.h
  ${UT_DIR}/src/widgets/LayerHierarchyEditor.cpp
  ${UT_DIR}/src/widgets/LayerHierarchyEditor.h
  ${UT_DIR}/src/widgets/ModalDialogs.cpp
  ${UT_DIR}/src/widgets/ModalDialogs.h
  ${UT_DIR}/src/widgets/PrimSpecEditor.cpp
  ${UT_DIR}/src/widgets/PrimSpecEditor.h
  ${UT_DIR}/src/widgets/PropertyEditor.cpp
  ${UT_DIR}/src/widgets/PropertyEditor.h
  ${UT_DIR}/src/widgets/RendererSettings.cpp
  ${UT_DIR}/src/widgets/RendererSettings.h
  ${UT_DIR}/src/widgets/StageOutliner.cpp
  ${UT_DIR}/src/widgets/StageOutliner.h
  ${UT_DIR}/src/widgets/ContentBrowser.cpp
  ${UT_DIR}/src/widgets/ContentBrowser.h
  ${UT_DIR}/src/widgets/TextFilter.cpp
  ${UT_DIR}/src/widgets/TextFilter.h
  ${UT_DIR}/src/widgets/Timeline.cpp
  ${UT_DIR}/src/widgets/Timeline.h
  ${UT_DIR}/src/widgets/ValueEditor.cpp
  ${UT_DIR}/src/widgets/ValueEditor.h
  ${UT_DIR}/src/widgets/VariantEditor.cpp
  ${UT_DIR}/src/widgets/VariantEditor.h)
target_include_directories(usdtweak PRIVATE ${UT_DIR}/src)
target_link_libraries(usdtweak PRIVATE usd glfw iconfontcppheaders filesystem
                                       fontawesme)
target_compile_options(usdtweak PRIVATE /wd4996)
