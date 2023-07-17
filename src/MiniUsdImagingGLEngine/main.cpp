#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec4f.h"
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/usd/usd/common.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/stageCache.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>

//
#include <Windows.h>
//
#include <GL/GL.h>
//
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//
#include <iostream>

class Window {
  GLFWwindow *_window = nullptr;

public:
  Window() {
    if (!glfwInit()) {
      throw "glfwInit";
    }
  }

  ~Window() {
    if (_window) {
      glfwDestroyWindow(_window);
    }

    glfwTerminate();
  }

  static void error_callback(int error, const char *description) {
    std::cerr << error << ": " << description << std::endl;
  }

  bool Create(int w, int h, const char *title) {
    glfwSetErrorCallback(error_callback);

    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    _window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!_window) {
      return false;
    }
    glfwMakeContextCurrent(_window);

    glfwSwapInterval(1);

    return true;
  }

  bool IsRunning(int *pW, int *pH) {
    if (glfwWindowShouldClose(_window)) {
      return false;
    }
    glfwGetFramebufferSize(_window, pW, pH);
    glfwPollEvents();
    return true;
  }

  void SwapBuffers() { glfwSwapBuffers(_window); }
};

int main(int argc, char **argv) {
  Window w;
  if (!w.Create(1920, 1080, "UsdImagingGLEngine")) {
    return 1;
  }

  // load usd
  pxr::UsdStageRefPtr stage;
  if (argc > 1) {
    stage = pxr::UsdStage::Open(argv[1]);
  }
  if (!stage) {
    return 2;
  }

  // camera
  pxr::GfCamera camera;
  float dist = 100.0f;
  pxr::GfMatrix4d t;
  t.SetTranslate(pxr::GfVec3d::ZAxis() * dist);
  camera.SetTransform(t);
  camera.SetFocusDistance(dist);

  // renderer
  pxr::UsdImagingGLEngine engine;
  // param
  pxr::UsdImagingGLRenderParams params;
  params.enableLighting = false;

  int width, height;
  while (w.IsRunning(&width, &height)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    engine.SetRenderViewport(pxr::GfVec4d(0, 0, width, height));
    engine.Render(stage->GetPseudoRoot(), params);
    engine.SetCameraState(camera.GetFrustum().ComputeViewMatrix(),
                          camera.GetFrustum().ComputeProjectionMatrix());

    w.SwapBuffers();
  }

  return 0;
}
