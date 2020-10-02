#pragma once

#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/imaging/hd/driver.h>
#include <pxr/usd/sdf/path.h>

PXR_NAMESPACE_OPEN_SCOPE
class UsdPrim;
PXR_NAMESPACE_CLOSE_SCOPE

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

    void Render(const pxr::UsdPrim &root,
                const pxr::UsdImagingGLRenderParams &params);
    
    void InvalidateBuffers();

    /// Returns true if the resulting image is fully converged.
    /// (otherwise, caller may need to call Render() again to refine the result)   
    bool IsConverged() const;

    void SetCameraState(const pxr::GfMatrix4d &viewMatrix,
                        const pxr::GfMatrix4d &projectionMatrix);
    void SetRenderViewport(pxr::GfVec4d const &viewport);                        
};
