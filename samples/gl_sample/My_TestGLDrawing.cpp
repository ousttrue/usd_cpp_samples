#include "pxr/imaging/glf/glew.h"

#include "My_TestGLDrawing.h"
#include <iostream>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/imaging/glf/simpleLightingContext.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImaging/tokens.h>

#include <sstream>
#include <iomanip>
#include <fstream>

GLuint vao;

void My_TestGLDrawing::InitTest()
{
    TRACE_FUNCTION();

    std::cout << "My_TestGLDrawing::InitTest()\n";
    _stage = pxr::UsdStage::Open(GetStageFilePath());
    pxr::SdfPathVector excludedPaths;

    if (pxr::UsdImagingGLEngine::IsHydraEnabled())
    {
        std::cout << "Using HD Renderer.\n";
        _engine.reset(new pxr::UsdImagingGLEngine(
            _stage->GetPseudoRoot().GetPath(), excludedPaths));
        if (!_GetRenderer().IsEmpty())
        {
            if (!_engine->SetRendererPlugin(_GetRenderer()))
            {
                std::cerr << "Couldn't set renderer plugin: " << _GetRenderer().GetText() << std::endl;
                exit(-1);
            }
            else
            {
                std::cout << "Renderer plugin: " << _GetRenderer().GetText()
                          << std::endl;
            }
        }
    }
    else
    {
        std::cout << "Using Reference Renderer.\n";
        _engine.reset(
            new pxr::UsdImagingGLEngine(_stage->GetPseudoRoot().GetPath(),
                                        excludedPaths));
    }

    for (const auto &renderSetting : GetRenderSettings())
    {
        _engine->SetRendererSetting(pxr::TfToken(renderSetting.first),
                                    renderSetting.second);
    }

    std::cout << glGetString(GL_VENDOR) << "\n";
    std::cout << glGetString(GL_RENDERER) << "\n";
    std::cout << glGetString(GL_VERSION) << "\n";

    if (_ShouldFrameAll())
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

    if (IsEnabledTestLighting())
    {
        if (pxr::UsdImagingGLEngine::IsHydraEnabled())
        {
            // set same parameter as GlfSimpleLightingContext::SetStateFromOpenGL
            // OpenGL defaults
            _lightingContext = pxr::GlfSimpleLightingContext::New();
            if (!IsEnabledSceneLights())
            {
                pxr::GlfSimpleLight light;
                if (IsEnabledCameraLight())
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
        else
        {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            if (IsEnabledCameraLight())
            {
                float position[4] = {_translate[0], _translate[2], _translate[1], 0};
                glLightfv(GL_LIGHT0, GL_POSITION, position);
            }
            else
            {
                float position[4] = {0, -.5, .5, 0};
                glLightfv(GL_LIGHT0, GL_POSITION, position);
            }
        }
    }
}

void My_TestGLDrawing::DrawTest(bool offscreen)
{
    TRACE_FUNCTION();

    std::cout << "My_TestGLDrawing::DrawTest()\n";

    pxr::TfStopwatch renderTime;

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

    const int width = GetWidth();
    const int height = GetHeight();

    pxr::GfVec4d viewport(0, 0, width, height);

    if (GetCameraPath().empty())
    {
        pxr::GfMatrix4d viewMatrix(1.0);
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(0, 1, 0), _rotate[0]));
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1, 0, 0), _rotate[1]));
        viewMatrix *= pxr::GfMatrix4d().SetTranslate(pxr::GfVec3d(_translate[0], _translate[1], _translate[2]));

        pxr::GfMatrix4d modelViewMatrix = viewMatrix;
        if (pxr::UsdGeomGetStageUpAxis(_stage) == pxr::UsdGeomTokens->z)
        {
            // rotate from z-up to y-up
            modelViewMatrix =
                pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1.0, 0.0, 0.0), -90.0)) *
                modelViewMatrix;
        }

        const double aspectRatio = double(width) / height;
        pxr::GfFrustum frustum;
        frustum.SetPerspective(60.0, aspectRatio, 1, 100000.0);
        const pxr::GfMatrix4d projMatrix = frustum.ComputeProjectionMatrix();

        _engine->SetCameraState(modelViewMatrix, projMatrix);
    }
    else
    {
        _engine->SetCameraPath(pxr::SdfPath(GetCameraPath()));
    }
    _engine->SetRenderViewport(viewport);

    bool const useAovs = !GetRendererAov().IsEmpty();
    pxr::GfVec4f fboClearColor = useAovs ? pxr::GfVec4f(0.0f) : GetClearColor();
    GLfloat clearDepth[1] = {1.0f};
    bool const clearOnlyOnce = ShouldClearOnce();
    bool cleared = false;

    pxr::UsdImagingGLRenderParams params;
    params.drawMode = GetDrawMode();
    params.enableLighting = IsEnabledTestLighting();
    params.enableIdRender = IsEnabledIdRender();
    params.complexity = _GetComplexity();
    params.cullStyle = IsEnabledCullBackfaces() ? pxr::UsdImagingGLCullStyle::CULL_STYLE_BACK : pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
    params.showGuides = IsShowGuides();
    params.showRender = IsShowRender();
    params.showProxy = IsShowProxy();
    params.clearColor = GetClearColor();

    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);

    if (useAovs)
    {
        _engine->SetRendererAov(GetRendererAov());
    }

    if (IsEnabledTestLighting())
    {
        if (pxr::UsdImagingGLEngine::IsHydraEnabled())
        {
            _engine->SetLightingState(_lightingContext);
        }
        else
        {
            _engine->SetLightingStateFromOpenGL();
        }
    }

    if (!GetClipPlanes().empty())
    {
        params.clipPlanes = GetClipPlanes();
        for (size_t i = 0; i < GetClipPlanes().size(); ++i)
        {
            glEnable(GL_CLIP_PLANE0 + i);
        }
    }

    for (double const &t : GetTimes())
    {
        pxr::UsdTimeCode time = t;
        if (t == -999)
        {
            time = pxr::UsdTimeCode::Default();
        }

        params.frame = time;

        // Make sure we render to convergence.
        pxr::TfErrorMark mark;
        int convergenceIterations = 0;

        {
            TRACE_FUNCTION_SCOPE("test profile: renderTime");

            renderTime.Start();

            do
            {
                TRACE_FUNCTION_SCOPE("iteration render convergence");

                convergenceIterations++;

                if (cleared && clearOnlyOnce)
                {
                    // Don't clear the FBO
                }
                else
                {
                    glClearBufferfv(GL_COLOR, 0, fboClearColor.data());
                    glClearBufferfv(GL_DEPTH, 0, clearDepth);

                    cleared = true;
                }

                _engine->Render(_stage->GetPseudoRoot(), params);
            } while (!_engine->IsConverged());

            {
                TRACE_FUNCTION_SCOPE("glFinish");
                glFinish();
            }

            renderTime.Stop();
        }

        {
            PXR_NAMESPACE_USING_DIRECTIVE;
            TF_VERIFY(mark.IsClean(), "Errors occurred while rendering!");
        }

        std::cout << "Iterations to convergence: " << convergenceIterations << std::endl;
        std::cout << "itemsDrawn " << perfLog.GetCounter(pxr::HdTokens->itemsDrawn) << std::endl;
        std::cout << "totalItemCount " << perfLog.GetCounter(pxr::HdTokens->totalItemCount) << std::endl;

        std::string imageFilePath = GetOutputFilePath();
        if (!imageFilePath.empty())
        {
            if (time != pxr::UsdTimeCode::Default())
            {
                std::stringstream suffix;
                suffix << "_" << std::setw(3) << std::setfill('0') << params.frame << ".png";
                imageFilePath = pxr::TfStringReplace(imageFilePath, ".png", suffix.str());
            }
            std::cout << imageFilePath << "\n";
            WriteToFile("color", imageFilePath);
        }
    }

    if (!GetPerfStatsFile().empty())
    {
        std::ofstream perfstatsRaw(GetPerfStatsFile(), std::ofstream::out);
        PXR_NAMESPACE_USING_DIRECTIVE;
        if (TF_VERIFY(perfstatsRaw))
        {
            perfstatsRaw << "{ 'profile'  : 'renderTime', "
                         << "   'metric'  : 'time', "
                         << "   'value'   : " << renderTime.GetSeconds() << ", "
                         << "   'samples' : " << GetTimes().size() << " }" << std::endl;
        }
    }
}

void My_TestGLDrawing::ShutdownTest()
{
    std::cout << "My_TestGLDrawing::ShutdownTest()\n";
    _engine->InvalidateBuffers();
}

void My_TestGLDrawing::MousePress(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = 1;
    _mousePos[0] = x;
    _mousePos[1] = y;
}

void My_TestGLDrawing::MouseRelease(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = 0;
}

void My_TestGLDrawing::MouseMove(int x, int y, int modKeys)
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
