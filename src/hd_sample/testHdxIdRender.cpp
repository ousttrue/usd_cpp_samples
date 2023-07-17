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

class My_TestGLDrawing
{
    // Hgi and HdDriver should be constructed before HdEngine to ensure they
    // are destructed last. Hgi may be used during engine/delegate destruction.
    pxr::HgiUniquePtr _hgi;
    std::unique_ptr<pxr::HdDriver> _driver;
    pxr::HdEngine _engine;
    pxr::HdStRenderDelegate _renderDelegate;
    pxr::HdRenderIndex *_renderIndex;
    GLuint _vao = 0;

public:
    My_TestGLDrawing()
    {
    }

    ~My_TestGLDrawing()
    {
    }

    pxr::HdRenderIndex *Init(int width, int height)
    {
        pxr::GlfRegisterDefaultDebugOutputMessageCallback();
        pxr::GlfContextCaps::InitInstance();
        std::cout << glGetString(GL_VENDOR) << "\n";
        std::cout << glGetString(GL_RENDERER) << "\n";
        std::cout << glGetString(GL_VERSION) << "\n";

        _hgi = pxr::Hgi::CreatePlatformDefaultHgi();
        _driver.reset(new pxr::HdDriver{pxr::HgiTokens->renderDriver, pxr::VtValue(_hgi.get())});
        _renderIndex = pxr::HdRenderIndex::New(&_renderDelegate, {_driver.get()});
        // TF_VERIFY(_renderIndex != nullptr);

        // XXX: Setup a VAO, the current drawing engine will not yet do this.
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        glBindVertexArray(0);

        return _renderIndex;
    }

    void Uninit()
    {
        delete _renderIndex;
    }

    void Draw(int width, int height, pxr::HdSceneDelegate *sceneDelegate)
    {
        GLfloat clearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, clearColor);
        GLfloat clearDepth[1] = {1.0f};
        glClearBufferfv(GL_DEPTH, 0, clearDepth);

        DrawScene(width, height, sceneDelegate);
    }

    void KeyRelease(int key)
    {
    }

private:
    void DrawScene(int width, int height, pxr::HdSceneDelegate *sceneDelegate)
    {
        glViewport(0, 0, width, height);

        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(_vao);

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
