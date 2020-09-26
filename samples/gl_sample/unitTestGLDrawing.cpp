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
#include "pxr/pxr.h"
#include "pxr/imaging/glf/glew.h"

#include "unitTestGLDrawing.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/diagnostic.h"
#include "pxr/imaging/glf/drawTarget.h"
#include "pxr/imaging/garch/glDebugWindow.h"

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

////////////////////////////////////////////////////////////

class UsdImagingGL_UnitTestWindow : public GarchGLDebugWindow
{
public:
    typedef UsdImagingGL_UnitTestWindow This;

public:
    UsdImagingGL_UnitTestWindow(UsdImagingGL_UnitTestGLDrawing *unitTest,
                                int w, int h);
    virtual ~UsdImagingGL_UnitTestWindow();

    void DrawOffscreen();

    bool WriteToFile(std::string const &attachment,
                     std::string const &filename);

    // GarchGLDebugWIndow overrides;
    virtual void OnInitializeGL();
    virtual void OnUninitializeGL();
    virtual void OnPaintGL();
    virtual void OnKeyRelease(int key);
    virtual void OnMousePress(int button, int x, int y, int modKeys);
    virtual void OnMouseRelease(int button, int x, int y, int modKeys);
    virtual void OnMouseMove(int x, int y, int modKeys);

private:
    UsdImagingGL_UnitTestGLDrawing *_unitTest;
    GlfDrawTargetRefPtr _drawTarget;
};

UsdImagingGL_UnitTestWindow::UsdImagingGL_UnitTestWindow(
    UsdImagingGL_UnitTestGLDrawing *unitTest, int w, int h)
    : GarchGLDebugWindow("UsdImagingGL Test", w, h), _unitTest(unitTest)
{
}

UsdImagingGL_UnitTestWindow::~UsdImagingGL_UnitTestWindow()
{
}

/* virtual */
void UsdImagingGL_UnitTestWindow::OnInitializeGL()
{
    GlfGlewInit();
    GlfRegisterDefaultDebugOutputMessageCallback();
    GlfContextCaps::InitInstance();

    //
    // Create an offscreen draw target which is the same size as this
    // widget and initialize the unit test with the draw target bound.
    //
    _drawTarget = GlfDrawTarget::New(GfVec2i(GetWidth(), GetHeight()));
    _drawTarget->Bind();
    _drawTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);
    _drawTarget->AddAttachment("depth", GL_DEPTH_COMPONENT, GL_FLOAT,
                               GL_DEPTH_COMPONENT);

    _unitTest->InitTest();

    _drawTarget->Unbind();
}

/* virtual */
void UsdImagingGL_UnitTestWindow::OnUninitializeGL()
{
    _drawTarget = GlfDrawTargetRefPtr();

    _unitTest->ShutdownTest();
}

/* virtual */
void UsdImagingGL_UnitTestWindow::OnPaintGL()
{
    //
    // Update the draw target's size and execute the unit test with
    // the draw target bound.
    //
    int width = GetWidth();
    int height = GetHeight();
    _drawTarget->Bind();
    _drawTarget->SetSize(GfVec2i(width, height));

    _unitTest->DrawTest(false);

    _drawTarget->Unbind();

    //
    // Blit the resulting color buffer to the window (this is a noop
    // if we're drawing offscreen).
    //
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _drawTarget->GetFramebufferId());

    glBlitFramebuffer(0, 0, width, height,
                      0, 0, width, height,
                      GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void UsdImagingGL_UnitTestWindow::DrawOffscreen()
{
    _drawTarget->Bind();
    _drawTarget->SetSize(GfVec2i(GetWidth(), GetHeight()));

    _unitTest->DrawTest(true);

    _drawTarget->Unbind();
}

bool UsdImagingGL_UnitTestWindow::WriteToFile(std::string const &attachment,
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

/* virtual */
void UsdImagingGL_UnitTestWindow::OnKeyRelease(int key)
{
    switch (key)
    {
    case 'q':
        ExitApp();
        return;
    }
    _unitTest->KeyRelease(key);
}

/* virtual */
void UsdImagingGL_UnitTestWindow::OnMousePress(int button,
                                               int x, int y, int modKeys)
{
    _unitTest->MousePress(button, x, y, modKeys);
}

/* virtual */
void UsdImagingGL_UnitTestWindow::OnMouseRelease(int button,
                                                 int x, int y, int modKeys)
{
    _unitTest->MouseRelease(button, x, y, modKeys);
}

/* virtual */
void UsdImagingGL_UnitTestWindow::OnMouseMove(int x, int y, int modKeys)
{
    _unitTest->MouseMove(x, y, modKeys);
}

////////////////////////////////////////////////////////////

UsdImagingGL_UnitTestGLDrawing::UsdImagingGL_UnitTestGLDrawing()
    : _widget(NULL), _testLighting(false), _sceneLights(false), _cameraLight(false), _testIdRender(false), _complexity(1.0f), _drawMode(UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH), _shouldFrameAll(false), _cullBackfaces(false), _showGuides(UsdImagingGLRenderParams().showGuides), _showRender(UsdImagingGLRenderParams().showRender), _showProxy(UsdImagingGLRenderParams().showProxy), _clearOnce(false)
{
}

UsdImagingGL_UnitTestGLDrawing::~UsdImagingGL_UnitTestGLDrawing()
{
}

int UsdImagingGL_UnitTestGLDrawing::GetWidth() const
{
    return _widget->GetWidth();
}

int UsdImagingGL_UnitTestGLDrawing::GetHeight() const
{
    return _widget->GetHeight();
}

bool UsdImagingGL_UnitTestGLDrawing::WriteToFile(std::string const &attachment,
                                                 std::string const &filename) const
{
    return _widget->WriteToFile(attachment, filename);
}

struct UsdImagingGL_UnitTestGLDrawing::_Args
{
    _Args() : offscreen(false)
    {
        clearColor[0] = 1.0f;
        clearColor[1] = 0.5f;
        clearColor[2] = 0.1f;
        clearColor[3] = 1.0f;

        translate[0] = 0.0;
        translate[1] = -1000.0;
        translate[2] = -2500.0;
    }

    std::string unresolvedStageFilePath;
    bool offscreen;
    std::string shading;
    std::vector<double> clipPlaneCoords;
    std::vector<double> complexities;
    float clearColor[4];
    float translate[3];
};

static void Die(const char *fmt, ...) ARCH_PRINTF_FUNCTION(1, 2);
static void Die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fflush(stderr);
    exit(1);
}

static void
ParseError(const char *pname, const char *fmt, ...) ARCH_PRINTF_FUNCTION(2, 3);
static void
ParseError(const char *pname, const char *fmt, ...)
{
    fprintf(stderr, "%s: ", TfGetBaseName(pname).c_str());
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, ".  Try '%s -' for help.\n", TfGetBaseName(pname).c_str());
    fflush(stderr);
    exit(1);
}

static void Usage(int argc, char *argv[])
{
    static const char usage[] =
        "%s [-stage filePath] [-write filePath]\n"
        "                           [-offscreen] [-lighting] [-idRender]\n"
        "                           [-camera pathToCamera]\n"
        "                           [-complexity complexity]\n"
        "                           [-renderer rendererName]\n"
        "                           [-shading [flat|smooth|wire|wireOnSurface]]\n"
        "                           [-frameAll]\n"
        "                           [-clipPlane clipPlane1 ... clipPlane4]\n"
        "                           [-complexities complexities1 complexities2 ...]\n"
        "                           [-times times1 times2 ...] [-cullBackfaces]\n"
        "                           [-clear r g b a] [-clearOnce] [-translate x y z]\n"
        "                           [-renderSetting name type value]\n"
        "                           [-rendererAov name]\n"
        "                           [-perfStatsFile path]\n"
        "                           [-traceFile path] [...]\n"
        "\n"
        "  usdImaging basic drawing test\n"
        "\n"
        "options:\n"
        "  -stage filePath     name of usd stage to open []\n"
        "  -write filePath     name of image file to write (suffix determines type) []\n"
        "  -offscreen          execute without mapping a window\n"
        "  -lighting           use simple lighting override shader\n"
        "  -sceneLights        use in combination with -lighting to utilize the lights \n"
        "                      defined in the scene\n"
        "  -camLight           use a single camera light\n"
        "  -idRender           ID rendering\n"
        "  -complexity complexity\n"
        "                      Set the fallback complexity [1]\n"
        "  -renderer rendererName\n"
        "                      use the specified renderer plugin []\n"
        "  -shading [flat|smooth|wire|wireOnSurface]\n"
        "                      force specific type of shading\n"
        "                      [flat|smooth|wire|wireOnSurface] []\n"
        "  -frameAll           set the view to frame all root prims on the stage\n"
        "  -clipPlane clipPlane1 ... clipPlane4\n"
        "                      set an additional camera clipping plane [()]\n"
        "  -complexities complexities1 complexities2 ...\n"
        "                      One or more complexities, each complexity will\n"
        "                      produce an image [()]\n"
        "  -times times1 times2 ...\n"
        "                      One or more time samples, each time will produce\n"
        "                      an image [()]\n"
        "  -cullBackfaces      enable backface culling\n"
        "  -clear r g b a      clear color\n"
        "  -clearOnce          Clear the framebuffer only once at the start \n"
        "                      instead of before each render.\n"
        "  -translate x y z    default camera translation\n"
        "  -rendererAov name   Name of AOV to display or write out\n"
        "  -perfStatsFile path Path to file performance stats are written to\n"
        "  -traceFile path     Path to trace file to write\n"
        "  -renderSetting name type value\n"
        "                      Specifies a setting with given name, type (such as\n"
        "                      float) and value passed to renderer. -renderSetting\n"
        "                      can be given multiple times to specify different\n"
        "                      settings\n"
        "  -guidesPurpose [show|hide]\n"
        "                      force prims of purpose 'guide' to be shown or hidden\n"
        "  -renderPurpose [show|hide]\n"
        "                      force prims of purpose 'render' to be shown or hidden\n"
        "  -proxyPurpose [show|hide]\n"
        "                      force prims of purpose 'proxy' to be shown or hidden\n";

    Die(usage, TfGetBaseName(argv[0]).c_str());
}

static void CheckForMissingArguments(int i, int n, int argc, char *argv[])
{
    if (i + n >= argc)
    {
        if (n == 1)
        {
            ParseError(argv[0], "missing parameter for '%s'", argv[i]);
        }
        else
        {
            ParseError(argv[0], "argument '%s' requires %d values", argv[i], n);
        }
    }
}

static double ParseDouble(int &i, int argc, char *argv[],
                          bool *invalid = nullptr)
{
    if (i + 1 == argc)
    {
        if (invalid)
        {
            *invalid = true;
            return 0.0;
        }
        ParseError(argv[0], "missing parameter for '%s'", argv[i]);
    }
    char *end;
    double result = strtod(argv[i + 1], &end);
    if (end == argv[i + 1] || *end != '\0')
    {
        if (invalid)
        {
            *invalid = true;
            return 0.0;
        }
        ParseError(argv[0], "invalid parameter for '%s': %s",
                   argv[i], argv[i + 1]);
    }
    ++i;
    if (invalid)
    {
        *invalid = false;
    }
    return result;
}

static bool ParseShowHide(int &i, int argc, char *argv[],
                          bool *result)
{
    if (i + 1 == argc)
    {
        ParseError(argv[0], "missing parameter for '%s'", argv[i]);
        return false;
    }
    if (strcmp(argv[i + 1], "show") == 0)
    {
        *result = true;
    }
    else if (strcmp(argv[i + 1], "hide") == 0)
    {
        *result = false;
    }
    else
    {
        ParseError(argv[0], "invalid parameter for '%s': %s. Must be either "
                            "'show' or 'hide'",
                   argv[i], argv[i + 1]);
        return false;
    }

    ++i;
    return true;
}

static const char *ParseString(int &i, int argc, char *argv[],
                               bool *invalid = nullptr)
{
    if (i + 1 == argc)
    {
        if (invalid)
        {
            *invalid = true;
            return nullptr;
        }
        ParseError(argv[0], "missing parameter for '%s'", argv[i]);
    }
    const char *const result = argv[i + 1];
    ++i;
    if (invalid)
    {
        *invalid = false;
    }
    return result;
}

static void
ParseDoubleVector(
    int &i, int argc, char *argv[],
    std::vector<double> *result)
{
    bool invalid = false;
    while (i != argc)
    {
        const double value = ParseDouble(i, argc, argv, &invalid);
        if (invalid)
        {
            break;
        }
        result->push_back(value);
    }
}

static VtValue ParseVtValue(int &i, int argc, char *argv[])
{
    const char *const typeString = ParseString(i, argc, argv);

    if (strcmp(typeString, "float") == 0)
    {
        CheckForMissingArguments(i, 1, argc, argv);
        return VtValue(float(ParseDouble(i, argc, argv)));
    }
    else
    {
        ParseError(argv[0], "unknown type '%s'", typeString);
        return VtValue();
    }
}

void UsdImagingGL_UnitTestGLDrawing::_Parse(int argc, char *argv[], _Args *args)
{
    for (int i = 1; i != argc; ++i)
    {
        if (strcmp(argv[i], "-") == 0)
        {
            Usage(argc, argv);
        }
        else if (strcmp(argv[i], "-frameAll") == 0)
        {
            _shouldFrameAll = true;
        }
        else if (strcmp(argv[i], "-cullBackfaces") == 0)
        {
            _cullBackfaces = true;
        }
        else if (strcmp(argv[i], "-offscreen") == 0)
        {
            args->offscreen = true;
        }
        else if (strcmp(argv[i], "-lighting") == 0)
        {
            _testLighting = true;
        }
        else if (strcmp(argv[i], "-sceneLights") == 0)
        {
            _sceneLights = true;
        }
        else if (strcmp(argv[i], "-camlight") == 0)
        {
            _cameraLight = true;
        }
        else if (strcmp(argv[i], "-camera") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _cameraPath = argv[++i];
        }
        else if (strcmp(argv[i], "-idRender") == 0)
        {
            _testIdRender = true;
        }
        else if (strcmp(argv[i], "-stage") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            args->unresolvedStageFilePath = argv[++i];
        }
        else if (strcmp(argv[i], "-write") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _outputFilePath = argv[++i];
        }
        else if (strcmp(argv[i], "-shading") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            args->shading = argv[++i];
        }
        else if (strcmp(argv[i], "-complexity") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _complexity = ParseDouble(i, argc, argv);
        }
        else if (strcmp(argv[i], "-renderer") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _renderer = TfToken(argv[++i]);
        }
        else if (strcmp(argv[i], "-rendererAov") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _rendererAov = TfToken(argv[++i]);
        }
        else if (strcmp(argv[i], "-perfStatsFile") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _perfStatsFile = argv[++i];
        }
        else if (strcmp(argv[i], "-traceFile") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _traceFile = argv[++i];
        }
        else if (strcmp(argv[i], "-clipPlane") == 0)
        {
            CheckForMissingArguments(i, 4, argc, argv);
            args->clipPlaneCoords.push_back(ParseDouble(i, argc, argv));
            args->clipPlaneCoords.push_back(ParseDouble(i, argc, argv));
            args->clipPlaneCoords.push_back(ParseDouble(i, argc, argv));
            args->clipPlaneCoords.push_back(ParseDouble(i, argc, argv));
        }
        else if (strcmp(argv[i], "-complexities") == 0)
        {
            ParseDoubleVector(i, argc, argv, &args->complexities);
        }
        else if (strcmp(argv[i], "-times") == 0)
        {
            ParseDoubleVector(i, argc, argv, &_times);
        }
        else if (strcmp(argv[i], "-clear") == 0)
        {
            CheckForMissingArguments(i, 4, argc, argv);
            args->clearColor[0] = (float)ParseDouble(i, argc, argv);
            args->clearColor[1] = (float)ParseDouble(i, argc, argv);
            args->clearColor[2] = (float)ParseDouble(i, argc, argv);
            args->clearColor[3] = (float)ParseDouble(i, argc, argv);
        }
        else if (strcmp(argv[i], "-translate") == 0)
        {
            CheckForMissingArguments(i, 3, argc, argv);
            args->translate[0] = (float)ParseDouble(i, argc, argv);
            args->translate[1] = (float)ParseDouble(i, argc, argv);
            args->translate[2] = (float)ParseDouble(i, argc, argv);
        }
        else if (strcmp(argv[i], "-renderSetting") == 0)
        {
            CheckForMissingArguments(i, 2, argc, argv);
            const char *const key = ParseString(i, argc, argv);
            _renderSettings[key] = ParseVtValue(i, argc, argv);
        }
        else if (strcmp(argv[i], "-guidesPurpose") == 0)
        {
            ParseShowHide(i, argc, argv, &_showGuides);
        }
        else if (strcmp(argv[i], "-renderPurpose") == 0)
        {
            ParseShowHide(i, argc, argv, &_showRender);
        }
        else if (strcmp(argv[i], "-proxyPurpose") == 0)
        {
            ParseShowHide(i, argc, argv, &_showProxy);
        }
        else if (strcmp(argv[i], "-clearOnce") == 0)
        {
            _clearOnce = true;
        }
        else
        {
            ParseError(argv[0], "unknown argument %s", argv[i]);
        }
    }
}

void UsdImagingGL_UnitTestGLDrawing::RunTest(int argc, char *argv[])
{
    _Args args;
    _Parse(argc, argv, &args);

    if (!_traceFile.empty())
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
        _stageFilePath = args.unresolvedStageFilePath;
    }

    _widget = new UsdImagingGL_UnitTestWindow(this, 640, 480);
    _widget->Init();

    if (_times.empty())
    {
        _times.push_back(-999);
    }

    if (args.complexities.size() > 0)
    {
        std::string imageFilePath = GetOutputFilePath();

        TF_FOR_ALL(compIt, args.complexities)
        {
            _complexity = *compIt;
            if (!imageFilePath.empty())
            {
                std::stringstream suffix;
                suffix << "_" << _complexity << ".png";
                _outputFilePath = TfStringReplace(imageFilePath, ".png", suffix.str());
            }

            _widget->DrawOffscreen();
        }
    }
    else if (args.offscreen)
    {
        _widget->DrawOffscreen();
    }
    else
    {
        _widget->Run();
    }

    if (!_traceFile.empty())
    {
        TraceCollector::GetInstance().SetEnabled(false);

        {
            std::ofstream traceOutFile(_traceFile);
            if (TF_VERIFY(traceOutFile))
            {
                TraceReporter::GetGlobalReporter()->Report(traceOutFile);
            }
        }

        TraceCollector::GetInstance().Clear();
        TraceReporter::GetGlobalReporter()->ClearTree();
    }
}

GLuint vao;

void UsdImagingGL_UnitTestGLDrawing::InitTest()
{
    TRACE_FUNCTION();

    std::cout << "UsdImagingGL_UnitTestGLDrawing::InitTest()\n";
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

void UsdImagingGL_UnitTestGLDrawing::DrawTest(bool offscreen)
{
    TRACE_FUNCTION();

    std::cout << "UsdImagingGL_UnitTestGLDrawing::DrawTest()\n";

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

void UsdImagingGL_UnitTestGLDrawing::ShutdownTest()
{
    std::cout << "UsdImagingGL_UnitTestGLDrawing::ShutdownTest()\n";
    _engine->InvalidateBuffers();
}

void UsdImagingGL_UnitTestGLDrawing::MousePress(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = 1;
    _mousePos[0] = x;
    _mousePos[1] = y;
}

void UsdImagingGL_UnitTestGLDrawing::MouseRelease(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = 0;
}

void UsdImagingGL_UnitTestGLDrawing::MouseMove(int x, int y, int modKeys)
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

void UsdImagingGL_UnitTestGLDrawing::KeyRelease(int key)
{
}

PXR_NAMESPACE_CLOSE_SCOPE
