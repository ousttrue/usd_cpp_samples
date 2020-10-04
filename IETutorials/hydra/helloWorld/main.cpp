
#include "pxr/base/tf/token.h"

#include "pxr/imaging/glf/glew.h"

#include "pxr/imaging/hd/renderIndex.h"
#include "pxr/imaging/hd/renderDelegate.h"
#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hd/task.h"
#include "pxr/imaging/hd/debugCodes.h"
#include "pxr/imaging/hdx/renderTask.h"
#include "pxr/imaging/hd/renderPassState.h"

#include "pxr/imaging/hdSt/renderDelegate.h"
#include "pxr/imaging/hdSt/renderPass.h"

#include "pxr/usd/sdf/path.h"

#include <glWindow.h>

#include "SceneDelegate.h"
#include <iostream>
#include "pxr/imaging/hgi/hgi.h"
#include "pxr/imaging/hgi/tokens.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/glContext.h"
#include "pxr/imaging/glf/diagnostic.h"

class DebugWindow : public GarchGLDebugWindow
{
    pxr::HgiUniquePtr _hgi;
    std::unique_ptr<pxr::HdDriver> _driver;
    pxr::HdEngine _engine;
    pxr::HdStRenderDelegate _renderDelegate;
    pxr::HdRenderIndex *_renderIndex = nullptr;
    GLuint _vao = 0;

    SceneDelegate *_sceneDelegate = nullptr;

public:
    DebugWindow(const char *title, int width, int height) : GarchGLDebugWindow(title, width, height)
    {
    }

    void OnInitializeGL() override
    {
        pxr::GlfGlewInit();
        pxr::GlfRegisterDefaultDebugOutputMessageCallback();
        pxr::GlfContextCaps::InitInstance();
        std::cout << glGetString(GL_VENDOR) << "\n";
        std::cout << glGetString(GL_RENDERER) << "\n";
        std::cout << glGetString(GL_VERSION) << "\n";

        // Hgi and HdDriver should be constructed before HdEngine to ensure they
        // are destructed last. Hgi may be used during engine/delegate destruction.
        _hgi = pxr::Hgi::CreatePlatformDefaultHgi();
        _driver.reset(new pxr::HdDriver{pxr::HgiTokens->renderDriver, pxr::VtValue(_hgi.get())});
        // RenderIndex which stores a flat list of the scene to render
        _renderIndex = pxr::HdRenderIndex::New(&_renderDelegate, {_driver.get()});

        // SceneDelegate can query information from the client SceneGraph to update the renderer
        pxr::SdfPath sceneId("/");
        _sceneDelegate = new SceneDelegate(_renderIndex, sceneId);

        pxr::SdfPath renderSetupTask("/renderSetupTask");
        pxr::SdfPath renderTask("/renderTask");
        _sceneDelegate->AddRenderSetupTask(renderSetupTask);
        _sceneDelegate->AddRenderTask(renderTask);
    }

    void OnPaintGL() override
    {
        GarchGLDebugWindow::OnPaintGL();

        // clear to blue
        glClearColor(0.1f, 0.1f, 0.3f, 1.0);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, GetWidth(), GetHeight());

        // execute the render tasks
        pxr::HdTaskSharedPtrVector tasks;
        pxr::SdfPath renderSetupTask("/renderSetupTask");
        pxr::SdfPath renderTask("/renderTask");
        tasks.push_back(_sceneDelegate->GetRenderIndex().GetTask(renderSetupTask));
        tasks.push_back(_sceneDelegate->GetRenderIndex().GetTask(renderTask));
        _engine.Execute(_renderIndex, &tasks);
    }
};

int main(int argc, char **argv)
{
    if (false)
    {
        pxr::TfDebug::Enable(pxr::HD_MDI);
    }

    // create a window, GLContext & extensions
    DebugWindow window("hydra - Hello World", 1280, 720);
    window.Init();

    // display the window & run the *event* loop until the window is closed
    window.Run();

    return 0;
}
