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
#include "unitTestGLDrawing.h"
#include "Args.h"

#include "pxr/pxr.h"

#include "pxr/base/arch/attributes.h"
#include "pxr/base/gf/vec2i.h"
#include "pxr/base/trace/collector.h"
#include "pxr/base/trace/reporter.h"

#include "pxr/base/plug/registry.h"
#include "pxr/base/arch/systemInfo.h"

#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/imaging/glf/simpleLightingContext.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImaging/tokens.h>

#include <stdio.h>
#include <stdarg.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

PXR_NAMESPACE_OPEN_SCOPE

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

UnitTestGLDrawing::UnitTestGLDrawing(const Args &a)
    : args(a)
{
    if (!args._traceFile.empty())
    {
        TraceCollector::GetInstance().SetEnabled(true);
    }

    UsdImagingGL_UnitTestHelper_InitPlugins();

    for (size_t i = 0; i < args.clipPlaneCoords.size() / 4; ++i)
    {
        _clipPlanes.push_back(GfVec4d(&args.clipPlaneCoords[i * 4]));
    }
    _clearColor = GfVec4f(args.clearColor);
    _translate = GfVec3f(args.translate);

    _drawMode = UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;

    if (args.shading.compare("wireOnSurface") == 0)
    {
        _drawMode = UsdImagingGLDrawMode::DRAW_WIREFRAME_ON_SURFACE;
    }
    else if (args.shading.compare("flat") == 0)
    {
        _drawMode = UsdImagingGLDrawMode::DRAW_SHADED_FLAT;
    }
    else if (args.shading.compare("wire") == 0)
    {
        _drawMode = UsdImagingGLDrawMode::DRAW_WIREFRAME;
    }

    if (!args.unresolvedStageFilePath.empty())
    {
        args._stageFilePath = args.unresolvedStageFilePath;
    }

    if (args._times.empty())
    {
        args._times.push_back(-999);
    }

    if (!args._traceFile.empty())
    {
        TraceCollector::GetInstance().SetEnabled(false);

        {
            std::ofstream traceOutFile(args._traceFile);
            if (TF_VERIFY(traceOutFile))
            {
                TraceReporter::GetGlobalReporter()->Report(traceOutFile);
            }
        }

        TraceCollector::GetInstance().Clear();
        TraceReporter::GetGlobalReporter()->ClearTree();
    }
}

UnitTestGLDrawing::~UnitTestGLDrawing()
{
}

void UnitTestGLDrawing::_InitTest(int width, int height)
{
    if(_initialized)
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

    _stage = pxr::UsdStage::Open(args.GetStageFilePath());
    pxr::SdfPathVector excludedPaths;

    if (pxr::UsdImagingGLEngine::IsHydraEnabled())
    {
        std::cout << "Using HD Renderer.\n";
        _engine.reset(new pxr::UsdImagingGLEngine(
            _stage->GetPseudoRoot().GetPath(), excludedPaths));
        if (!args._GetRenderer().IsEmpty())
        {
            if (!_engine->SetRendererPlugin(args._GetRenderer()))
            {
                std::cerr << "Couldn't set renderer plugin: " << args._GetRenderer().GetText() << std::endl;
                exit(-1);
            }
            else
            {
                std::cout << "Renderer plugin: " << args._GetRenderer().GetText()
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

    for (const auto &renderSetting : args.GetRenderSettings())
    {
        _engine->SetRendererSetting(pxr::TfToken(renderSetting.first),
                                    renderSetting.second);
    }

    std::cout << glGetString(GL_VENDOR) << "\n";
    std::cout << glGetString(GL_RENDERER) << "\n";
    std::cout << glGetString(GL_VERSION) << "\n";

    if (args._ShouldFrameAll())
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

    if (args.IsEnabledTestLighting())
    {
        if (pxr::UsdImagingGLEngine::IsHydraEnabled())
        {
            // set same parameter as GlfSimpleLightingContext::SetStateFromOpenGL
            // OpenGL defaults
            _lightingContext = pxr::GlfSimpleLightingContext::New();
            if (!args.IsEnabledSceneLights())
            {
                pxr::GlfSimpleLight light;
                if (args.IsEnabledCameraLight())
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
            if (args.IsEnabledCameraLight())
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

uint32_t UnitTestGLDrawing::DrawTest(int width, int height)
{
    _InitTest(width, height);

    TRACE_FUNCTION();

    std::cout << "UnitTestGLDrawing::DrawTest()\n";

    _drawTarget->Bind();
    _drawTarget->SetSize(pxr::GfVec2i(width, height));

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

    pxr::GfVec4d viewport(0, 0, width, height);

    if (args.GetCameraPath().empty())
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
        _engine->SetCameraPath(pxr::SdfPath(args.GetCameraPath()));
    }
    _engine->SetRenderViewport(viewport);

    bool const useAovs = !args.GetRendererAov().IsEmpty();
    pxr::GfVec4f fboClearColor = useAovs ? pxr::GfVec4f(0.0f) : GetClearColor();
    GLfloat clearDepth[1] = {1.0f};
    bool const clearOnlyOnce = args.ShouldClearOnce();
    bool cleared = false;

    pxr::UsdImagingGLRenderParams params;
    params.drawMode = GetDrawMode();
    params.enableLighting = args.IsEnabledTestLighting();
    params.enableIdRender = args.IsEnabledIdRender();
    params.complexity = args._GetComplexity();
    params.cullStyle = args.IsEnabledCullBackfaces() ? pxr::UsdImagingGLCullStyle::CULL_STYLE_BACK : pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
    params.showGuides = args.IsShowGuides();
    params.showRender = args.IsShowRender();
    params.showProxy = args.IsShowProxy();
    params.clearColor = GetClearColor();

    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);

    if (useAovs)
    {
        _engine->SetRendererAov(args.GetRendererAov());
    }

    if (args.IsEnabledTestLighting())
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

    for (double const &t : args.GetTimes())
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

        std::string imageFilePath = args.GetOutputFilePath();
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

    if (!args.GetPerfStatsFile().empty())
    {
        std::ofstream perfstatsRaw(args.GetPerfStatsFile(), std::ofstream::out);
        PXR_NAMESPACE_USING_DIRECTIVE;
        if (TF_VERIFY(perfstatsRaw))
        {
            perfstatsRaw << "{ 'profile'  : 'renderTime', "
                         << "   'metric'  : 'time', "
                         << "   'value'   : " << renderTime.GetSeconds() << ", "
                         << "   'samples' : " << args.GetTimes().size() << " }" << std::endl;
        }
    }

    _drawTarget->Unbind();

    return _drawTarget->GetFramebufferId();
}

void UnitTestGLDrawing::ShutdownTest()
{
    std::cout << "UnitTestGLDrawing::ShutdownTest()\n";

    _drawTarget = pxr::GlfDrawTargetRefPtr();

    _engine->InvalidateBuffers();
}

void UnitTestGLDrawing::MousePress(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = 1;
    _mousePos[0] = x;
    _mousePos[1] = y;
}

void UnitTestGLDrawing::MouseRelease(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = 0;
}

void UnitTestGLDrawing::MouseMove(int x, int y, int modKeys)
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

void UnitTestGLDrawing::KeyRelease(int key)
{
}

bool UnitTestGLDrawing::WriteToFile(std::string const &attachment,
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


PXR_NAMESPACE_CLOSE_SCOPE
