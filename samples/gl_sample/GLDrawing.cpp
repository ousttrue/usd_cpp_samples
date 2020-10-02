#include "GLEngine.h"
#include "GLDrawing.h"
#include "Args.h"
#include <pxr/base/trace/collector.h>
#include <pxr/base/trace/reporter.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/plug/registry.h>
#include <pxr/base/arch/systemInfo.h>
#include <pxr/base/tf/getenv.h>
#include <pxr/base/tf/envSetting.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/imaging/glf/simpleLightingContext.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/usdImaging/usdImaging/tokens.h>

#include <stdio.h>
#include <stdarg.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace pxr;

bool _GetHydraEnabledEnvVar()
{
    // XXX: Note that we don't cache the result here.  This is primarily because
    // of the way usdview currently interacts with this setting.  This should
    // be cleaned up, and the new class hierarchy around GLEngine
    // makes it much easier to do so.
    return pxr::TfGetenv("HD_ENABLED", "1") == "1";
}

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_ENV_SETTING(USDIMAGINGGL_ENGINE_DEBUG_SCENE_DELEGATE_ID, "/",
                      "Default usdImaging scene delegate id");

PXR_NAMESPACE_CLOSE_SCOPE

pxr::SdfPath const &
_GetUsdImagingDelegateId()
{
    static pxr::SdfPath const delegateId =
        pxr::SdfPath(pxr::TfGetEnvSetting(pxr::USDIMAGINGGL_ENGINE_DEBUG_SCENE_DELEGATE_ID));

    return delegateId;
}

bool _IsHydraEnabled()
{

    if (!_GetHydraEnabledEnvVar())
    {
        return false;
    }

    // Check to see if we have a default plugin for the renderer
    pxr::TfToken defaultPlugin =
        pxr::HdRendererPluginRegistry::GetInstance().GetDefaultPluginId();

    return !defaultPlugin.IsEmpty();
}

static void UsdImagingGL_UnitTestHelper_InitPlugins()
{
    // Unfortunately, in order to properly find plugins in our test setup, we
    // need to know where the test is running.
    std::string testDir = TfGetPathName(ArchGetExecutablePath());
    std::string pluginDir = TfStringCatPaths(testDir,
                                             "UsdImagingPlugins/lib/UsdImagingTest.framework/Resources");
    printf("registering plugins in: %s\n", pluginDir.c_str());

    PlugRegistry::GetInstance().RegisterPlugins(pluginDir);
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

class Impl
{
    Args _args;
    UsdImagingGLDrawMode _drawMode = UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
    GfVec4f _clearColor;
    GfVec3f _translate;
    bool _initialized = false;
    pxr::GlfDrawTargetRefPtr _drawTarget;
    pxr::UsdStageRefPtr _stage;
    std::shared_ptr<GLEngine> _engine;
    pxr::GlfSimpleLightingContextRefPtr _lightingContext;
    float _rotate[2] = {0, 0};
    float __translate[3] = {0, 0, 0};
    int _mousePos[2] = {0, 0};
    bool _mouseButton[3] = {false, false, false};

public:
    Impl(const Args &args)
        : _args(args)
    {
        UsdImagingGL_UnitTestHelper_InitPlugins();

        _clearColor = GfVec4f(_args.clearColor);
        _translate = GfVec3f(_args.translate);

        _drawMode = UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;

        if (_args.shading.compare("wireOnSurface") == 0)
        {
            _drawMode = UsdImagingGLDrawMode::DRAW_WIREFRAME_ON_SURFACE;
        }
        else if (_args.shading.compare("flat") == 0)
        {
            _drawMode = UsdImagingGLDrawMode::DRAW_SHADED_FLAT;
        }
        else if (_args.shading.compare("wire") == 0)
        {
            _drawMode = UsdImagingGLDrawMode::DRAW_WIREFRAME;
        }

        if (!_args.unresolvedStageFilePath.empty())
        {
            _args._stageFilePath = _args.unresolvedStageFilePath;
        }
    }

    ~Impl()
    {
        std::cout << "GLDrawing::ShutdownTest()\n";
        // _drawTarget = pxr::GlfDrawTargetRefPtr();
        // _engine->InvalidateBuffers();
    }

    uint32_t Draw(int width, int height, const pxr::UsdTimeCode &time)
    {
        _Init(width, height);

        TRACE_FUNCTION();

        std::cout << "UnitTestGLDrawing::DrawTest()\n";

        _drawTarget->Bind();
        _drawTarget->SetSize(pxr::GfVec2i(width, height));

        // pxr::TfStopwatch renderTime;

        auto &perfLog = pxr::HdPerfLog::GetInstance();
        perfLog.Enable();

        // Reset all counters we care about.
        perfLog.ResetCache(pxr::HdTokens->extent);
        perfLog.ResetCache(pxr::HdTokens->points);
        perfLog.ResetCache(pxr::HdTokens->topology);
        perfLog.ResetCache(pxr::HdTokens->transform);
        perfLog.SetCounter(pxr::UsdImagingTokens->usdVaryingExtent, 0);
        perfLog.SetCounter(pxr::UsdImagingTokens->usdVaryingPrimvar, 0);
        perfLog.SetCounter(pxr::UsdImagingTokens->usdVaryingTopology, 0);
        perfLog.SetCounter(pxr::UsdImagingTokens->usdVaryingVisibility, 0);
        perfLog.SetCounter(pxr::UsdImagingTokens->usdVaryingXform, 0);

        pxr::UsdImagingGLRenderParams params;
        params.drawMode = GetDrawMode();
        params.enableLighting = _args.IsEnabledTestLighting();
        params.enableIdRender = _args.IsEnabledIdRender();
        params.complexity = _args._GetComplexity();
        params.cullStyle = _args.IsEnabledCullBackfaces() ? pxr::UsdImagingGLCullStyle::CULL_STYLE_BACK : pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
        params.clearColor = GetClearColor();

        RenderFrameInfo frameInfo(_stage->GetPseudoRoot(), params);

        bool const useAovs = !_args.GetRendererAov().IsEmpty();
        frameInfo.fboClearColor = useAovs ? pxr::GfVec4f(0.0f) : GetClearColor();
        frameInfo.clearDepth = {1.0f};

        frameInfo.viewport = pxr::GfVec4d(0, 0, width, height);
        {
            pxr::GfMatrix4d viewMatrix(1.0);
            viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(0, 1, 0), _rotate[0]));
            viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1, 0, 0), _rotate[1]));
            viewMatrix *= pxr::GfMatrix4d().SetTranslate(pxr::GfVec3d(_translate[0], _translate[1], _translate[2]));

            frameInfo.modelViewMatrix = viewMatrix;
            if (pxr::UsdGeomGetStageUpAxis(_stage) == pxr::UsdGeomTokens->z)
            {
                // rotate from z-up to y-up
                frameInfo.modelViewMatrix =
                    pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1.0, 0.0, 0.0), -90.0)) *
                    frameInfo.modelViewMatrix;
            }

            const double aspectRatio = double(width) / height;
            pxr::GfFrustum frustum;
            frustum.SetPerspective(60.0, aspectRatio, 1, 100000.0);
            frameInfo.projectionMatrix = frustum.ComputeProjectionMatrix();
        }

        glViewport(0, 0, width, height);

        glEnable(GL_DEPTH_TEST);

        {
            params.frame = time;

            // Make sure we render to convergence.
            pxr::TfErrorMark mark;

            _engine->RenderFrame(frameInfo);

            {
                PXR_NAMESPACE_USING_DIRECTIVE;
                TF_VERIFY(mark.IsClean(), "Errors occurred while rendering!");
            }

            // std::cout << "Iterations to convergence: " << convergenceIterations << std::endl;
            std::cout << "itemsDrawn " << perfLog.GetCounter(pxr::HdTokens->itemsDrawn) << std::endl;
            std::cout << "totalItemCount " << perfLog.GetCounter(pxr::HdTokens->totalItemCount) << std::endl;

            std::string imageFilePath = _args.GetOutputFilePath();
            if (!imageFilePath.empty())
            {
                if (time != pxr::UsdTimeCode::Default())
                {
                    std::stringstream suffix;
                    suffix << "_" << std::setw(3) << std::setfill('0') << params.frame << ".png";
                    imageFilePath = pxr::TfStringReplace(imageFilePath, ".png", suffix.str());
                }
                std::cout << imageFilePath << "\n";
                // TODO:
                WriteToFile("color", imageFilePath);
            }
        }

        _drawTarget->Unbind();

        return _drawTarget->GetFramebufferId();
    }

    void MousePress(int button, int x, int y, int modKeys)
    {
        _mouseButton[button] = 1;
        _mousePos[0] = x;
        _mousePos[1] = y;
    }

    void MouseRelease(int button, int x, int y, int modKeys)
    {
        _mouseButton[button] = 0;
    }

    void MouseMove(int x, int y, int modKeys)
    {
        int dx = x - _mousePos[0];
        int dy = y - _mousePos[1];

        if (_mouseButton[0])
        {
            _rotate[0] += dx;
            _rotate[1] += dy;
        }
        else if (_mouseButton[1])
        {
            _translate[0] += dx;
            _translate[1] -= dy;
        }
        else if (_mouseButton[2])
        {
            _translate[2] += dx;
        }

        _mousePos[0] = x;
        _mousePos[1] = y;
    }

    void KeyRelease(int key)
    {
    }

private:
    UsdImagingGLDrawMode GetDrawMode() const { return _drawMode; }
    GfVec4f const &GetClearColor() const { return _clearColor; }
    GfVec3f const &GetTranslate() const { return _translate; }
    bool WriteToFile(std::string const &attachment,
                     std::string const &filename)
    {
        // We need to unbind the draw target before writing to file to be sure the
        // attachment is in a good state.
        bool isBound = _drawTarget->IsBound();
        if (isBound)
            _drawTarget->Unbind();

        bool result = _drawTarget->WriteToFile(attachment, filename);

        if (isBound)
            _drawTarget->Bind();
        return result;
    }

    void _Init(int width, int height)
    {
        if (_initialized)
        {
            return;
        }
        _initialized = true;

        TRACE_FUNCTION();

        std::cout << "UnitTestGLDrawing::InitTest()\n";

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

        _stage = pxr::UsdStage::Open(_args.GetStageFilePath());
        pxr::SdfPathVector excludedPaths;

        {
            std::cout << "Using HD Renderer.\n";
            _engine.reset(new GLEngine(_GetDefaultRendererPluginId(),
                                       _stage->GetPseudoRoot().GetPath(), excludedPaths));
        }

        std::cout << glGetString(GL_VENDOR) << "\n";
        std::cout << glGetString(GL_RENDERER) << "\n";
        std::cout << glGetString(GL_VERSION) << "\n";

        if (_args._ShouldFrameAll())
        {
            pxr::TfTokenVector purposes;
            purposes.push_back(pxr::UsdGeomTokens->default_);
            purposes.push_back(pxr::UsdGeomTokens->proxy);

            // Extent hints are sometimes authored as an optimization to avoid
            // computing bounds, they are particularly useful for some tests where
            // there is no bound on the first frame.
            bool useExtentHints = true;
            pxr::UsdGeomBBoxCache bboxCache(pxr::UsdTimeCode::Default(), purposes, useExtentHints);

            pxr::GfBBox3d bbox = bboxCache.ComputeWorldBound(_stage->GetPseudoRoot());
            pxr::GfRange3d world = bbox.ComputeAlignedRange();

            pxr::GfVec3d worldCenter = (world.GetMin() + world.GetMax()) / 2.0;
            double worldSize = world.GetSize().GetLength();

            std::cerr << "worldCenter: " << worldCenter << "\n";
            std::cerr << "worldSize: " << worldSize << "\n";
            if (pxr::UsdGeomGetStageUpAxis(_stage) == pxr::UsdGeomTokens->z)
            {
                // transpose y and z centering translation
                _translate[0] = -worldCenter[0];
                _translate[1] = -worldCenter[2];
                _translate[2] = -worldCenter[1] - worldSize;
            }
            else
            {
                _translate[0] = -worldCenter[0];
                _translate[1] = -worldCenter[1];
                _translate[2] = -worldCenter[2] - worldSize;
            }
        }
        else
        {
            _translate[0] = GetTranslate()[0];
            _translate[1] = GetTranslate()[1];
            _translate[2] = GetTranslate()[2];
        }

        if (_args.IsEnabledTestLighting())
        {
            // set same parameter as GlfSimpleLightingContext::SetStateFromOpenGL
            // OpenGL defaults
            _lightingContext = pxr::GlfSimpleLightingContext::New();
            if (!_args.IsEnabledSceneLights())
            {
                pxr::GlfSimpleLight light;
                if (_args.IsEnabledCameraLight())
                {
                    light.SetPosition(pxr::GfVec4f(_translate[0], _translate[2], _translate[1], 0));
                }
                else
                {
                    light.SetPosition(pxr::GfVec4f(0, -.5, .5, 0));
                }
                light.SetDiffuse(pxr::GfVec4f(1, 1, 1, 1));
                light.SetAmbient(pxr::GfVec4f(0, 0, 0, 1));
                light.SetSpecular(pxr::GfVec4f(1, 1, 1, 1));
                pxr::GlfSimpleLightVector lights;
                lights.push_back(light);
                _lightingContext->SetLights(lights);
            }

            pxr::GlfSimpleMaterial material;
            material.SetAmbient(pxr::GfVec4f(0.2, 0.2, 0.2, 1.0));
            material.SetDiffuse(pxr::GfVec4f(0.8, 0.8, 0.8, 1.0));
            material.SetSpecular(pxr::GfVec4f(0, 0, 0, 1));
            material.SetShininess(0.0001f);
            _lightingContext->SetMaterial(material);
            _lightingContext->SetSceneAmbient(pxr::GfVec4f(0.2, 0.2, 0.2, 1.0));
        }
    }
};

GLDrawing::GLDrawing(const struct Args &args)
    : _impl(new Impl(args))
{
}
GLDrawing::~GLDrawing()
{
    Shutdown();
}
uint32_t GLDrawing::Draw(int w, int h, const pxr::UsdTimeCode &time)
{
    // return fbo id
    if (!_impl)
        return 0;
    return _impl->Draw(w, h, time);
}
void GLDrawing::Shutdown()
{
    if (_impl)
    {
        delete _impl;
        _impl = nullptr;
    }
}
void GLDrawing::MousePress(int button, int x, int y, int modKeys)
{
    if (!_impl)
        return;
    _impl->MousePress(button, x, y, modKeys);
}
void GLDrawing::MouseRelease(int button, int x, int y, int modKeys)
{
    if (!_impl)
        return;
    _impl->MouseRelease(button, x, y, modKeys);
}
void GLDrawing::MouseMove(int x, int y, int modKeys)
{
    if (!_impl)
        return;
    _impl->MouseMove(x, y, modKeys);
}
void GLDrawing::KeyRelease(int key)
{
    if (!_impl)
        return;
    _impl->KeyRelease(key);
}
