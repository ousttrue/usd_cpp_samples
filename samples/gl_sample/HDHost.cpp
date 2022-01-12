#include "HDHost.h"
#include "GLEngine.h"

static int _GetRefineLevel(float c) {
  // TODO: Change complexity to refineLevel when we refactor UsdImaging.
  //
  // Convert complexity float to refine level int.
  int refineLevel = 0;

  // to avoid floating point inaccuracy (e.g. 1.3 > 1.3f)
  c = std::min(c + 0.01f, 2.0f);

  if (1.0f <= c && c < 1.1f) {
    refineLevel = 0;
  } else if (1.1f <= c && c < 1.2f) {
    refineLevel = 1;
  } else if (1.2f <= c && c < 1.3f) {
    refineLevel = 2;
  } else if (1.3f <= c && c < 1.4f) {
    refineLevel = 3;
  } else if (1.4f <= c && c < 1.5f) {
    refineLevel = 4;
  } else if (1.5f <= c && c < 1.6f) {
    refineLevel = 5;
  } else if (1.6f <= c && c < 1.7f) {
    refineLevel = 6;
  } else if (1.7f <= c && c < 1.8f) {
    refineLevel = 7;
  } else if (1.8f <= c && c <= 2.0f) {
    refineLevel = 8;
  } else {
    using namespace pxr;
    TF_CODING_ERROR("Invalid complexity %f, expected range is [1.0,2.0]\n", c);
  }
  return refineLevel;
}

HDHost::HDHost() {}

HDHost::~HDHost() { Shutdown(); }

void HDHost::Shutdown() {
  if (_sceneDelegate) {
    delete _sceneDelegate;
    _sceneDelegate = nullptr;
  }
  if (_engine) {
    delete _engine;
    _engine = nullptr;
  }
}

void HDHost::Load(const char *path) { _stage = pxr::UsdStage::Open(path); }

void HDHost::Draw(int width, int height) {
  if (!_engine) {
    _engine = new GLEngineImpl();
    _sceneDelegate = new pxr::UsdImagingDelegate(
        _engine->RenderIndex(), pxr::SdfPath::AbsoluteRootPath());
    // _sceneDelegate->SetUsdDrawModesEnabled(params.enableUsdDrawModes);
    pxr::SdfPathVector _excludedPrimPaths;
    _sceneDelegate->Populate(
        _stage->GetPrimAtPath(_stage->GetPseudoRoot().GetPath()),
        _excludedPrimPaths);
    pxr::SdfPathVector _invisedPrimPaths;
    _sceneDelegate->SetInvisedPrimPaths(_invisedPrimPaths);
  }

  // Set the fallback refine level, if this changes from the existing
  // value, all prim refine levels will be dirtied.
  const int refineLevel = _GetRefineLevel(/*params.complexity*/ 1.0f);
  _sceneDelegate->SetRefineLevelFallback(refineLevel);

  // Apply any queued up scene edits.
  _sceneDelegate->ApplyPendingUpdates();

  // XXX(UsdImagingPaths): Is it correct to map USD root path directly
  // to the cachePath here?
  // const SdfPath cachePath = root.GetPath();
  auto paths = {_sceneDelegate->ConvertCachePathToIndexPath(
      _stage->GetPseudoRoot().GetPath())};

  _engine->Draw(paths, width, height, _camera.ViewMatrix());
}

void HDHost::MousePress(int button, int x, int y, int modKeys) {
  _camera.MousePress(button, x, y, modKeys);
}

void HDHost::MouseRelease(int button, int x, int y, int modKeys) {
  _camera.MouseRelease(button, x, y, modKeys);
}

void HDHost::MouseMove(int x, int y, int modKeys) {
  _camera.MouseMove(x, y, modKeys);
}
