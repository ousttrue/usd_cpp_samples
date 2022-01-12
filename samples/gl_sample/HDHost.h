#pragma once
#include "Camera.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImaging/delegate.h>

class HDHost {
  pxr::UsdStageRefPtr _stage;
  pxr::UsdImagingDelegate *_sceneDelegate = nullptr;
  CameraView _camera;
  class GLEngineImpl *_engine = nullptr;

public:
  HDHost();
  ~HDHost();
  void Shutdown();
  void Load(const char *path);
  void Draw(int w, int h);
  void MousePress(int button, int x, int y, int modKeys);
  void MouseRelease(int button, int x, int y, int modKeys);
  void MouseMove(int x, int y, int modKeys);
};
