#pragma once

#include <pxr/base/gf/frustum.h>
#include <pxr/usd/sdf/path.h>
#include <array>
#include <functional>

PXR_NAMESPACE_OPEN_SCOPE
class UsdPrim;
class HdRenderIndex;
PXR_NAMESPACE_CLOSE_SCOPE

using CreateSceneDelegate = std::function<void(pxr::HdRenderIndex *)>;

struct RenderFrameInfo
{
    pxr::GfVec4d viewport;
    std::array<float, 1> clearDepth;
    pxr::GfMatrix4d modelViewMatrix;
    pxr::GfMatrix4d projectionMatrix;
};

class GLEngine
{
    class GLEngineImpl *_impl = nullptr;

    // Disallow copies
    GLEngine(const GLEngine &) = delete;
    GLEngine &operator=(const GLEngine &) = delete;

public:
    GLEngine(const CreateSceneDelegate &createSceneDelegate);
    ~GLEngine();
    uint32_t RenderFrame(const RenderFrameInfo &info, const pxr::SdfPathVector &paths);
};
