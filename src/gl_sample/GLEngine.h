#pragma once
#include "pxr/base/gf/matrix4d.h"
#include "pxr/usd/sdf/path.h"
#include "pxr/usd/usd/stage.h"
#include <cstddef>
#include <iostream>
#include <pxr/base/arch/systemInfo.h>
#include <pxr/base/plug/registry.h>
#include <pxr/base/tf/getenv.h>
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/imaging/glf/diagnostic.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/imaging/glf/glContext.h>
#include <pxr/imaging/glf/simpleLightingContext.h>
#include <pxr/imaging/hd/driver.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/imaging/hdx/taskController.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgi/tokens.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>

class GLEngineImpl {
  pxr::HgiUniquePtr _hgi;
  pxr::HdDriver _hgiDriver;

  std::unique_ptr<pxr::HdxTaskController> _taskController;
  pxr::HdPluginRenderDelegateUniqueHandle _renderDelegate;
  std::unique_ptr<pxr::HdRenderIndex> _renderIndex;
  std::unique_ptr<pxr::HdEngine> _engine;
  pxr::HdxSelectionTrackerSharedPtr _selTracker;
  pxr::GfVec4f _selectionColor = {1.0f, 1.0f, 0.0f, 1.0f};
  pxr::HdRprimCollection _renderCollection;
  pxr::GlfDrawTargetRefPtr _drawTarget;

public:
  GLEngineImpl();
  ~GLEngineImpl();
  pxr::HdRenderIndex *RenderIndex() { return _renderIndex.get(); }

  void Draw(
      // const pxr::UsdStageRefPtr &stage,
      const pxr::SdfPathVector &paths, int width, int height,
      const pxr::GfMatrix4d &viewMatrix);

private:
  uint32_t RenderFrame(const struct RenderFrameInfo &info,
                       const pxr::SdfPathVector &paths);
  void Render(const pxr::UsdImagingGLRenderParams &params,
              const pxr::SdfPathVector &paths);
  bool IsConverged();

  void SetCameraState(const pxr::GfMatrix4d &viewMatrix,
                      const pxr::GfMatrix4d &projectionMatrix);

  void SetRenderViewport(pxr::GfVec4d const &viewport);

  pxr::SdfPath _ComputeControllerPath(
      const pxr::HdPluginRenderDelegateUniqueHandle &renderDelegate);

  void SetColorCorrectionSettings(pxr::TfToken const &id);

  /* static */
  bool _UpdateHydraCollection(pxr::HdRprimCollection *collection,
                              pxr::SdfPathVector const &roots,
                              pxr::UsdImagingGLRenderParams const &params);
  void _Execute(const pxr::UsdImagingGLRenderParams &params,
                pxr::HdTaskSharedPtrVector tasks);
};
