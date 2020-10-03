#include <pxr/imaging/glf/glew.h>
#include "GLEngine.h"
#include <assert.h>
#include <memory>
#include <pxr/usdImaging/usdImaging/delegate.h>
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/imaging/glf/diagnostic.h>
#include "pxr/imaging/glf/glContext.h"
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/imaging/hdx/taskController.h>
#include <pxr/imaging/hgi/tokens.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hd/driver.h>
#include <pxr/imaging/glf/simpleLightingContext.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/base/tf/getenv.h>
#include <pxr/base/arch/systemInfo.h>
// #include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/base/plug/registry.h>

static void UsdImagingGL_UnitTestHelper_InitPlugins()
{
    // Unfortunately, in order to properly find plugins in our test setup, we
    // need to know where the test is running.
    std::string testDir = pxr::TfGetPathName(pxr::ArchGetExecutablePath());
    std::string pluginDir = pxr::TfStringCatPaths(testDir,
                                                  "UsdImagingPlugins/lib/UsdImagingTest.framework/Resources");
    printf("registering plugins in: %s\n", pluginDir.c_str());

    pxr::PlugRegistry::GetInstance().RegisterPlugins(pluginDir);
}

static bool _InitGL()
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

    // Make sure there is an OpenGL context when
    // trying to initialize Hydra/Reference
    pxr::GlfGLContextSharedPtr context = pxr::GlfGLContext::GetCurrentGLContext();
    if (!context || !context->IsValid())
    {
        using namespace pxr;
        TF_CODING_ERROR("OpenGL context required, using reference renderer");
        return false;
    }

    return true;
}

bool IsColorCorrectionCapable()
{
    return pxr::GlfContextCaps::GetInstance().floatingPointBuffersEnabled;
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

//static
void _ComputeRenderTags(pxr::UsdImagingGLRenderParams const &params,
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
pxr::HdxRenderTaskParams
_MakeHydraUsdImagingGLRenderParams(
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

static pxr::TfToken _GetDefaultRendererPluginId()
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

class GLEngineImpl
{
    pxr::HgiUniquePtr _hgi;
    // Similar for HdDriver.
    pxr::HdDriver _hgiDriver;
    pxr::SdfPath const _sceneDelegateId;

    std::unique_ptr<pxr::HdxTaskController> _taskController;
    pxr::HdPluginRenderDelegateUniqueHandle _renderDelegate;
    std::unique_ptr<pxr::HdRenderIndex> _renderIndex;
    std::unique_ptr<pxr::UsdImagingDelegate> _sceneDelegate;
    std::unique_ptr<pxr::HdEngine> _engine;
    pxr::HdxSelectionTrackerSharedPtr _selTracker;
    pxr::GfVec4f _selectionColor;
    pxr::SdfPath _rootPath;
    pxr::SdfPathVector _excludedPrimPaths;
    pxr::SdfPathVector _invisedPrimPaths;
    bool _isPopulated;
    pxr::HdRprimCollection _renderCollection;
    pxr::GlfDrawTargetRefPtr _drawTarget;

public:
    GLEngineImpl(const pxr::SdfPath &rootPath)
        : _hgi(),
          _sceneDelegateId(pxr::SdfPath::AbsoluteRootPath()),
          _selTracker(std::make_shared<pxr::HdxSelectionTracker>()),
          _selectionColor(1.0f, 1.0f, 0.0f, 1.0f),
          _rootPath(rootPath),
          _isPopulated(false)
    {
        UsdImagingGL_UnitTestHelper_InitPlugins();

        // _renderIndex, _taskController, and _sceneDelegate are initialized
        // by the plugin system.
        auto rendererID = _GetDefaultRendererPluginId();
        if (!SetRendererPlugin(rendererID))
        {
            using namespace pxr;
            TF_CODING_ERROR("No renderer plugins found! "
                            "Check before creation.");
        }
    }

    ~GLEngineImpl()
    {
        TF_PY_ALLOW_THREADS_IN_SCOPE();

        // Destroy objects in opposite order of construction.
        _engine = nullptr;
        _taskController = nullptr;
        _sceneDelegate = nullptr;
        _renderIndex = nullptr;
        _renderDelegate = nullptr;
    }

    uint32_t RenderFrame(const RenderFrameInfo &info, const pxr::UsdPrim &root)
    {
        static bool s_glInitilazed = false;
        if (!s_glInitilazed)
        {
            if (!_InitGL())
            {
                assert(false);
                return 0;
            }
            s_glInitilazed = true;
        }

        auto width = info.viewport.data()[2];
        auto height = info.viewport.data()[3];

        if (!_drawTarget)
        {
            //
            // Create an offscreen draw target which is the same size as this
            // widget and initialize the unit test with the draw target bound.
            //
            _drawTarget = pxr::GlfDrawTarget::New(pxr::GfVec2i(width, height));
            _drawTarget->Bind();
            _drawTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);
            _drawTarget->AddAttachment("depth", GL_DEPTH_COMPONENT, GL_FLOAT,
                                       GL_DEPTH_COMPONENT);

            _drawTarget->Unbind();
        }
        _drawTarget->Bind();
        _drawTarget->SetSize(pxr::GfVec2i(width, height));

        SetRenderViewport(info.viewport);
        SetCameraState(info.modelViewMatrix, info.projectionMatrix);

        using namespace pxr;

        // TRACE_FUNCTION_SCOPE("test profile: renderTime");

        // renderTime.Start();

        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);

        pxr::UsdImagingGLRenderParams params;
        params.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
        params.enableLighting = false;
        params.clearColor = pxr::GfVec4f(1.0f, 0.5f, 0.1f, 1.0f);
        params.frame = pxr::UsdTimeCode::Default();
        for (int convergenceIterations = 0; true; convergenceIterations++)
        {
            TRACE_FUNCTION_SCOPE("iteration render convergence");

            glClearBufferfv(GL_COLOR, 0, params.clearColor.data());
            glClearBufferfv(GL_DEPTH, 0, info.clearDepth.data());

            Render(root, params);
            if (IsConverged())
            {
                break;
            }
        }

        {
            TRACE_FUNCTION_SCOPE("glFinish");
            glFinish();
        }

        // renderTime.Stop();

        _drawTarget->Unbind();

        return _drawTarget->GetFramebufferId();
    }

    void Render(const pxr::UsdPrim &root,
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

    bool IsConverged()
    {
        using namespace pxr;

        TF_VERIFY(_taskController);
        return _taskController->IsConverged();
    }

    void SetCameraState(const pxr::GfMatrix4d &viewMatrix,
                        const pxr::GfMatrix4d &projectionMatrix)
    {
        using namespace pxr;
        TF_VERIFY(_taskController);
        _taskController->SetFreeCameraMatrices(viewMatrix, projectionMatrix);
    }

    void SetRenderViewport(pxr::GfVec4d const &viewport)
    {
        using namespace pxr;
        TF_VERIFY(_taskController);
        _taskController->SetRenderViewport(viewport);
    }

private:
    void _InitializeHgiIfNecessary()
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

    bool SetRendererPlugin(pxr::TfToken const &id)
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

    void _SetRenderDelegateAndRestoreState(
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

    pxr::HdSelectionSharedPtr
    _GetSelection() const
    {
        if (pxr::HdSelectionSharedPtr const selection = _selTracker->GetSelectionMap())
        {
            return selection;
        }

        return std::make_shared<pxr::HdSelection>();
    }

    void _SetRenderDelegate(
        pxr::HdPluginRenderDelegateUniqueHandle &&renderDelegate)
    {
        // This relies on SetRendererPlugin to release the GIL...

        // Destruction
        // _DestroyHydraObjects();

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

    pxr::SdfPath
    _ComputeControllerPath(
        const pxr::HdPluginRenderDelegateUniqueHandle &renderDelegate)
    {
        const std::string pluginId =
            pxr::TfMakeValidIdentifier(renderDelegate.GetPluginId().GetText());
        const pxr::TfToken rendererName(
            pxr::TfStringPrintf("_UsdImaging_%s_%p", pluginId.c_str(), this));

        return _sceneDelegateId.AppendChild(rendererName);
    }

    bool _CanPrepareBatch(
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

    void _PreSetTime(const pxr::UsdPrim &root,
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

    void _PostSetTime(
        const pxr::UsdPrim &root,
        const pxr::UsdImagingGLRenderParams &params)
    {
        HD_TRACE_FUNCTION();
    }

    void PrepareBatch(
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

    void RenderBatch(
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

    void SetColorCorrectionSettings(
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

    /* static */
    bool _UpdateHydraCollection(
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

    void _Execute(const pxr::UsdImagingGLRenderParams &params,
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
};

//----------------------------------------------------------------------------
// Construction
//----------------------------------------------------------------------------
GLEngine::GLEngine(const pxr::SdfPath &rootPath)
    : _impl(new GLEngineImpl(rootPath))
{
}

GLEngine::~GLEngine()
{
    delete _impl;
}

uint32_t GLEngine::RenderFrame(const RenderFrameInfo &info, const pxr::UsdPrim &root)
{
    return _impl->RenderFrame(info, root);
}
