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
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3f.h"
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include "pxr/imaging/hdSt/renderDelegate.h"
#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hgi/hgi.h"
#include "pxr/imaging/glf/drawTarget.h"

GLuint vao;

static pxr::GfMatrix4d
_GetTranslate(float tx, float ty, float tz)
{
    pxr::GfMatrix4d m(1.0f);
    m.SetRow(3, pxr::GfVec4f(tx, ty, tz, 1.0));
    return m;
}

class My_TestGLDrawing
{
public:
    My_TestGLDrawing()
    {
        SetCameraRotate(0, 0);
        SetCameraTranslate(pxr::GfVec3f(0));
    }

    ~My_TestGLDrawing()
    {
    }

    void InitTest(int width, int height)
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

        _hgi = pxr::Hgi::CreatePlatformDefaultHgi();
        _driver.reset(new pxr::HdDriver{pxr::HgiTokens->renderDriver, pxr::VtValue(_hgi.get())});

        _renderIndex = pxr::HdRenderIndex::New(&_renderDelegate, {_driver.get()});
        // TF_VERIFY(_renderIndex != nullptr);
        _delegate = new pxr::Hdx_UnitTestDelegate(_renderIndex);

        _delegate->SetRefineLevel(_refineLevel);

        // prepare render task
        pxr::SdfPath renderSetupTask("/renderSetupTask");
        pxr::SdfPath renderTask("/renderTask");
        _delegate->AddRenderSetupTask(renderSetupTask);
        _delegate->AddRenderTask(renderTask);

        // render task parameters.
        pxr::HdxRenderTaskParams param = _delegate->GetTaskParam(
                                                      renderSetupTask, pxr::HdTokens->params)
                                             .Get<pxr::HdxRenderTaskParams>();
        param.enableLighting = true; // use default lighting
        _delegate->SetTaskParam(renderSetupTask, pxr::HdTokens->params, pxr::VtValue(param));
        _delegate->SetTaskParam(renderTask, pxr::HdTokens->collection,
                                pxr::VtValue(pxr::HdRprimCollection(pxr::HdTokens->geometry,
                                                                    pxr::HdReprSelector(_reprName))));

        // prepare scene
        // To ensure that the non-aggregated element index returned via picking,
        // we need to have at least two cubes with uniform colors.
        pxr::GfVec4f red(1, 0, 0, 1), green(0, 1, 0, 1), blue(0, 0, 1, 1),
            yellow(1, 1, 0, 1), magenta(1, 0, 1, 1), cyan(0, 1, 1, 1),
            white(1, 1, 1, 1), black(0, 0, 0, 1);

        pxr::GfVec4f faceColors[] = {red, green, blue, yellow, magenta, cyan};
        pxr::VtValue faceColor = pxr::VtValue(_BuildArray(&faceColors[0],
                                                          sizeof(faceColors) / sizeof(faceColors[0])));

        pxr::GfVec4f vertColors[] = {white, blue, green, yellow,
                                     black, blue, magenta, red};
        pxr::VtValue vertColor = pxr::VtValue(_BuildArray(&vertColors[0],
                                                          sizeof(vertColors) / sizeof(vertColors[0])));

        _delegate->AddCube(pxr::SdfPath("/cube0"), _GetTranslate(5, 0, 5),
                           /*guide=*/false, /*instancerId=*/pxr::SdfPath(),
                           /*scheme=*/pxr::PxOsdOpenSubdivTokens->catmullClark,
                           /*color=*/faceColor,
                           /*colorInterpolation=*/pxr::HdInterpolationUniform);
        _delegate->AddCube(pxr::SdfPath("/cube1"), _GetTranslate(-5, 0, 5),
                           /*guide=*/false, /*instancerId=*/pxr::SdfPath(),
                           /*scheme=*/pxr::PxOsdOpenSubdivTokens->catmullClark,
                           /*color=*/faceColor,
                           /*colorInterpolation=*/pxr::HdInterpolationUniform);
        _delegate->AddCube(pxr::SdfPath("/cube2"), _GetTranslate(-5, 0, -5));
        _delegate->AddCube(pxr::SdfPath("/cube3"), _GetTranslate(5, 0, -5),
                           /*guide=*/false, /*instancerId=*/pxr::SdfPath(),
                           /*scheme=*/pxr::PxOsdOpenSubdivTokens->catmullClark,
                           /*color=*/vertColor,
                           /*colorInterpolation=*/pxr::HdInterpolationVertex);

        {
            _delegate->AddInstancer(pxr::SdfPath("/instancerTop"));
            _delegate->AddCube(pxr::SdfPath("/protoTop"),
                               pxr::GfMatrix4d(1), false, pxr::SdfPath("/instancerTop"));

            std::vector<pxr::SdfPath> prototypes;
            prototypes.push_back(pxr::SdfPath("/protoTop"));

            pxr::VtVec3fArray scale(3);
            pxr::VtVec4fArray rotate(3);
            pxr::VtVec3fArray translate(3);
            pxr::VtIntArray prototypeIndex(3);

            scale[0] = pxr::GfVec3f(1);
            rotate[0] = pxr::GfVec4f(0);
            translate[0] = pxr::GfVec3f(3, 0, 2);
            prototypeIndex[0] = 0;

            scale[1] = pxr::GfVec3f(1);
            rotate[1] = pxr::GfVec4f(0);
            translate[1] = pxr::GfVec3f(0, 0, 2);
            prototypeIndex[1] = 0;

            scale[2] = pxr::GfVec3f(1);
            rotate[2] = pxr::GfVec4f(0);
            translate[2] = pxr::GfVec3f(-3, 0, 2);
            prototypeIndex[2] = 0;

            _delegate->SetInstancerProperties(pxr::SdfPath("/instancerTop"),
                                              prototypeIndex,
                                              scale, rotate, translate);
        }

        {
            _delegate->AddInstancer(pxr::SdfPath("/instancerBottom"));
            _delegate->AddCube(pxr::SdfPath("/protoBottom"),
                               pxr::GfMatrix4d(1), false, pxr::SdfPath("/instancerBottom"));

            std::vector<pxr::SdfPath> prototypes;
            prototypes.push_back(pxr::SdfPath("/protoBottom"));

            pxr::VtVec3fArray scale(3);
            pxr::VtVec4fArray rotate(3);
            pxr::VtVec3fArray translate(3);
            pxr::VtIntArray prototypeIndex(3);

            scale[0] = pxr::GfVec3f(1);
            rotate[0] = pxr::GfVec4f(0);
            translate[0] = pxr::GfVec3f(3, 0, -2);
            prototypeIndex[0] = 0;

            scale[1] = pxr::GfVec3f(1);
            rotate[1] = pxr::GfVec4f(0);
            translate[1] = pxr::GfVec3f(0, 0, -2);
            prototypeIndex[1] = 0;

            scale[2] = pxr::GfVec3f(1);
            rotate[2] = pxr::GfVec4f(0);
            translate[2] = pxr::GfVec3f(-3, 0, -2);
            prototypeIndex[2] = 0;

            _delegate->SetInstancerProperties(pxr::SdfPath("/instancerBottom"),
                                              prototypeIndex,
                                              scale, rotate, translate);
        }

        SetCameraTranslate(pxr::GfVec3f(0, 0, -20));

        // XXX: Setup a VAO, the current drawing engine will not yet do this.
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindVertexArray(0);
    }

    void UninitTest()
    {
        delete _delegate;
        delete _renderIndex;
    }

    void DrawTest(int width, int height)
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

    void MousePress(int button, int x, int y, int modKeys)
    {
        _mouseButton[button] = true;
        _mousePos[0] = x;
        _mousePos[1] = y;

        int instanceIndex = 0;
        pxr::SdfPath primId = PickScene(x, y, &instanceIndex);

        if (!primId.IsEmpty())
        {
            std::cout << "pick(" << x << ", " << y << "): "
                      << "primId == " << primId << " "
                      << "instance == " << instanceIndex << "\n";
        }
    }

    void MouseRelease(int button, int x, int y, int modKeys)
    {
        _mouseButton[button] = false;
    }

    void MouseMove(int x, int y, int modKeys)
    {
        int dx = x - _mousePos[0];
        int dy = y - _mousePos[1];

        if (modKeys & pxr::GarchGLDebugWindow::Alt)
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

    void KeyRelease(int key)
    {
    }

private:
    struct PickParam
    {
        pxr::GfVec2d location;
        pxr::GfVec4d viewport;
    };

    void DrawScene(int width, int height, PickParam const *pickParam = nullptr)
    {
        pxr::GfMatrix4d viewMatrix = GetViewMatrix();

        pxr::GfFrustum frustum = GetFrustum(width, height);
        pxr::GfVec4d viewport(0, 0, width, height);

        if (pickParam)
        {
            frustum = frustum.ComputeNarrowedFrustum(
                pxr::GfVec2d((2.0 * pickParam->location[0]) / width - 1.0,
                             (2.0 * (height - pickParam->location[1])) / height - 1.0),
                pxr::GfVec2d(1.0 / width, 1.0 / height));
            viewport = pickParam->viewport;
        }

        pxr::GfMatrix4d projMatrix = frustum.ComputeProjectionMatrix();
        _delegate->SetCamera(viewMatrix, projMatrix);

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        pxr::HdTaskSharedPtrVector tasks;
        pxr::SdfPath renderSetupTask("/renderSetupTask");
        pxr::SdfPath renderTask("/renderTask");
        tasks.push_back(_delegate->GetRenderIndex().GetTask(renderSetupTask));
        tasks.push_back(_delegate->GetRenderIndex().GetTask(renderTask));

        pxr::HdxRenderTaskParams param = _delegate->GetTaskParam(
                                                      renderSetupTask, pxr::HdTokens->params)
                                             .Get<pxr::HdxRenderTaskParams>();
        param.enableIdRender = (pickParam != nullptr);
        param.viewport = viewport;
        _delegate->SetTaskParam(renderSetupTask, pxr::HdTokens->params, pxr::VtValue(param));

        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(vao);

        _engine.Execute(&_delegate->GetRenderIndex(), &tasks);

        glBindVertexArray(0);
    }

    pxr::SdfPath PickScene(int pickX, int pickY, int *outInstanceIndex = nullptr)
    {
        const int width = 128;
        const int height = 128;

        pxr::GlfDrawTargetRefPtr drawTarget = pxr::GlfDrawTarget::New(pxr::GfVec2i(width, height));
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
        pickParam.location = pxr::GfVec2d(pickX, pickY);
        pickParam.viewport = pxr::GfVec4d(0, 0, width, height);

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

        pxr::SdfPath result;
        if (didHit)
        {
            int idIndex = zMinIndex * 4;

            result = _delegate->GetRenderIndex().GetRprimPathFromPrimId(
                pxr::HdxPickTask::DecodeIDRenderColor(&primId[idIndex]));
            if (outInstanceIndex)
            {
                *outInstanceIndex = pxr::HdxPickTask::DecodeIDRenderColor(
                    &instanceId[idIndex]);
            }
        }

        return result;
    }

    void OffscreenTest();
    void RunTest(int argc, char *argv[]);
    void Idle() {}
    bool WriteToFile(std::string const &attachment,
                     std::string const &filename) const
    {
        // return _widget->WriteToFile(attachment, filename);
        return false;
    }

    void SetCameraRotate(float rx, float ry)
    {
        _rotate[0] = rx;
        _rotate[1] = ry;
    }

    void SetCameraTranslate(pxr::GfVec3f t)
    {
        _translate = t;
    }

    pxr::GfVec3f GetCameraTranslate() const
    {
        return _translate;
    }

    pxr::GfMatrix4d GetViewMatrix() const
    {
        pxr::GfMatrix4d viewMatrix;
        viewMatrix.SetIdentity();
        // rotate from z-up to y-up
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1.0, 0.0, 0.0), -90.0));
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(0, 1, 0), _rotate[1]));
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1, 0, 0), _rotate[0]));
        viewMatrix *= pxr::GfMatrix4d().SetTranslate(pxr::GfVec3d(_translate[0], _translate[1], _translate[2]));

        return viewMatrix;
    }

    pxr::GfMatrix4d GetProjectionMatrix(int width, int height) const
    {
        return GetFrustum(width, height).ComputeProjectionMatrix();
    }

    pxr::GfFrustum GetFrustum(int width, int height) const
    {
        double aspectRatio = double(width) / height;

        pxr::GfFrustum frustum;
        frustum.SetPerspective(45.0, aspectRatio, 1, 100000.0);
        return frustum;
    }
    pxr::GfVec2i GetMousePos() const { return pxr::GfVec2i(_mousePos[0], _mousePos[1]); }

private:
    float _rotate[2] = {0, 0};
    pxr::GfVec3f _translate = pxr::GfVec3f(0, 0, 0);
    int _mousePos[2] = {0, 0};
    bool _mouseButton[3] = {false, false, false};

    // Hgi and HdDriver should be constructed before HdEngine to ensure they
    // are destructed last. Hgi may be used during engine/delegate destruction.
    pxr::HgiUniquePtr _hgi;
    std::unique_ptr<pxr::HdDriver> _driver;
    pxr::HdEngine _engine;
    pxr::HdStRenderDelegate _renderDelegate;
    pxr::HdRenderIndex *_renderIndex;
    pxr::Hdx_UnitTestDelegate *_delegate;
    pxr::TfToken _reprName = pxr::HdReprTokens->hull;
    int _refineLevel = 0;
    pxr::GlfDrawTargetRefPtr _drawTarget;
};

///

Drawing::Drawing()
    : _impl(new My_TestGLDrawing)
{
}

Drawing::~Drawing()
{
    Uninit();
}

void Drawing::Init(int width, int height)
{
    _impl->InitTest(width, height);
}

void Drawing::Uninit()
{
    if (_impl)
    {
        delete _impl;
        _impl = nullptr;
    }
}

void Drawing::Draw(int width, int height)
{
    if (_impl)
        _impl->DrawTest(width, height);
}

void Drawing::MousePress(int button, int x, int y, int modKeys)
{
    if (_impl)
        _impl->MousePress(button, x, y, modKeys);
}

void Drawing::MouseRelease(int button, int x, int y, int modKeys)
{
    if (_impl)
        _impl->MouseRelease(button, x, y, modKeys);
}

void Drawing::MouseMove(int x, int y, int modKeys)
{
    if (_impl)
        _impl->MouseMove(x, y, modKeys);
}

void Drawing::KeyRelease(int key)
{
    if (_impl)
        _impl->KeyRelease(key);
}
