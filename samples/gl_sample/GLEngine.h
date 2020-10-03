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
    const pxr::UsdPrim &root;
    const pxr::UsdImagingGLRenderParams &params;

    RenderFrameInfo(const pxr::UsdPrim &prim, const pxr::UsdImagingGLRenderParams &glParams)
        : root(prim), params(glParams)
    {
    }
};

class GLEngine
{
    class GLEngineImpl *_impl = nullptr;

    // Disallow copies
    GLEngine(const GLEngine &) = delete;
    GLEngine &operator=(const GLEngine &) = delete;

public:
    GLEngine(
        const pxr::TfToken &renderID,
        const pxr::SdfPath &rootPath,
        const pxr::SdfPathVector &excludedPaths,
        const pxr::SdfPathVector &invisedPaths = pxr::SdfPathVector(),
        const pxr::SdfPath &sceneDelegateID =
            pxr::SdfPath::AbsoluteRootPath(),
        const pxr::HdDriver &driver = pxr::HdDriver());
    ~GLEngine();
    uint32_t RenderFrame(const RenderFrameInfo &info);
};
