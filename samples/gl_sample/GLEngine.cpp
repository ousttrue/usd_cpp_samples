#include "GLEngine.h"
#include <assert.h>
#include <memory>
#include <pxr/usd/usd/prim.h>
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/imaging/glf/diagnostic.h>
#include "pxr/imaging/glf/glContext.h"
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgi/tokens.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/imaging/hd/driver.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h>
#include <pxr/imaging/hd/driver.h>
#include <pxr/imaging/hdx/taskController.h>
#include <pxr/imaging/glf/simpleLightingContext.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/base/tf/getenv.h>
#include <pxr/base/arch/systemInfo.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>

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
        // pxr::GlfGlewInit();

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
    GLEngineImpl(const CreateSceneDelegate &createSceneDelegate)
        : _selTracker(std::make_shared<pxr::HdxSelectionTracker>())
    {
        UsdImagingGL_UnitTestHelper_InitPlugins();

        // _renderIndex, _taskController, and _sceneDelegate are initialized
        // by the plugin system.
        auto id = _GetDefaultRendererPluginId();
        // if (!SetRendererPlugin(rendererID))
        // {
        // }

        {
            using namespace pxr;
            // _InitializeHgiIfNecessary();
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

            HdRendererPluginRegistry &registry =
                HdRendererPluginRegistry::GetInstance();

            // Special case: id = TfToken() selects the first plugin in the list.
            const TfToken resolvedId =
                id.IsEmpty() ? registry.GetDefaultPluginId() : id;

            // if (_renderDelegate && _renderDelegate.GetPluginId() == resolvedId)
            // {
            //     return true;
            // }

            TF_PY_ALLOW_THREADS_IN_SCOPE();

            HdPluginRenderDelegateUniqueHandle renderDelegate =
                registry.CreateRenderDelegate(resolvedId);
            if (!renderDelegate)
            {
                using namespace pxr;
                TF_CODING_ERROR("No renderer plugins found! "
                                "Check before creation.");
            }

            // _SetRenderDelegate(std::move(renderDelegate));
            {
                // This relies on SetRendererPlugin to release the GIL...

                // Destruction
                // _DestroyHydraObjects();

                // _isPopulated = false;

                // Creation

                // Use the new render delegate.
                _renderDelegate = std::move(renderDelegate);

                // Recreate the render index
                _renderIndex.reset(
                    pxr::HdRenderIndex::New(
                        _renderDelegate.Get(), {&_hgiDriver}));

                // Create the new delegate
                createSceneDelegate(_renderIndex.get());

                // Create the new task controller
                _taskController = std::make_unique<pxr::HdxTaskController>(
                    _renderIndex.get(),
                    _ComputeControllerPath(_renderDelegate));

                // The task context holds on to resources in the render
                // deletegate, so we want to destroy it first and thus
                // create it last.
                _engine = std::make_unique<pxr::HdEngine>();
            }

            // return true;
        }
    }

    ~GLEngineImpl()
    {
        TF_PY_ALLOW_THREADS_IN_SCOPE();

        // Destroy objects in opposite order of construction.
        _engine = nullptr;
        _taskController = nullptr;
        _renderIndex = nullptr;
        _renderDelegate = nullptr;
    }

    uint32_t RenderFrame(const RenderFrameInfo &info, const pxr::SdfPathVector &paths)
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

            Render(params, paths);
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

    void Render(const pxr::UsdImagingGLRenderParams &params,
                const pxr::SdfPathVector &paths)
    {
        using namespace pxr;

        TF_VERIFY(_taskController);

        // RenderBatch(paths, params);
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

            VtValue selectionValue(_selTracker);
            _engine->SetTaskContextData(HdxTokens->selectionState, selectionValue);
            _Execute(params, _taskController->GetRenderingTasks());
        }
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
    pxr::SdfPath
    _ComputeControllerPath(
        const pxr::HdPluginRenderDelegateUniqueHandle &renderDelegate)
    {
        const std::string pluginId =
            pxr::TfMakeValidIdentifier(renderDelegate.GetPluginId().GetText());
        const pxr::TfToken rendererName(
            pxr::TfStringPrintf("_UsdImaging_%s_%p", pluginId.c_str(), this));

        return pxr::SdfPath::AbsoluteRootPath().AppendChild(rendererName);
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
GLEngine::GLEngine(const CreateSceneDelegate &createSceneDelegate)
    : _impl(new GLEngineImpl(createSceneDelegate))
{
}

GLEngine::~GLEngine()
{
    delete _impl;
}

uint32_t GLEngine::RenderFrame(const RenderFrameInfo &info, const pxr::SdfPathVector &paths)
{
    return _impl->RenderFrame(info, paths);
}
