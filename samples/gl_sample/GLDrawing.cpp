#include "GLEngine.h"
#include "GLDrawing.h"
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
    UsdImagingGLDrawMode _drawMode = UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
    GfVec4f _clearColor = GfVec4f(1.0f, 0.5f, 0.1f, 1.0f);
    pxr::UsdStageRefPtr _stage;
    std::shared_ptr<GLEngine> _engine;

    float _rotate[2] = {0, 0};
    GfVec3f _translate = GfVec3f(0.0f, 0.0f, -100.0f);
    int _mousePos[2] = {0, 0};
    bool _mouseButton[3] = {false, false, false};

public:
    Impl(const char *usdFile)
    {
        UsdImagingGL_UnitTestHelper_InitPlugins();
        _stage = pxr::UsdStage::Open(usdFile);
    }

    ~Impl()
    {
        std::cout << "GLDrawing::ShutdownTest()\n";
    }

    uint32_t Draw(int width, int height, const pxr::UsdTimeCode &time)
    {
        TRACE_FUNCTION();

        pxr::UsdImagingGLRenderParams params;
        params.drawMode = _drawMode;
        params.enableLighting = false;
        params.clearColor = _clearColor;
        params.frame = time;

        RenderFrameInfo frameInfo(_stage->GetPseudoRoot(), params);

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

        if (!_engine)
        {
            std::cout << "Using HD Renderer.\n";
            pxr::SdfPathVector excludedPaths;
            _engine.reset(new GLEngine(_GetDefaultRendererPluginId(),
                                       _stage->GetPseudoRoot().GetPath(), excludedPaths));
        }

        return _engine->RenderFrame(frameInfo);
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
};

GLDrawing::GLDrawing(const char *usdFile)
    : _impl(new Impl(usdFile))
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
