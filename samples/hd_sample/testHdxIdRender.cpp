#include "pxr/imaging/glf/glew.h"
#include "testHdxRenderer.h"
#include "pxr/imaging/hgi/tokens.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/rotation.h"
#include "pxr/base/gf/vec2i.h"
#include "pxr/base/gf/vec4d.h"
#include <pxr/imaging/garch/glDebugWindow.h>
#include <iostream>
#include <memory>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include "pxr/imaging/hdSt/renderDelegate.h"
#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hgi/hgi.h"
#include "pxr/imaging/glf/drawTarget.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/diagnostic.h"

GLuint vao;

class My_TestGLDrawing
{
    // Hgi and HdDriver should be constructed before HdEngine to ensure they
    // are destructed last. Hgi may be used during engine/delegate destruction.
    pxr::HgiUniquePtr _hgi;
    std::unique_ptr<pxr::HdDriver> _driver;
    pxr::HdEngine _engine;
    pxr::HdStRenderDelegate _renderDelegate;
    pxr::HdRenderIndex *_renderIndex;
    pxr::GlfDrawTargetRefPtr _drawTarget;

public:
    My_TestGLDrawing()
    {
    }

    ~My_TestGLDrawing()
    {
    }

    pxr::HdRenderIndex *Init(int width, int height)
    {
        pxr::GlfGlewInit();
        pxr::GlfRegisterDefaultDebugOutputMessageCallback();
        pxr::GlfContextCaps::InitInstance();

        std::cout << glGetString(GL_VENDOR) << "\n";
        std::cout << glGetString(GL_RENDERER) << "\n";
        std::cout << glGetString(GL_VERSION) << "\n";

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

        // XXX: Setup a VAO, the current drawing engine will not yet do this.
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindVertexArray(0);

        return _renderIndex;
    }

    void Uninit()
    {
        delete _renderIndex;
    }

    void Draw(int width, int height, pxr::HdSceneDelegate *sceneDelegate)
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

        DrawScene(width, height, sceneDelegate);

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

    void KeyRelease(int key)
    {
    }

private:
    void DrawScene(int width, int height, pxr::HdSceneDelegate *sceneDelegate)
    {
        glViewport(0, 0, width, height);

        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(vao);

        pxr::HdTaskSharedPtrVector tasks;
        pxr::SdfPath renderSetupTask("/renderSetupTask");
        pxr::SdfPath renderTask("/renderTask");
        tasks.push_back(sceneDelegate->GetRenderIndex().GetTask(renderSetupTask));
        tasks.push_back(sceneDelegate->GetRenderIndex().GetTask(renderTask));
        _engine.Execute(&sceneDelegate->GetRenderIndex(), &tasks);

        glBindVertexArray(0);
    }
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

pxr::HdRenderIndex *Drawing::Init(int width, int height)
{
    return _impl->Init(width, height);
}

void Drawing::Uninit()
{
    if (_impl)
    {
        delete _impl;
        _impl = nullptr;
    }
}

void Drawing::Draw(int width, int height, pxr::HdSceneDelegate *sceneDelegate)
{
    if (_impl)
        _impl->Draw(width, height, sceneDelegate);
}

void Drawing::KeyRelease(int key)
{
    if (_impl)
        _impl->KeyRelease(key);
}
