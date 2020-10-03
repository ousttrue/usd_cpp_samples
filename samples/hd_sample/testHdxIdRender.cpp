#include "pxr/imaging/glf/glew.h"
#include "testHdxRenderer.h"
#include "unitTestDelegate.h"
#include "pxr/imaging/hgi/tokens.h"
#include "pxr/imaging/hdx/renderTask.h"
#include "pxr/imaging/hdx/pickTask.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/rotation.h"
#include "pxr/base/gf/vec2i.h"
#include "pxr/base/gf/vec4d.h"
#include <pxr/imaging/garch/glDebugWindow.h>
#include <iostream>
#include <memory>


GLuint vao;

static pxr::GfMatrix4d
_GetTranslate(float tx, float ty, float tz)
{
    pxr::GfMatrix4d m(1.0f);
    m.SetRow(3, pxr::GfVec4f(tx, ty, tz, 1.0));
    return m;
}

PXR_NAMESPACE_USING_DIRECTIVE

My_TestGLDrawing::My_TestGLDrawing()
{
    _rotate[0] = _rotate[1] = 0;
    _translate[0] = _translate[1] = _translate[2] = 0;

    _mousePos[0] = _mousePos[1] = 0;
    _mouseButton[0] = _mouseButton[1] = _mouseButton[2] = false;

    SetCameraRotate(0, 0);
    SetCameraTranslate(pxr::GfVec3f(0));
    _reprName = pxr::HdReprTokens->hull;
    _refineLevel = 0;
}

void My_TestGLDrawing::InitTest(int width, int height)
{
    //
    // Create an offscreen draw target which is the same size as this
    // widget and initialize the unit test with the draw target bound.
    //
    _drawTarget = pxr::GlfDrawTarget::New(pxr::GfVec2i(width, height));
    _drawTarget->Bind();
    _drawTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);
    _drawTarget->AddAttachment("depth", GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                               GL_DEPTH24_STENCIL8);
    _drawTarget->Unbind();

    _hgi = Hgi::CreatePlatformDefaultHgi();
    _driver.reset(new HdDriver{HgiTokens->renderDriver, VtValue(_hgi.get())});

    _renderIndex = HdRenderIndex::New(&_renderDelegate, {_driver.get()});
    TF_VERIFY(_renderIndex != nullptr);
    _delegate = new Hdx_UnitTestDelegate(_renderIndex);

    _delegate->SetRefineLevel(_refineLevel);

    // prepare render task
    SdfPath renderSetupTask("/renderSetupTask");
    SdfPath renderTask("/renderTask");
    _delegate->AddRenderSetupTask(renderSetupTask);
    _delegate->AddRenderTask(renderTask);

    // render task parameters.
    HdxRenderTaskParams param = _delegate->GetTaskParam(
                                             renderSetupTask, HdTokens->params)
                                    .Get<HdxRenderTaskParams>();
    param.enableLighting = true; // use default lighting
    _delegate->SetTaskParam(renderSetupTask, HdTokens->params, VtValue(param));
    _delegate->SetTaskParam(renderTask, HdTokens->collection,
                            VtValue(HdRprimCollection(HdTokens->geometry,
                                                      HdReprSelector(_reprName))));

    // prepare scene
    // To ensure that the non-aggregated element index returned via picking,
    // we need to have at least two cubes with uniform colors.
    GfVec4f red(1, 0, 0, 1), green(0, 1, 0, 1), blue(0, 0, 1, 1),
        yellow(1, 1, 0, 1), magenta(1, 0, 1, 1), cyan(0, 1, 1, 1),
        white(1, 1, 1, 1), black(0, 0, 0, 1);

    GfVec4f faceColors[] = {red, green, blue, yellow, magenta, cyan};
    VtValue faceColor = VtValue(_BuildArray(&faceColors[0],
                                            sizeof(faceColors) / sizeof(faceColors[0])));

    GfVec4f vertColors[] = {white, blue, green, yellow,
                            black, blue, magenta, red};
    VtValue vertColor = VtValue(_BuildArray(&vertColors[0],
                                            sizeof(vertColors) / sizeof(vertColors[0])));

    _delegate->AddCube(SdfPath("/cube0"), _GetTranslate(5, 0, 5),
                       /*guide=*/false, /*instancerId=*/SdfPath(),
                       /*scheme=*/PxOsdOpenSubdivTokens->catmullClark,
                       /*color=*/faceColor,
                       /*colorInterpolation=*/HdInterpolationUniform);
    _delegate->AddCube(SdfPath("/cube1"), _GetTranslate(-5, 0, 5),
                       /*guide=*/false, /*instancerId=*/SdfPath(),
                       /*scheme=*/PxOsdOpenSubdivTokens->catmullClark,
                       /*color=*/faceColor,
                       /*colorInterpolation=*/HdInterpolationUniform);
    _delegate->AddCube(SdfPath("/cube2"), _GetTranslate(-5, 0, -5));
    _delegate->AddCube(SdfPath("/cube3"), _GetTranslate(5, 0, -5),
                       /*guide=*/false, /*instancerId=*/SdfPath(),
                       /*scheme=*/PxOsdOpenSubdivTokens->catmullClark,
                       /*color=*/vertColor,
                       /*colorInterpolation=*/HdInterpolationVertex);

    {
        _delegate->AddInstancer(SdfPath("/instancerTop"));
        _delegate->AddCube(SdfPath("/protoTop"),
                           GfMatrix4d(1), false, SdfPath("/instancerTop"));

        std::vector<SdfPath> prototypes;
        prototypes.push_back(SdfPath("/protoTop"));

        VtVec3fArray scale(3);
        VtVec4fArray rotate(3);
        VtVec3fArray translate(3);
        VtIntArray prototypeIndex(3);

        scale[0] = GfVec3f(1);
        rotate[0] = GfVec4f(0);
        translate[0] = GfVec3f(3, 0, 2);
        prototypeIndex[0] = 0;

        scale[1] = GfVec3f(1);
        rotate[1] = GfVec4f(0);
        translate[1] = GfVec3f(0, 0, 2);
        prototypeIndex[1] = 0;

        scale[2] = GfVec3f(1);
        rotate[2] = GfVec4f(0);
        translate[2] = GfVec3f(-3, 0, 2);
        prototypeIndex[2] = 0;

        _delegate->SetInstancerProperties(SdfPath("/instancerTop"),
                                          prototypeIndex,
                                          scale, rotate, translate);
    }

    {
        _delegate->AddInstancer(SdfPath("/instancerBottom"));
        _delegate->AddCube(SdfPath("/protoBottom"),
                           GfMatrix4d(1), false, SdfPath("/instancerBottom"));

        std::vector<SdfPath> prototypes;
        prototypes.push_back(SdfPath("/protoBottom"));

        VtVec3fArray scale(3);
        VtVec4fArray rotate(3);
        VtVec3fArray translate(3);
        VtIntArray prototypeIndex(3);

        scale[0] = GfVec3f(1);
        rotate[0] = GfVec4f(0);
        translate[0] = GfVec3f(3, 0, -2);
        prototypeIndex[0] = 0;

        scale[1] = GfVec3f(1);
        rotate[1] = GfVec4f(0);
        translate[1] = GfVec3f(0, 0, -2);
        prototypeIndex[1] = 0;

        scale[2] = GfVec3f(1);
        rotate[2] = GfVec4f(0);
        translate[2] = GfVec3f(-3, 0, -2);
        prototypeIndex[2] = 0;

        _delegate->SetInstancerProperties(SdfPath("/instancerBottom"),
                                          prototypeIndex,
                                          scale, rotate, translate);
    }

    SetCameraTranslate(GfVec3f(0, 0, -20));

    // XXX: Setup a VAO, the current drawing engine will not yet do this.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindVertexArray(0);
}

void My_TestGLDrawing::UninitTest()
{
    delete _delegate;
    delete _renderIndex;
}

void My_TestGLDrawing::DrawTest(int width, int height)
{
    //
    // Update the draw target's size and execute the unit test with
    // the draw target bound.
    //
    _drawTarget->Bind();
    _drawTarget->SetSize(pxr::GfVec2i(width, height));

    GLfloat clearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clearColor);

    GLfloat clearDepth[1] = {1.0f};
    glClearBufferfv(GL_DEPTH, 0, clearDepth);

    DrawScene(width, height);

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

void My_TestGLDrawing::
    DrawScene(int width, int height, PickParam const *pickParam)
{
    GfMatrix4d viewMatrix = GetViewMatrix();

    GfFrustum frustum = GetFrustum(width, height);
    GfVec4d viewport(0, 0, width, height);

    if (pickParam)
    {
        frustum = frustum.ComputeNarrowedFrustum(
            GfVec2d((2.0 * pickParam->location[0]) / width - 1.0,
                    (2.0 * (height - pickParam->location[1])) / height - 1.0),
            GfVec2d(1.0 / width, 1.0 / height));
        viewport = pickParam->viewport;
    }

    GfMatrix4d projMatrix = frustum.ComputeProjectionMatrix();
    _delegate->SetCamera(viewMatrix, projMatrix);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    HdTaskSharedPtrVector tasks;
    SdfPath renderSetupTask("/renderSetupTask");
    SdfPath renderTask("/renderTask");
    tasks.push_back(_delegate->GetRenderIndex().GetTask(renderSetupTask));
    tasks.push_back(_delegate->GetRenderIndex().GetTask(renderTask));

    HdxRenderTaskParams param = _delegate->GetTaskParam(
                                             renderSetupTask, HdTokens->params)
                                    .Get<HdxRenderTaskParams>();
    param.enableIdRender = (pickParam != nullptr);
    param.viewport = viewport;
    _delegate->SetTaskParam(renderSetupTask, HdTokens->params, VtValue(param));

    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(vao);

    _engine.Execute(&_delegate->GetRenderIndex(), &tasks);

    glBindVertexArray(0);
}

SdfPath
My_TestGLDrawing::PickScene(int pickX, int pickY, int *outInstanceIndex)
{
    const int width = 128;
    const int height = 128;

    GlfDrawTargetRefPtr drawTarget = GlfDrawTarget::New(GfVec2i(width, height));
    drawTarget->Bind();
    drawTarget->AddAttachment(
        "primId", GL_RGBA, GL_UNSIGNED_BYTE, GL_RGBA8);
    drawTarget->AddAttachment(
        "instanceId", GL_RGBA, GL_UNSIGNED_BYTE, GL_RGBA8);
    drawTarget->AddAttachment(
        "depth", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F);
    drawTarget->Unbind();

    drawTarget->Bind();

    GLenum drawBuffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
    };
    glDrawBuffers(2, drawBuffers);

    glEnable(GL_DEPTH_TEST);

    GLfloat clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, clearColor);
    glClearBufferfv(GL_COLOR, 1, clearColor);

    GLfloat clearDepth[1] = {1.0f};
    glClearBufferfv(GL_DEPTH, 0, clearDepth);

    PickParam pickParam;
    pickParam.location = GfVec2d(pickX, pickY);
    pickParam.viewport = GfVec4d(0, 0, width, height);

    DrawScene(width, height, &pickParam);

    drawTarget->Unbind();

    GLubyte primId[width * height * 4];
    glBindTexture(GL_TEXTURE_2D,
                  drawTarget->GetAttachments().at("primId")->GetGlTextureName());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, primId);

    GLubyte instanceId[width * height * 4];
    glBindTexture(GL_TEXTURE_2D,
                  drawTarget->GetAttachments().at("instanceId")->GetGlTextureName());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, instanceId);

    GLfloat depths[width * height];
    glBindTexture(GL_TEXTURE_2D,
                  drawTarget->GetAttachments().at("depth")->GetGlTextureName());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depths);

    double zMin = 1.0;
    int zMinIndex = -1;
    for (int y = 0, i = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++, i++)
        {
            if (depths[i] < zMin)
            {
                zMin = depths[i];
                zMinIndex = i;
            }
        }
    }

    bool didHit = (zMin < 1.0);

    SdfPath result;
    if (didHit)
    {
        int idIndex = zMinIndex * 4;

        result = _delegate->GetRenderIndex().GetRprimPathFromPrimId(
            HdxPickTask::DecodeIDRenderColor(&primId[idIndex]));
        if (outInstanceIndex)
        {
            *outInstanceIndex = HdxPickTask::DecodeIDRenderColor(
                &instanceId[idIndex]);
        }
    }

    return result;
}

void My_TestGLDrawing::MousePress(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = true;
    _mousePos[0] = x;
    _mousePos[1] = y;

    int instanceIndex = 0;
    SdfPath primId = PickScene(x, y, &instanceIndex);

    if (!primId.IsEmpty())
    {
        std::cout << "pick(" << x << ", " << y << "): "
                  << "primId == " << primId << " "
                  << "instance == " << instanceIndex << "\n";
    }
}

void My_TestGLDrawing::ParseArgs(int argc, char *argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if (arg == "--repr")
        {
            _reprName = TfToken(argv[++i]);
        }
        else if (arg == "--refineLevel")
        {
            _refineLevel = atoi(argv[++i]);
        }
    }
}

bool My_TestGLDrawing::WriteToFile(std::string const &attachment,
                                   std::string const &filename) const
{
    // return _widget->WriteToFile(attachment, filename);
    return false;
}

void My_TestGLDrawing::MouseRelease(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = false;
}

void My_TestGLDrawing::MouseMove(int x, int y, int modKeys)
{
    int dx = x - _mousePos[0];
    int dy = y - _mousePos[1];

    if (modKeys & GarchGLDebugWindow::Alt)
    {
        if (_mouseButton[0])
        {
            _rotate[1] += dx;
            _rotate[0] += dy;
        }
        else if (_mouseButton[1])
        {
            _translate[0] += 0.1 * dx;
            _translate[1] -= 0.1 * dy;
        }
        else if (_mouseButton[2])
        {
            _translate[2] += 0.1 * dx;
        }
    }

    _mousePos[0] = x;
    _mousePos[1] = y;
}

void My_TestGLDrawing::KeyRelease(int key)
{
}

GfMatrix4d
My_TestGLDrawing::GetViewMatrix() const
{
    GfMatrix4d viewMatrix;
    viewMatrix.SetIdentity();
    // rotate from z-up to y-up
    viewMatrix *= GfMatrix4d().SetRotate(GfRotation(GfVec3d(1.0, 0.0, 0.0), -90.0));
    viewMatrix *= GfMatrix4d().SetRotate(GfRotation(GfVec3d(0, 1, 0), _rotate[1]));
    viewMatrix *= GfMatrix4d().SetRotate(GfRotation(GfVec3d(1, 0, 0), _rotate[0]));
    viewMatrix *= GfMatrix4d().SetTranslate(GfVec3d(_translate[0], _translate[1], _translate[2]));

    return viewMatrix;
}

GfMatrix4d
My_TestGLDrawing::GetProjectionMatrix(int width, int height) const
{
    return GetFrustum(width, height).ComputeProjectionMatrix();
}

GfFrustum
My_TestGLDrawing::GetFrustum(int width, int height) const
{
    double aspectRatio = double(width) / height;

    GfFrustum frustum;
    frustum.SetPerspective(45.0, aspectRatio, 1, 100000.0);
    return frustum;
}
