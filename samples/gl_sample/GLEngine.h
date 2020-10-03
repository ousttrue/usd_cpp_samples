#pragma once

#include <pxr/base/gf/frustum.h>
#include <pxr/imaging/hd/driver.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <array>

PXR_NAMESPACE_OPEN_SCOPE
class UsdPrim;
PXR_NAMESPACE_CLOSE_SCOPE

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
    GLEngine(const pxr::SdfPath &rootPath);
    ~GLEngine();
    uint32_t RenderFrame(const RenderFrameInfo &info, const pxr::UsdPrim &root);
};
