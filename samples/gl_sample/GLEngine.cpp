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
#include "pxr/imaging/glf/glew.h"
#include "GLEngine.h"
#include <assert.h>

#include "pxr/usdImaging/usdImaging/delegate.h"

#include "pxr/usd/usdGeom/tokens.h"
#include "pxr/usd/usdGeom/camera.h"

#include "pxr/imaging/glf/diagnostic.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/glContext.h"
#include "pxr/imaging/glf/info.h"

#include "pxr/imaging/hd/rendererPlugin.h"
#include "pxr/imaging/hd/rendererPluginRegistry.h"
#include "pxr/imaging/hdx/taskController.h"
#include "pxr/imaging/hdx/tokens.h"

#include "pxr/imaging/hgi/hgi.h"
#include "pxr/imaging/hgi/tokens.h"

#include "pxr/base/tf/envSetting.h"
#include "pxr/base/tf/getenv.h"
#include "pxr/base/tf/stl.h"

#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3d.h"

#include <string>

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_ENV_SETTING(USDIMAGINGGL_ENGINE_DEBUG_SCENE_DELEGATE_ID, "/",
                      "Default usdImaging scene delegate id");

PXR_NAMESPACE_CLOSE_SCOPE

namespace
{

    bool
    _GetHydraEnabledEnvVar()
    {
        // XXX: Note that we don't cache the result here.  This is primarily because
        // of the way usdview currently interacts with this setting.  This should
        // be cleaned up, and the new class hierarchy around GLEngine
        // makes it much easier to do so.
        return pxr::TfGetenv("HD_ENABLED", "1") == "1";
    }

    pxr::SdfPath const &
    _GetUsdImagingDelegateId()
    {
        static pxr::SdfPath const delegateId =
            pxr::SdfPath(pxr::TfGetEnvSetting(pxr::USDIMAGINGGL_ENGINE_DEBUG_SCENE_DELEGATE_ID));

        return delegateId;
    }

    void _InitGL()
    {
        static std::once_flag initFlag;

        std::call_once(initFlag, [] {
            // Initialize Glew library for GL Extensions if needed
            pxr::GlfGlewInit();

            // Initialize if needed and switch to shared GL context.
            pxr::GlfSharedGLContextScopeHolder sharedContext;

            // Initialize GL context caps based on shared context
            pxr::GlfContextCaps::InitInstance();
        });
    }

    bool
    _IsHydraEnabled()
    {
        // Make sure there is an OpenGL context when
        // trying to initialize Hydra/Reference
        pxr::GlfGLContextSharedPtr context = pxr::GlfGLContext::GetCurrentGLContext();
        if (!context || !context->IsValid())
        {
            using namespace pxr;
            TF_CODING_ERROR("OpenGL context required, using reference renderer");
            return false;
        }

        if (!_GetHydraEnabledEnvVar())
        {
            return false;
        }

        // Check to see if we have a default plugin for the renderer
        pxr::TfToken defaultPlugin =
            pxr::HdRendererPluginRegistry::GetInstance().GetDefaultPluginId();

        return !defaultPlugin.IsEmpty();
    }

} // anonymous namespace

//----------------------------------------------------------------------------
// Global State
//----------------------------------------------------------------------------

/*static*/
bool GLEngine::IsHydraEnabled()
{
    static bool isHydraEnabled = _IsHydraEnabled();
    return isHydraEnabled;
}

//----------------------------------------------------------------------------
// Construction
//----------------------------------------------------------------------------

GLEngine::GLEngine(const pxr::HdDriver &driver)
    : GLEngine(pxr::SdfPath::AbsoluteRootPath(),
               {},
               {},
               _GetUsdImagingDelegateId(),
               driver)
{
}

GLEngine::GLEngine(
    const pxr::SdfPath &rootPath,
    const pxr::SdfPathVector &excludedPaths,
    const pxr::SdfPathVector &invisedPaths,
    const pxr::SdfPath &sceneDelegateID,
    const pxr::HdDriver &driver)
    : _hgi(), _hgiDriver(driver), _sceneDelegateId(sceneDelegateID), _selTracker(std::make_shared<pxr::HdxSelectionTracker>()), _selectionColor(1.0f, 1.0f, 0.0f, 1.0f), _rootPath(rootPath), _excludedPrimPaths(excludedPaths), _invisedPrimPaths(invisedPaths), _isPopulated(false)
{
    _InitGL();

    if (IsHydraEnabled())
    {

        // _renderIndex, _taskController, and _sceneDelegate are initialized
        // by the plugin system.
        if (!SetRendererPlugin(_GetDefaultRendererPluginId()))
        {
            using namespace pxr;
            TF_CODING_ERROR("No renderer plugins found! "
                            "Check before creation.");
        }
    }
    else
    {

        assert(false);

        // // In the legacy implementation, both excluded paths and invised paths
        // // are treated the same way.
        // pxr::SdfPathVector pathsToExclude = excludedPaths;
        // pathsToExclude.insert(pathsToExclude.end(),
        //     invisedPaths.begin(), invisedPaths.end());
        // _legacyImpl =std::make_unique<UsdImagingGLLegacyEngine>(pathsToExclude);
    }
}

void GLEngine::_DestroyHydraObjects()
{
    // Destroy objects in opposite order of construction.
    _engine = nullptr;
    _taskController = nullptr;
    _sceneDelegate = nullptr;
    _renderIndex = nullptr;
    _renderDelegate = nullptr;
}

GLEngine::~GLEngine()
{
    TF_PY_ALLOW_THREADS_IN_SCOPE();

    _DestroyHydraObjects();
}

//----------------------------------------------------------------------------
// Rendering
//----------------------------------------------------------------------------

void GLEngine::PrepareBatch(
    const pxr::UsdPrim &root,
    const pxr::UsdImagingGLRenderParams &params)
{
    using namespace pxr;

    HD_TRACE_FUNCTION();

    TF_VERIFY(_sceneDelegate);

    if (_CanPrepareBatch(root, params))
    {
        if (!_isPopulated)
        {
            _sceneDelegate->SetUsdDrawModesEnabled(params.enableUsdDrawModes);
            _sceneDelegate->Populate(
                root.GetStage()->GetPrimAtPath(_rootPath),
                _excludedPrimPaths);
            _sceneDelegate->SetInvisedPrimPaths(_invisedPrimPaths);
            _isPopulated = true;
        }

        _PreSetTime(root, params);
        // SetTime will only react if time actually changes.
        _sceneDelegate->SetTime(params.frame);
        _PostSetTime(root, params);
    }
}

void GLEngine::RenderBatch(
    const pxr::SdfPathVector &paths,
    const pxr::UsdImagingGLRenderParams &params)
{
    using namespace pxr;

    TF_VERIFY(_taskController);

    _taskController->SetFreeCameraClipPlanes(params.clipPlanes);
    _UpdateHydraCollection(&_renderCollection, paths, params);
    _taskController->SetCollection(_renderCollection);

    TfTokenVector renderTags;
    _ComputeRenderTags(params, &renderTags);
    _taskController->SetRenderTags(renderTags);

    HdxRenderTaskParams hdParams = _MakeHydraUsdImagingGLRenderParams(params);

    _taskController->SetRenderParams(hdParams);
    _taskController->SetEnableSelection(params.highlight);

    SetColorCorrectionSettings(params.colorCorrectionMode);

    // XXX App sets the clear color via 'params' instead of setting up Aovs
    // that has clearColor in their descriptor. So for now we must pass this
    // clear color to the color AOV.
    HdAovDescriptor colorAovDesc =
        _taskController->GetRenderOutputSettings(HdAovTokens->color);
    if (colorAovDesc.format != HdFormatInvalid)
    {
        colorAovDesc.clearValue = VtValue(params.clearColor);
        _taskController->SetRenderOutputSettings(
            HdAovTokens->color, colorAovDesc);
    }

    // Forward scene materials enable option to delegate
    _sceneDelegate->SetSceneMaterialsEnabled(params.enableSceneMaterials);

    VtValue selectionValue(_selTracker);
    _engine->SetTaskContextData(HdxTokens->selectionState, selectionValue);
    _Execute(params, _taskController->GetRenderingTasks());
}

void GLEngine::Render(
    const pxr::UsdPrim &root,
    const pxr::UsdImagingGLRenderParams &params)
{
    using namespace pxr;

    TF_VERIFY(_taskController);

    PrepareBatch(root, params);

    // XXX(UsdImagingPaths): Is it correct to map USD root path directly
    // to the cachePath here?
    const SdfPath cachePath = root.GetPath();
    const SdfPathVector paths = {
        _sceneDelegate->ConvertCachePathToIndexPath(cachePath)};

    RenderBatch(paths, params);
}

void GLEngine::InvalidateBuffers()
{
    using namespace pxr;
}

bool GLEngine::IsConverged() const
{
    using namespace pxr;

    TF_VERIFY(_taskController);
    return _taskController->IsConverged();
}

//----------------------------------------------------------------------------
// Root and Transform Visibility
//----------------------------------------------------------------------------

void GLEngine::SetRootTransform(pxr::GfMatrix4d const &xf)
{
    using namespace pxr;
    TF_VERIFY(_sceneDelegate);
    _sceneDelegate->SetRootTransform(xf);
}

void GLEngine::SetRootVisibility(bool isVisible)
{
    using namespace pxr;
    TF_VERIFY(_sceneDelegate);
    _sceneDelegate->SetRootVisibility(isVisible);
}

//----------------------------------------------------------------------------
// Camera and Light State
//----------------------------------------------------------------------------

void GLEngine::SetRenderViewport(pxr::GfVec4d const &viewport)
{
    using namespace pxr;
    TF_VERIFY(_taskController);
    _taskController->SetRenderViewport(viewport);
}

void GLEngine::SetWindowPolicy(pxr::CameraUtilConformWindowPolicy policy)
{
    using namespace pxr;
    TF_VERIFY(_taskController);
    // Note: Free cam uses SetCameraState, which expects the frustum to be
    // pre-adjusted for the viewport size.

    // The usdImagingDelegate manages the window policy for scene cameras.
    _sceneDelegate->SetWindowPolicy(policy);
}

void GLEngine::SetCameraPath(pxr::SdfPath const &id)
{
    using namespace pxr;
    TF_VERIFY(_taskController);
    _taskController->SetCameraPath(id);

    // The camera that is set for viewing will also be used for
    // time sampling.
    _sceneDelegate->SetCameraForSampling(id);
}

void GLEngine::SetCameraState(const pxr::GfMatrix4d &viewMatrix,
                              const pxr::GfMatrix4d &projectionMatrix)
{
    using namespace pxr;
    TF_VERIFY(_taskController);
    _taskController->SetFreeCameraMatrices(viewMatrix, projectionMatrix);
}

void GLEngine::SetCameraStateFromOpenGL()
{
    pxr::GfMatrix4d viewMatrix, projectionMatrix;
    pxr::GfVec4d viewport;
    glGetDoublev(GL_MODELVIEW_MATRIX, viewMatrix.GetArray());
    glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix.GetArray());
    glGetDoublev(GL_VIEWPORT, &viewport[0]);

    SetCameraState(viewMatrix, projectionMatrix);
    SetRenderViewport(viewport);
}

void GLEngine::SetLightingStateFromOpenGL()
{
    using namespace pxr;
    TF_VERIFY(_taskController);

    if (!_lightingContextForOpenGLState)
    {
        _lightingContextForOpenGLState = GlfSimpleLightingContext::New();
    }
    _lightingContextForOpenGLState->SetStateFromOpenGL();

    _taskController->SetLightingState(_lightingContextForOpenGLState);
}

void GLEngine::SetLightingState(pxr::GlfSimpleLightingContextPtr const &src)
{
    using namespace pxr;
    TF_VERIFY(_taskController);
    _taskController->SetLightingState(src);
}

void GLEngine::SetLightingState(
    pxr::GlfSimpleLightVector const &lights,
    pxr::GlfSimpleMaterial const &material,
    pxr::GfVec4f const &sceneAmbient)
{
    using namespace pxr;
    TF_VERIFY(_taskController);

    // we still use _lightingContextForOpenGLState for convenience, but
    // set the values directly.
    if (!_lightingContextForOpenGLState)
    {
        _lightingContextForOpenGLState = GlfSimpleLightingContext::New();
    }
    _lightingContextForOpenGLState->SetLights(lights);
    _lightingContextForOpenGLState->SetMaterial(material);
    _lightingContextForOpenGLState->SetSceneAmbient(sceneAmbient);
    _lightingContextForOpenGLState->SetUseLighting(lights.size() > 0);

    _taskController->SetLightingState(_lightingContextForOpenGLState);
}

//----------------------------------------------------------------------------
// Selection Highlighting
//----------------------------------------------------------------------------

void GLEngine::SetSelected(pxr::SdfPathVector const &paths)
{
    using namespace pxr;
    TF_VERIFY(_sceneDelegate);

    // populate new selection
    HdSelectionSharedPtr const selection = std::make_shared<HdSelection>();
    // XXX: Usdview currently supports selection on click. If we extend to
    // rollover (locate) selection, we need to pass that mode here.
    static const HdSelection::HighlightMode mode =
        HdSelection::HighlightModeSelect;
    for (SdfPath const &path : paths)
    {
        _sceneDelegate->PopulateSelection(mode,
                                          path,
                                          UsdImagingDelegate::ALL_INSTANCES,
                                          selection);
    }

    // set the result back to selection tracker
    _selTracker->SetSelection(selection);
}

void GLEngine::ClearSelected()
{
    using namespace pxr;
    TF_VERIFY(_selTracker);

    _selTracker->SetSelection(std::make_shared<HdSelection>());
}

pxr::HdSelectionSharedPtr
GLEngine::_GetSelection() const
{
    if (pxr::HdSelectionSharedPtr const selection = _selTracker->GetSelectionMap())
    {
        return selection;
    }

    return std::make_shared<pxr::HdSelection>();
}

void GLEngine::AddSelected(pxr::SdfPath const &path, int instanceIndex)
{
    using namespace pxr;
    TF_VERIFY(_sceneDelegate);

    HdSelectionSharedPtr const selection = _GetSelection();

    // XXX: Usdview currently supports selection on click. If we extend to
    // rollover (locate) selection, we need to pass that mode here.
    static const HdSelection::HighlightMode mode =
        HdSelection::HighlightModeSelect;
    _sceneDelegate->PopulateSelection(mode, path, instanceIndex, selection);

    // set the result back to selection tracker
    _selTracker->SetSelection(selection);
}

void GLEngine::SetSelectionColor(pxr::GfVec4f const &color)
{
    using namespace pxr;
    TF_VERIFY(_taskController);

    _selectionColor = color;
    _taskController->SetSelectionColor(_selectionColor);
}

//----------------------------------------------------------------------------
// Picking
//----------------------------------------------------------------------------

bool GLEngine::TestIntersection(
    const pxr::GfMatrix4d &viewMatrix,
    const pxr::GfMatrix4d &projectionMatrix,
    const pxr::UsdPrim &root,
    const pxr::UsdImagingGLRenderParams &params,
    pxr::GfVec3d *outHitPoint,
    pxr::SdfPath *outHitPrimPath,
    pxr::SdfPath *outHitInstancerPath,
    int *outHitInstanceIndex,
    pxr::HdInstancerContext *outInstancerContext)
{
    using namespace pxr;
    TF_VERIFY(_sceneDelegate);
    TF_VERIFY(_taskController);

    // XXX(UsdImagingPaths): This is incorrect...  "Root" points to a USD
    // subtree, but the subtree in the hydra namespace might be very different
    // (e.g. for native instancing).  We need a translation step.
    const SdfPath cachePath = root.GetPath();
    const SdfPathVector roots = {
        _sceneDelegate->ConvertCachePathToIndexPath(cachePath)};
    _UpdateHydraCollection(&_intersectCollection, roots, params);

    TfTokenVector renderTags;
    _ComputeRenderTags(params, &renderTags);
    _taskController->SetRenderTags(renderTags);

    _taskController->SetRenderParams(
        _MakeHydraUsdImagingGLRenderParams(params));

    // Forward scene materials enable option to delegate
    _sceneDelegate->SetSceneMaterialsEnabled(params.enableSceneMaterials);

    HdxPickHitVector allHits;
    HdxPickTaskContextParams pickParams;
    pickParams.resolveMode = HdxPickTokens->resolveNearestToCenter;
    pickParams.viewMatrix = viewMatrix;
    pickParams.projectionMatrix = projectionMatrix;
    pickParams.clipPlanes = params.clipPlanes;
    pickParams.collection = _intersectCollection;
    pickParams.outHits = &allHits;
    const VtValue vtPickParams(pickParams);

    _engine->SetTaskContextData(HdxPickTokens->pickParams, vtPickParams);
    _Execute(params, _taskController->GetPickingTasks());

    // Since we are in nearest-hit mode, we expect allHits to have
    // a single point in it.
    if (allHits.size() != 1)
    {
        return false;
    }

    HdxPickHit &hit = allHits[0];

    if (outHitPoint)
    {
        *outHitPoint = GfVec3d(hit.worldSpaceHitPoint[0],
                               hit.worldSpaceHitPoint[1],
                               hit.worldSpaceHitPoint[2]);
    }

    hit.objectId = _sceneDelegate->GetScenePrimPath(
        hit.objectId, hit.instanceIndex, outInstancerContext);
    hit.instancerId = _sceneDelegate->ConvertIndexPathToCachePath(
                                        hit.instancerId)
                          .GetAbsoluteRootOrPrimPath();

    if (outHitPrimPath)
    {
        *outHitPrimPath = hit.objectId;
    }
    if (outHitInstancerPath)
    {
        *outHitInstancerPath = hit.instancerId;
    }
    if (outHitInstanceIndex)
    {
        *outHitInstanceIndex = hit.instanceIndex;
    }

    return true;
}

bool GLEngine::DecodeIntersection(
    unsigned char const primIdColor[4],
    unsigned char const instanceIdColor[4],
    pxr::SdfPath *outHitPrimPath,
    pxr::SdfPath *outHitInstancerPath,
    int *outHitInstanceIndex,
    pxr::HdInstancerContext *outInstancerContext)
{
    using namespace pxr;
    TF_VERIFY(_sceneDelegate);

    const int primId = HdxPickTask::DecodeIDRenderColor(primIdColor);
    const int instanceIdx = HdxPickTask::DecodeIDRenderColor(instanceIdColor);
    SdfPath primPath =
        _sceneDelegate->GetRenderIndex().GetRprimPathFromPrimId(primId);

    if (primPath.IsEmpty())
    {
        return false;
    }

    SdfPath delegateId, instancerId;
    _sceneDelegate->GetRenderIndex().GetSceneDelegateAndInstancerIds(
        primPath, &delegateId, &instancerId);

    primPath = _sceneDelegate->GetScenePrimPath(
        primPath, instanceIdx, outInstancerContext);
    instancerId = _sceneDelegate->ConvertIndexPathToCachePath(instancerId)
                      .GetAbsoluteRootOrPrimPath();

    if (outHitPrimPath)
    {
        *outHitPrimPath = primPath;
    }
    if (outHitInstancerPath)
    {
        *outHitInstancerPath = instancerId;
    }
    if (outHitInstanceIndex)
    {
        *outHitInstanceIndex = instanceIdx;
    }

    return true;
}

//----------------------------------------------------------------------------
// Renderer Plugin Management
//----------------------------------------------------------------------------

/* static */
pxr::TfTokenVector
GLEngine::GetRendererPlugins()
{
    pxr::HfPluginDescVector pluginDescriptors;
    pxr::HdRendererPluginRegistry::GetInstance().GetPluginDescs(&pluginDescriptors);

    pxr::TfTokenVector plugins;
    for (size_t i = 0; i < pluginDescriptors.size(); ++i)
    {
        plugins.push_back(pluginDescriptors[i].id);
    }
    return plugins;
}

/* static */
std::string
GLEngine::GetRendererDisplayName(pxr::TfToken const &id)
{
    if (id.IsEmpty())
    {
        // No renderer name is returned if the user requested to disable Hydra,
        // or if the machine does not support any of the available renderers
        // and it automatically switches to our legacy engine.
        return std::string();
    }

    using namespace pxr;
    HfPluginDesc pluginDescriptor;
    if (!TF_VERIFY(HdRendererPluginRegistry::GetInstance().GetPluginDesc(id, &pluginDescriptor)))
    {
        return std::string();
    }

    return pluginDescriptor.displayName;
}

pxr::TfToken
GLEngine::GetCurrentRendererId() const
{
    return _renderDelegate.GetPluginId();
}

void GLEngine::_InitializeHgiIfNecessary()
{
    // If the client of GLEngine does not provide a HdDriver, we
    // construct a default one that is owned by GLEngine.
    // The cleanest pattern is for the client app to provide this since you
    // may have multiple UsdImagingGLEngines in one app that ideally all use
    // the same HdDriver and Hgi to share GPU resources.
    if (_hgiDriver.driver.IsEmpty())
    {
        _hgi = pxr::Hgi::CreatePlatformDefaultHgi();
        _hgiDriver.name = pxr::HgiTokens->renderDriver;
        _hgiDriver.driver = pxr::VtValue(_hgi.get());
    }
}

bool GLEngine::SetRendererPlugin(pxr::TfToken const &id)
{
    using namespace pxr;
    _InitializeHgiIfNecessary();

    HdRendererPluginRegistry &registry =
        HdRendererPluginRegistry::GetInstance();

    // Special case: id = TfToken() selects the first plugin in the list.
    const TfToken resolvedId =
        id.IsEmpty() ? registry.GetDefaultPluginId() : id;

    if (_renderDelegate && _renderDelegate.GetPluginId() == resolvedId)
    {
        return true;
    }

    TF_PY_ALLOW_THREADS_IN_SCOPE();

    HdPluginRenderDelegateUniqueHandle renderDelegate =
        registry.CreateRenderDelegate(resolvedId);
    if (!renderDelegate)
    {
        return false;
    }

    _SetRenderDelegateAndRestoreState(std::move(renderDelegate));

    return true;
}

void GLEngine::_SetRenderDelegateAndRestoreState(
    pxr::HdPluginRenderDelegateUniqueHandle &&renderDelegate)
{
    // Pull old delegate/task controller state.

    const pxr::GfMatrix4d rootTransform =
        _sceneDelegate ? _sceneDelegate->GetRootTransform() : pxr::GfMatrix4d(1.0);
    const bool isVisible =
        _sceneDelegate ? _sceneDelegate->GetRootVisibility() : true;
    pxr::HdSelectionSharedPtr const selection = _GetSelection();

    _SetRenderDelegate(std::move(renderDelegate));

    // Rebuild state in the new delegate/task controller.
    _sceneDelegate->SetRootVisibility(isVisible);
    _sceneDelegate->SetRootTransform(rootTransform);
    _selTracker->SetSelection(selection);
    _taskController->SetSelectionColor(_selectionColor);
}

pxr::SdfPath
GLEngine::_ComputeControllerPath(
    const pxr::HdPluginRenderDelegateUniqueHandle &renderDelegate)
{
    const std::string pluginId =
        pxr::TfMakeValidIdentifier(renderDelegate.GetPluginId().GetText());
    const pxr::TfToken rendererName(
        pxr::TfStringPrintf("_UsdImaging_%s_%p", pluginId.c_str(), this));

    return _sceneDelegateId.AppendChild(rendererName);
}

void GLEngine::_SetRenderDelegate(
    pxr::HdPluginRenderDelegateUniqueHandle &&renderDelegate)
{
    // This relies on SetRendererPlugin to release the GIL...

    // Destruction
    _DestroyHydraObjects();

    _isPopulated = false;

    // Creation

    // Use the new render delegate.
    _renderDelegate = std::move(renderDelegate);

    // Recreate the render index
    _renderIndex.reset(
        pxr::HdRenderIndex::New(
            _renderDelegate.Get(), {&_hgiDriver}));

    // Create the new delegate
    _sceneDelegate = std::make_unique<pxr::UsdImagingDelegate>(
        _renderIndex.get(), _sceneDelegateId);

    // Create the new task controller
    _taskController = std::make_unique<pxr::HdxTaskController>(
        _renderIndex.get(),
        _ComputeControllerPath(_renderDelegate));

    // The task context holds on to resources in the render
    // deletegate, so we want to destroy it first and thus
    // create it last.
    _engine = std::make_unique<pxr::HdEngine>();
}

//----------------------------------------------------------------------------
// AOVs and Renderer Settings
//----------------------------------------------------------------------------

pxr::TfTokenVector
GLEngine::GetRendererAovs() const
{
    using namespace pxr;
    TF_VERIFY(_renderIndex);

    if (_renderIndex->IsBprimTypeSupported(HdPrimTypeTokens->renderBuffer))
    {

        static const TfToken candidates[] =
            {HdAovTokens->primId,
             HdAovTokens->depth,
             HdAovTokens->normal,
             HdAovTokensMakePrimvar(TfToken("st"))};

        TfTokenVector aovs = {HdAovTokens->color};
        for (auto const &aov : candidates)
        {
            if (_renderDelegate->GetDefaultAovDescriptor(aov).format != HdFormatInvalid)
            {
                aovs.push_back(aov);
            }
        }
        return aovs;
    }
    return TfTokenVector();
}

bool GLEngine::SetRendererAov(pxr::TfToken const &id)
{
    using namespace pxr;
    TF_VERIFY(_renderIndex);
    if (_renderIndex->IsBprimTypeSupported(HdPrimTypeTokens->renderBuffer))
    {
        _taskController->SetRenderOutputs({id});
        return true;
    }
    return false;
}

pxr::HgiTextureHandle
GLEngine::GetAovTexture(
    pxr::TfToken const &name) const
{
    pxr::VtValue aov;
    pxr::HgiTextureHandle aovTexture;

    if (_engine->GetTaskContextData(name, &aov))
    {
        if (aov.IsHolding<pxr::HgiTextureHandle>())
        {
            aovTexture = aov.Get<pxr::HgiTextureHandle>();
        }
    }

    return aovTexture;
}

pxr::UsdImagingGLRendererSettingsList
GLEngine::GetRendererSettingsList() const
{
    using namespace pxr;
    TF_VERIFY(_renderDelegate);

    const HdRenderSettingDescriptorList descriptors =
        _renderDelegate->GetRenderSettingDescriptors();
    UsdImagingGLRendererSettingsList ret;

    for (auto const &desc : descriptors)
    {
        UsdImagingGLRendererSetting r;
        r.key = desc.key;
        r.name = desc.name;
        r.defValue = desc.defaultValue;

        // Use the type of the default value to tell us what kind of
        // widget to create...
        if (r.defValue.IsHolding<bool>())
        {
            r.type = UsdImagingGLRendererSetting::TYPE_FLAG;
        }
        else if (r.defValue.IsHolding<int>() ||
                 r.defValue.IsHolding<unsigned int>())
        {
            r.type = UsdImagingGLRendererSetting::TYPE_INT;
        }
        else if (r.defValue.IsHolding<float>())
        {
            r.type = UsdImagingGLRendererSetting::TYPE_FLOAT;
        }
        else if (r.defValue.IsHolding<std::string>())
        {
            r.type = UsdImagingGLRendererSetting::TYPE_STRING;
        }
        else
        {
            TF_WARN("Setting '%s' with type '%s' doesn't have a UI"
                    " implementation...",
                    r.name.c_str(),
                    r.defValue.GetTypeName().c_str());
            continue;
        }
        ret.push_back(r);
    }

    return ret;
}

pxr::VtValue
GLEngine::GetRendererSetting(pxr::TfToken const &id) const
{
    using namespace pxr;
    TF_VERIFY(_renderDelegate);
    return _renderDelegate->GetRenderSetting(id);
}

void GLEngine::SetRendererSetting(pxr::TfToken const &id, pxr::VtValue const &value)
{
    using namespace pxr;
    TF_VERIFY(_renderDelegate);
    _renderDelegate->SetRenderSetting(id, value);
}

// ---------------------------------------------------------------------
// Control of background rendering threads.
// ---------------------------------------------------------------------
bool GLEngine::IsPauseRendererSupported() const
{
    using namespace pxr;
    TF_VERIFY(_renderDelegate);
    return _renderDelegate->IsPauseSupported();
}

bool GLEngine::PauseRenderer()
{
    using namespace pxr;
    TF_PY_ALLOW_THREADS_IN_SCOPE();

    TF_VERIFY(_renderDelegate);
    return _renderDelegate->Pause();
}

bool GLEngine::ResumeRenderer()
{
    using namespace pxr;
    TF_PY_ALLOW_THREADS_IN_SCOPE();

    TF_VERIFY(_renderDelegate);
    return _renderDelegate->Resume();
}

bool GLEngine::IsStopRendererSupported() const
{
    using namespace pxr;
    TF_VERIFY(_renderDelegate);
    return _renderDelegate->IsStopSupported();
}

bool GLEngine::StopRenderer()
{
    using namespace pxr;
    TF_PY_ALLOW_THREADS_IN_SCOPE();

    TF_VERIFY(_renderDelegate);
    return _renderDelegate->Stop();
}

bool GLEngine::RestartRenderer()
{
    using namespace pxr;
    TF_PY_ALLOW_THREADS_IN_SCOPE();

    TF_VERIFY(_renderDelegate);
    return _renderDelegate->Restart();
}

//----------------------------------------------------------------------------
// Color Correction
//----------------------------------------------------------------------------
void GLEngine::SetColorCorrectionSettings(
    pxr::TfToken const &id)
{
    using namespace pxr;
    if (!IsColorCorrectionCapable())
    {
        return;
    }

    TF_VERIFY(_taskController);

    HdxColorCorrectionTaskParams hdParams;
    hdParams.colorCorrectionMode = id;
    _taskController->SetColorCorrectionParams(hdParams);
}

bool GLEngine::IsColorCorrectionCapable()
{
    return pxr::GlfContextCaps::GetInstance().floatingPointBuffersEnabled &&
           IsHydraEnabled();
}

//----------------------------------------------------------------------------
// Resource Information
//----------------------------------------------------------------------------

pxr::VtDictionary
GLEngine::GetRenderStats() const
{
    using namespace pxr;
    TF_VERIFY(_renderDelegate);
    return _renderDelegate->GetRenderStats();
}

pxr::Hgi *GLEngine::GetHgi()
{
    return _hgi.get();
}

//----------------------------------------------------------------------------
// Private/Protected
//----------------------------------------------------------------------------

pxr::HdRenderIndex *
GLEngine::_GetRenderIndex() const
{
    return _renderIndex.get();
}

void GLEngine::_Execute(const pxr::UsdImagingGLRenderParams &params,
                        pxr::HdTaskSharedPtrVector tasks)
{
    using namespace pxr;
    TF_VERIFY(_sceneDelegate);

    // User is responsible for initializing GL context and glew
    const bool isCoreProfileContext = GlfContextCaps::GetInstance().coreProfile;

    GLF_GROUP_FUNCTION();

    GLuint vao;
    if (isCoreProfileContext)
    {
        // We must bind a VAO (Vertex Array Object) because core profile
        // contexts do not have a default vertex array object. VAO objects are
        // container objects which are not shared between contexts, so we create
        // and bind a VAO here so that core rendering code does not have to
        // explicitly manage per-GL context state.
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    }
    else
    {
        glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // hydra orients all geometry during topological processing so that
    // front faces have ccw winding. We disable culling because culling
    // is handled by fragment shader discard.
    if (params.flipFrontFacing)
    {
        glFrontFace(GL_CW); // < State is pushed via GL_POLYGON_BIT
    }
    else
    {
        glFrontFace(GL_CCW); // < State is pushed via GL_POLYGON_BIT
    }
    glDisable(GL_CULL_FACE);

    if (params.applyRenderState)
    {
        glDisable(GL_BLEND);
    }

    // note: to get benefit of alpha-to-coverage, the target framebuffer
    // has to be a MSAA buffer.
    if (params.enableIdRender)
    {
        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }
    else if (params.enableSampleAlphaToCoverage)
    {
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }

    // for points width
    glEnable(GL_PROGRAM_POINT_SIZE);

    // TODO:
    //  * forceRefresh
    //  * showGuides, showRender, showProxy
    //  * gammaCorrectColors

    {
        // Release the GIL before calling into hydra, in case any hydra plugins
        // call into python.
        TF_PY_ALLOW_THREADS_IN_SCOPE();
        _engine->Execute(_renderIndex.get(), &tasks);
    }

    if (isCoreProfileContext)
    {

        glBindVertexArray(0);
        // XXX: We should not delete the VAO on every draw call, but we
        // currently must because it is GL Context state and we do not control
        // the context.
        glDeleteVertexArrays(1, &vao);
    }
    else
    {
        glPopAttrib(); // GL_ENABLE_BIT | GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT
    }
}

bool GLEngine::_CanPrepareBatch(
    const pxr::UsdPrim &root,
    const pxr::UsdImagingGLRenderParams &params)
{
    using namespace pxr;
    HD_TRACE_FUNCTION();

    if (!TF_VERIFY(root, "Attempting to draw an invalid/null prim\n"))
        return false;

    if (!root.GetPath().HasPrefix(_rootPath))
    {
        TF_CODING_ERROR("Attempting to draw path <%s>, but engine is rooted"
                        "at <%s>\n",
                        root.GetPath().GetText(),
                        _rootPath.GetText());
        return false;
    }

    return true;
}

static int
_GetRefineLevel(float c)
{
    // TODO: Change complexity to refineLevel when we refactor UsdImaging.
    //
    // Convert complexity float to refine level int.
    int refineLevel = 0;

    // to avoid floating point inaccuracy (e.g. 1.3 > 1.3f)
    c = std::min(c + 0.01f, 2.0f);

    if (1.0f <= c && c < 1.1f)
    {
        refineLevel = 0;
    }
    else if (1.1f <= c && c < 1.2f)
    {
        refineLevel = 1;
    }
    else if (1.2f <= c && c < 1.3f)
    {
        refineLevel = 2;
    }
    else if (1.3f <= c && c < 1.4f)
    {
        refineLevel = 3;
    }
    else if (1.4f <= c && c < 1.5f)
    {
        refineLevel = 4;
    }
    else if (1.5f <= c && c < 1.6f)
    {
        refineLevel = 5;
    }
    else if (1.6f <= c && c < 1.7f)
    {
        refineLevel = 6;
    }
    else if (1.7f <= c && c < 1.8f)
    {
        refineLevel = 7;
    }
    else if (1.8f <= c && c <= 2.0f)
    {
        refineLevel = 8;
    }
    else
    {
        using namespace pxr;
        TF_CODING_ERROR("Invalid complexity %f, expected range is [1.0,2.0]\n",
                        c);
    }
    return refineLevel;
}

void GLEngine::_PreSetTime(const pxr::UsdPrim &root,
                           const pxr::UsdImagingGLRenderParams &params)
{
    using namespace pxr;
    HD_TRACE_FUNCTION();

    // Set the fallback refine level, if this changes from the existing value,
    // all prim refine levels will be dirtied.
    const int refineLevel = _GetRefineLevel(params.complexity);
    _sceneDelegate->SetRefineLevelFallback(refineLevel);

    // Apply any queued up scene edits.
    _sceneDelegate->ApplyPendingUpdates();
}

void GLEngine::_PostSetTime(
    const pxr::UsdPrim &root,
    const pxr::UsdImagingGLRenderParams &params)
{
    HD_TRACE_FUNCTION();
}

/* static */
bool GLEngine::_UpdateHydraCollection(
    pxr::HdRprimCollection *collection,
    pxr::SdfPathVector const &roots,
    pxr::UsdImagingGLRenderParams const &params)
{
    using namespace pxr;

    if (collection == nullptr)
    {
        TF_CODING_ERROR("Null passed to _UpdateHydraCollection");
        return false;
    }

    // choose repr
    HdReprSelector reprSelector = HdReprSelector(HdReprTokens->smoothHull);
    const bool refined = params.complexity > 1.0;

    if (params.drawMode == UsdImagingGLDrawMode::DRAW_POINTS)
    {
        reprSelector = HdReprSelector(HdReprTokens->points);
    }
    else if (params.drawMode == UsdImagingGLDrawMode::DRAW_GEOM_FLAT ||
             params.drawMode == UsdImagingGLDrawMode::DRAW_SHADED_FLAT)
    {
        // Flat shading
        reprSelector = HdReprSelector(HdReprTokens->hull);
    }
    else if (
        params.drawMode == UsdImagingGLDrawMode::DRAW_WIREFRAME_ON_SURFACE)
    {
        // Wireframe on surface
        reprSelector = HdReprSelector(refined ? HdReprTokens->refinedWireOnSurf : HdReprTokens->wireOnSurf);
    }
    else if (params.drawMode == UsdImagingGLDrawMode::DRAW_WIREFRAME)
    {
        // Wireframe
        reprSelector = HdReprSelector(refined ? HdReprTokens->refinedWire : HdReprTokens->wire);
    }
    else
    {
        // Smooth shading
        reprSelector = HdReprSelector(refined ? HdReprTokens->refined : HdReprTokens->smoothHull);
    }

    // By default our main collection will be called geometry
    const TfToken colName = HdTokens->geometry;

    // Check if the collection needs to be updated (so we can avoid the sort).
    SdfPathVector const &oldRoots = collection->GetRootPaths();

    // inexpensive comparison first
    bool match = collection->GetName() == colName &&
                 oldRoots.size() == roots.size() &&
                 collection->GetReprSelector() == reprSelector;

    // Only take the time to compare root paths if everything else matches.
    if (match)
    {
        // Note that oldRoots is guaranteed to be sorted.
        for (size_t i = 0; i < roots.size(); i++)
        {
            // Avoid binary search when both vectors are sorted.
            if (oldRoots[i] == roots[i])
                continue;
            // Binary search to find the current root.
            if (!std::binary_search(oldRoots.begin(), oldRoots.end(), roots[i]))
            {
                match = false;
                break;
            }
        }

        // if everything matches, do nothing.
        if (match)
            return false;
    }

    // Recreate the collection.
    *collection = HdRprimCollection(colName, reprSelector);
    collection->SetRootPaths(roots);

    return true;
}

/* static */
pxr::HdxRenderTaskParams
GLEngine::_MakeHydraUsdImagingGLRenderParams(
    pxr::UsdImagingGLRenderParams const &renderParams)
{
    // Note this table is dangerous and making changes to the order of the
    // enums in UsdImagingGLCullStyle, will affect this with no compiler help.
    static const pxr::HdCullStyle USD_2_HD_CULL_STYLE[] =
        {
            pxr::HdCullStyleDontCare,              // Cull No Opinion (unused)
            pxr::HdCullStyleNothing,               // CULL_STYLE_NOTHING,
            pxr::HdCullStyleBack,                  // CULL_STYLE_BACK,
            pxr::HdCullStyleFront,                 // CULL_STYLE_FRONT,
            pxr::HdCullStyleBackUnlessDoubleSided, // CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED
        };
    static_assert(((sizeof(USD_2_HD_CULL_STYLE) /
                    sizeof(USD_2_HD_CULL_STYLE[0])) == (size_t)pxr::UsdImagingGLCullStyle::CULL_STYLE_COUNT),
                  "enum size mismatch");

    pxr::HdxRenderTaskParams params;

    params.overrideColor = renderParams.overrideColor;
    params.wireframeColor = renderParams.wireframeColor;

    if (renderParams.drawMode == pxr::UsdImagingGLDrawMode::DRAW_GEOM_ONLY ||
        renderParams.drawMode == pxr::UsdImagingGLDrawMode::DRAW_POINTS)
    {
        params.enableLighting = false;
    }
    else
    {
        params.enableLighting = renderParams.enableLighting &&
                                !renderParams.enableIdRender;
    }

    params.enableIdRender = renderParams.enableIdRender;
    params.depthBiasUseDefault = true;
    params.depthFunc = pxr::HdCmpFuncLess;
    params.cullStyle = USD_2_HD_CULL_STYLE[(size_t)renderParams.cullStyle];

    // Decrease the alpha threshold if we are using sample alpha to
    // coverage.
    if (renderParams.alphaThreshold < 0.0)
    {
        params.alphaThreshold =
            renderParams.enableSampleAlphaToCoverage ? 0.1f : 0.5f;
    }
    else
    {
        params.alphaThreshold =
            renderParams.alphaThreshold;
    }

    params.enableSceneMaterials = renderParams.enableSceneMaterials;

    // We don't provide the following because task controller ignores them:
    // - params.camera
    // - params.viewport

    return params;
}

//static
void GLEngine::_ComputeRenderTags(pxr::UsdImagingGLRenderParams const &params,
                                  pxr::TfTokenVector *renderTags)
{
    // Calculate the rendertags needed based on the parameters passed by
    // the application
    renderTags->clear();
    renderTags->reserve(4);
    renderTags->push_back(pxr::HdRenderTagTokens->geometry);
    if (params.showGuides)
    {
        renderTags->push_back(pxr::HdRenderTagTokens->guide);
    }
    if (params.showProxy)
    {
        renderTags->push_back(pxr::HdRenderTagTokens->proxy);
    }
    if (params.showRender)
    {
        renderTags->push_back(pxr::HdRenderTagTokens->render);
    }
}

/* static */
pxr::TfToken
GLEngine::_GetDefaultRendererPluginId()
{
    static const std::string defaultRendererDisplayName =
        pxr::TfGetenv("HD_DEFAULT_RENDERER", "");

    if (defaultRendererDisplayName.empty())
    {
        return pxr::TfToken();
    }

    pxr::HfPluginDescVector pluginDescs;
    pxr::HdRendererPluginRegistry::GetInstance().GetPluginDescs(&pluginDescs);

    // Look for the one with the matching display name
    for (size_t i = 0; i < pluginDescs.size(); ++i)
    {
        if (pluginDescs[i].displayName == defaultRendererDisplayName)
        {
            return pluginDescs[i].id;
        }
    }

    using namespace pxr;
    TF_WARN("Failed to find default renderer with display name '%s'.",
            defaultRendererDisplayName.c_str());

    return TfToken();
}

pxr::UsdImagingDelegate *
GLEngine::_GetSceneDelegate() const
{
    return _sceneDelegate.get();
}

pxr::HdEngine *
GLEngine::_GetHdEngine()
{
    return _engine.get();
}

pxr::HdxTaskController *
GLEngine::_GetTaskController() const
{
    return _taskController.get();
}
