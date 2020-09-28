//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//

/// \file usdImagingGL/engine.h
#pragma once

#include "pxr/pxr.h"
#include "pxr/usdImaging/usdImagingGL/api.h"
#include "pxr/usdImaging/usdImagingGL/version.h"
#include "pxr/usdImaging/usdImaging/version.h"

#include "pxr/usdImaging/usdImagingGL/renderParams.h"
#include "pxr/usdImaging/usdImagingGL/rendererSettings.h"

#include "pxr/imaging/cameraUtil/conformWindow.h"

#include "pxr/imaging/hd/driver.h"
#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hd/rprimCollection.h"
#include "pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h"

#include "pxr/imaging/hdx/selectionTracker.h"
#include "pxr/imaging/hdx/renderSetupTask.h"
#include "pxr/imaging/hdx/pickTask.h"

#include "pxr/imaging/hgi/hgi.h"

#include "pxr/imaging/glf/drawTarget.h"
#include "pxr/imaging/glf/simpleLight.h"
#include "pxr/imaging/glf/simpleMaterial.h"

#include "pxr/imaging/hgi/hgi.h"

#include "pxr/usd/sdf/path.h"
#include "pxr/usd/usd/timeCode.h"

#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec4d.h"
#include "pxr/base/gf/vec4f.h"
#include "pxr/base/gf/vec4i.h"

#include "pxr/base/vt/dictionary.h"

#include "pxr/base/tf/declarePtrs.h"

PXR_NAMESPACE_OPEN_SCOPE

class UsdPrim;
class HdRenderIndex;
class HdxTaskController;
class UsdImagingDelegate;
class UsdImagingGLLegacyEngine;
TF_DECLARE_WEAK_AND_REF_PTRS(GlfSimpleLightingContext);

PXR_NAMESPACE_CLOSE_SCOPE

/// \class GLEngine
///
/// The GLEngine is the main entry point API for rendering USD scenes.
///
class GLEngine
{
public:
    // ---------------------------------------------------------------------
    /// \name Global State
    /// @{
    // ---------------------------------------------------------------------

    /// Returns true if Hydra is enabled for GL drawing.
    
    static bool IsHydraEnabled();

    /// @}

    // ---------------------------------------------------------------------
    /// \name Construction
    /// @{
    // ---------------------------------------------------------------------

    /// A HdDriver, containing the Hgi of your choice, can be optionally passed
    /// in during construction. This can be helpful if you application creates
    /// multiple GLEngine that wish to use the same HdDriver / Hgi.
    
    GLEngine(const pxr::HdDriver &driver = pxr::HdDriver());

    
    GLEngine(const pxr::SdfPath &rootPath,
             const pxr::SdfPathVector &excludedPaths,
             const pxr::SdfPathVector &invisedPaths = pxr::SdfPathVector(),
             const pxr::SdfPath &sceneDelegateID =
                 pxr::SdfPath::AbsoluteRootPath(),
             const pxr::HdDriver &driver = pxr::HdDriver());

    // Disallow copies
    GLEngine(const GLEngine &) = delete;
    GLEngine &operator=(const GLEngine &) = delete;

    
    ~GLEngine();

    /// @}

    // ---------------------------------------------------------------------
    /// \name Rendering
    /// @{
    // ---------------------------------------------------------------------

    /// Support for batched drawing
    
    void PrepareBatch(const pxr::UsdPrim &root,
                      const pxr::UsdImagingGLRenderParams &params);
    
    void RenderBatch(const pxr::SdfPathVector &paths,
                     const pxr::UsdImagingGLRenderParams &params);

    /// Entry point for kicking off a render
    
    void Render(const pxr::UsdPrim &root,
                const pxr::UsdImagingGLRenderParams &params);

    
    void InvalidateBuffers();

    /// Returns true if the resulting image is fully converged.
    /// (otherwise, caller may need to call Render() again to refine the result)
    
    bool IsConverged() const;

    /// @}

    // ---------------------------------------------------------------------
    /// \name Root Transform and Visibility
    /// @{
    // ---------------------------------------------------------------------

    /// Sets the root transform.
    
    void SetRootTransform(pxr::GfMatrix4d const &xf);

    /// Sets the root visibility.
    
    void SetRootVisibility(bool isVisible);

    /// @}

    // ---------------------------------------------------------------------
    /// \name Camera State
    /// @{
    // ---------------------------------------------------------------------

    /// Set the viewport to use for rendering as (x,y,w,h), where (x,y)
    /// represents the lower left corner of the viewport rectangle, and (w,h)
    /// is the width and height of the viewport in pixels.
    
    void SetRenderViewport(pxr::GfVec4d const &viewport);

    /// Set the window policy to use.
    /// XXX: This is currently used for scene cameras set via SetCameraPath.
    /// See comment in SetCameraState for the free cam.
    
    void SetWindowPolicy(pxr::CameraUtilConformWindowPolicy policy);

    /// Scene camera API
    /// Set the scene camera path to use for rendering.
    
    void SetCameraPath(pxr::SdfPath const &id);

    /// Free camera API
    /// Set camera framing state directly (without pointing to a camera on the
    /// USD stage). The projection matrix is expected to be pre-adjusted for the
    /// window policy.
    
    void SetCameraState(const pxr::GfMatrix4d &viewMatrix,
                        const pxr::GfMatrix4d &projectionMatrix);

    /// Helper function to extract camera and viewport state from opengl and
    /// then call SetCameraState and SetRenderViewport
    
    void SetCameraStateFromOpenGL();

    /// @}

    // ---------------------------------------------------------------------
    /// \name Light State
    /// @{
    // ---------------------------------------------------------------------

    /// Helper function to extract lighting state from opengl and then
    /// call SetLights.
    
    void SetLightingStateFromOpenGL();

    /// Copy lighting state from another lighting context.
    
    void SetLightingState(pxr::GlfSimpleLightingContextPtr const &src);

    /// Set lighting state
    /// Derived classes should ensure that passing an empty lights
    /// vector disables lighting.
    /// \param lights is the set of lights to use, or empty to disable lighting.
    
    void SetLightingState(pxr::GlfSimpleLightVector const &lights,
                          pxr::GlfSimpleMaterial const &material,
                          pxr::GfVec4f const &sceneAmbient);

    /// @}

    // ---------------------------------------------------------------------
    /// \name Selection Highlighting
    /// @{
    // ---------------------------------------------------------------------

    /// Sets (replaces) the list of prim paths that should be included in
    /// selection highlighting. These paths may include root paths which will
    /// be expanded internally.
    
    void SetSelected(pxr::SdfPathVector const &paths);

    /// Clear the list of prim paths that should be included in selection
    /// highlighting.
    
    void ClearSelected();

    /// Add a path with instanceIndex to the list of prim paths that should be
    /// included in selection highlighting. UsdImagingDelegate::ALL_INSTANCES
    /// can be used for highlighting all instances if path is an instancer.
    
    void AddSelected(pxr::SdfPath const &path, int instanceIndex);

    /// Sets the selection highlighting color.
    
    void SetSelectionColor(pxr::GfVec4f const &color);

    /// @}

    // ---------------------------------------------------------------------
    /// \name Picking
    /// @{
    // ---------------------------------------------------------------------

    /// Finds closest point of intersection with a frustum by rendering.
    ///
    /// This method uses a PickRender and a customized depth buffer to find an
    /// approximate point of intersection by rendering. This is less accurate
    /// than implicit methods or rendering with GL_SELECT, but leverages any
    /// data already cached in the renderer.
    ///
    /// Returns whether a hit occurred and if so, \p outHitPoint will contain
    /// the intersection point in world space (i.e. \p projectionMatrix and
    /// \p viewMatrix factored back out of the result).
    ///
    /// \p outHitPrimPath will point to the gprim selected by the pick.
    /// \p outHitInstancerPath will point to the point instancer (if applicable)
    /// of that gprim. For nested instancing, outHitInstancerPath points to
    /// the closest instancer.
    ///
    
    bool TestIntersection(
        const pxr::GfMatrix4d &viewMatrix,
        const pxr::GfMatrix4d &projectionMatrix,
        const pxr::UsdPrim &root,
        const pxr::UsdImagingGLRenderParams &params,
        pxr::GfVec3d *outHitPoint,
        pxr::SdfPath *outHitPrimPath = NULL,
        pxr::SdfPath *outHitInstancerPath = NULL,
        int *outHitInstanceIndex = NULL,
        pxr::HdInstancerContext *outInstancerContext = NULL);

    /// Decodes a pick result given hydra prim ID/instance ID (like you'd get
    /// from an ID render).
    
    bool DecodeIntersection(
        unsigned char const primIdColor[4],
        unsigned char const instanceIdColor[4],
        pxr::SdfPath *outHitPrimPath = NULL,
        pxr::SdfPath *outHitInstancerPath = NULL,
        int *outHitInstanceIndex = NULL,
        pxr::HdInstancerContext *outInstancerContext = NULL);

    /// @}

    // ---------------------------------------------------------------------
    /// \name Renderer Plugin Management
    /// @{
    // ---------------------------------------------------------------------

    /// Return the vector of available render-graph delegate plugins.
    
    static pxr::TfTokenVector GetRendererPlugins();

    /// Return the user-friendly description of a renderer plugin.
    
    static std::string GetRendererDisplayName(pxr::TfToken const &id);

    /// Return the id of the currently used renderer plugin.
    
    pxr::TfToken GetCurrentRendererId() const;

    /// Set the current render-graph delegate to \p id.
    /// the plugin will be loaded if it's not yet.
    
    bool SetRendererPlugin(pxr::TfToken const &id);

    /// @}

    // ---------------------------------------------------------------------
    /// \name AOVs and Renderer Settings
    /// @{
    // ---------------------------------------------------------------------

    /// Return the vector of available renderer AOV settings.
    
    pxr::TfTokenVector GetRendererAovs() const;

    /// Set the current renderer AOV to \p id.
    
    bool SetRendererAov(pxr::TfToken const &id);

    /// Returns an AOV texture handle for the given token.
    
    pxr::HgiTextureHandle GetAovTexture(pxr::TfToken const &name) const;

    /// Returns the list of renderer settings.
    
    pxr::UsdImagingGLRendererSettingsList GetRendererSettingsList() const;

    /// Gets a renderer setting's current value.
    
    pxr::VtValue GetRendererSetting(pxr::TfToken const &id) const;

    /// Sets a renderer setting's value.
    
    void SetRendererSetting(pxr::TfToken const &id,
                            pxr::VtValue const &value);

    /// @}

    // ---------------------------------------------------------------------
    /// \name Control of background rendering threads.
    /// @{
    // ---------------------------------------------------------------------

    /// Query the renderer as to whether it supports pausing and resuming.
    
    bool IsPauseRendererSupported() const;

    /// Pause the renderer.
    ///
    /// Returns \c true if successful.
    
    bool PauseRenderer();

    /// Resume the renderer.
    ///
    /// Returns \c true if successful.
    
    bool ResumeRenderer();

    /// Query the renderer as to whether it supports stopping and restarting.
    
    bool IsStopRendererSupported() const;

    /// Stop the renderer.
    ///
    /// Returns \c true if successful.
    
    bool StopRenderer();

    /// Restart the renderer.
    ///
    /// Returns \c true if successful.
    
    bool RestartRenderer();

    /// @}

    // ---------------------------------------------------------------------
    /// \name Color Correction
    /// @{
    // ---------------------------------------------------------------------

    /// Set \p id to one of the HdxColorCorrectionTokens.
    
    void SetColorCorrectionSettings(
        pxr::TfToken const &id);

    /// @}

    /// Returns true if the platform is color correction capable.
    
    static bool IsColorCorrectionCapable();

    // ---------------------------------------------------------------------
    /// \name Render Statistics
    /// @{
    // ---------------------------------------------------------------------

    /// Returns render statistics.
    ///
    /// The contents of the dictionary will depend on the current render
    /// delegate.
    ///
    
    pxr::VtDictionary GetRenderStats() const;

    /// @}

    // ---------------------------------------------------------------------
    /// \name HGI
    /// @{
    // ---------------------------------------------------------------------

    /// Returns the HGI interface.
    ///
    
    pxr::Hgi *GetHgi();

    /// @}

protected:
    /// Open some protected methods for whitebox testing.
    friend class UsdImagingGL_UnitTestGLDrawing;

    /// Returns the render index of the engine, if any.  This is only used for
    /// whitebox testing.
    
    pxr::HdRenderIndex *_GetRenderIndex() const;

    
    void _Execute(const pxr::UsdImagingGLRenderParams &params,
                  pxr::HdTaskSharedPtrVector tasks);

    // These functions factor batch preparation into separate steps so they
    // can be reused by both the vectorized and non-vectorized API.
    
    bool _CanPrepareBatch(const pxr::UsdPrim &root,
                          const pxr::UsdImagingGLRenderParams &params);
    
    void _PreSetTime(const pxr::UsdPrim &root,
                     const pxr::UsdImagingGLRenderParams &params);
    
    void _PostSetTime(const pxr::UsdPrim &root,
                      const pxr::UsdImagingGLRenderParams &params);

    // Create a hydra collection given root paths and render params.
    // Returns true if the collection was updated.
    
    static bool _UpdateHydraCollection(pxr::HdRprimCollection *collection,
                                       pxr::SdfPathVector const &roots,
                                       pxr::UsdImagingGLRenderParams const &params);
    
    static pxr::HdxRenderTaskParams _MakeHydraUsdImagingGLRenderParams(
        pxr::UsdImagingGLRenderParams const &params);
    
    static void _ComputeRenderTags(pxr::UsdImagingGLRenderParams const &params,
                                   pxr::TfTokenVector *renderTags);

    
    void _InitializeHgiIfNecessary();

    
    void _SetRenderDelegateAndRestoreState(
        pxr::HdPluginRenderDelegateUniqueHandle &&);

    
    void _SetRenderDelegate(pxr::HdPluginRenderDelegateUniqueHandle &&);

    
    pxr::SdfPath _ComputeControllerPath(const pxr::HdPluginRenderDelegateUniqueHandle &);

    
    static pxr::TfToken _GetDefaultRendererPluginId();

    
    pxr::UsdImagingDelegate *_GetSceneDelegate() const;

    
    pxr::HdEngine *_GetHdEngine();

    
    pxr::HdxTaskController *_GetTaskController() const;

       
    pxr::HdSelectionSharedPtr _GetSelection() const;

protected:
    // private:
    // Note that any of the fields below might become private
    // in the future and subclasses should use the above getters
    // to access them instead.

    pxr::HgiUniquePtr _hgi;
    // Similar for HdDriver.
    pxr::HdDriver _hgiDriver;

protected:
    pxr::HdPluginRenderDelegateUniqueHandle _renderDelegate;
    std::unique_ptr<pxr::HdRenderIndex> _renderIndex;

    pxr::SdfPath const _sceneDelegateId;

    std::unique_ptr<pxr::HdxTaskController> _taskController;

    pxr::HdxSelectionTrackerSharedPtr _selTracker;
    pxr::HdRprimCollection _renderCollection;
    pxr::HdRprimCollection _intersectCollection;

    pxr::GlfSimpleLightingContextRefPtr _lightingContextForOpenGLState;

    // Data we want to live across render plugin switches:
    pxr::GfVec4f _selectionColor;

    pxr::SdfPath _rootPath;
    pxr::SdfPathVector _excludedPrimPaths;
    pxr::SdfPathVector _invisedPrimPaths;
    bool _isPopulated;

private:
    void _DestroyHydraObjects();

    std::unique_ptr<pxr::UsdImagingDelegate> _sceneDelegate;
    std::unique_ptr<pxr::HdEngine> _engine;
};
