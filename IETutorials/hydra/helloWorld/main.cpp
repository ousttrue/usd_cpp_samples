
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

class DebugWindow : public GarchGLDebugWindow
{
    pxr::HgiUniquePtr _hgi;
    pxr::HdDriver _driver;

    std::shared_ptr<pxr::HdStRenderDelegate> _renderDelegate;
    pxr::HdRenderIndex *_index = nullptr;
    SceneDelegate *_sceneDelegate;
    pxr::HdEngine _engine;
    pxr::HdTaskSharedPtrVector _tasks;

public:
    DebugWindow(const char *title, int width, int height) : GarchGLDebugWindow(title, width, height)
    {
        // create a RenderDelegate which is required for the RenderIndex
        _renderDelegate.reset(new pxr::HdStRenderDelegate());

        // Hgi and HdDriver should be constructed before HdEngine to ensure they
        // are destructed last. Hgi may be used during engine/delegate destruction.
        _hgi = pxr::Hgi::CreatePlatformDefaultHgi();
        _driver = {pxr::HgiTokens->renderDriver, pxr::VtValue(_hgi.get())};

        // RenderIndex which stores a flat list of the scene to render
        _index = pxr::HdRenderIndex::New(_renderDelegate.get(), {&_driver});

        // SceneDelegate can query information from the client SceneGraph to update the renderer
        pxr::SdfPath sceneId("/");
        _sceneDelegate = new SceneDelegate(_index, sceneId);

        {
            pxr::SdfPath renderSetupId("/renderSetup");
            _sceneDelegate->AddRenderSetupTask(renderSetupId);
            _tasks.push_back(_index->GetTask(renderSetupId));
        }

        {
            pxr::SdfPath renderId("/render");
            _sceneDelegate->AddRenderTask(renderId);
            _tasks.push_back(_index->GetTask(renderId));
        }
    }

    void OnInitializeGL() override
    {
        pxr::GlfGlewInit();
        // Test uses ContextCaps, so need to create a GL instance.
        // pxr::GlfTestGLContext::RegisterGLContextCallbacks();
        static pxr::GlfSharedGLContextScopeHolder sharedContext;
        pxr::GlfContextCaps::InitInstance();
    }

    void OnPaintGL() override
    {
        GarchGLDebugWindow::OnPaintGL();

        // clear to blue
        glClearColor(0.1f, 0.1f, 0.3f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);

        // execute the render tasks
        _engine.Execute(_index, &_tasks);
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
