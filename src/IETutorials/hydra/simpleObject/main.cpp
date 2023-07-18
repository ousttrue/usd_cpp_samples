
#include "pxr/base/tf/token.h"
#include "pxr/imaging/garch/gl.h"
#include "pxr/imaging/hgi/hgi.h"
#include "pxr/imaging/hgi/tokens.h"
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


#include "pxr/imaging/garch/glDebugWindow.h"

#include "SceneDelegate.h"


class DebugWindow : public pxr::GarchGLDebugWindow
{
	pxr::HgiUniquePtr hgi;
	std::unique_ptr<pxr::HdDriver> driver;

public:
	DebugWindow(const char *title, int width, int height) : GarchGLDebugWindow(title, width, height)
	{
		// create a RenderDelegate which is required for the RenderIndex
		renderDelegate.reset( new pxr::HdStRenderDelegate() );

		// Hgi and HdDriver should be constructed before HdEngine to ensure they
		// are destructed last. Hgi may be used during engine/delegate destruction.
		hgi = pxr::Hgi::CreatePlatformDefaultHgi();
		driver.reset(new pxr::HdDriver{pxr::HgiTokens->renderDriver, pxr::VtValue(hgi.get())});
		// RenderIndex which stores a flat list of the scene to render
		index = pxr::HdRenderIndex::New(renderDelegate.get(), {driver.get()});		

		// names for elements in the RenderIndex
		pxr::SdfPath renderSetupId("/renderSetup");
		pxr::SdfPath renderId("/render");
		pxr::SdfPath sceneId("/");

		// SceneDelegate can query information from the client SceneGraph to update the renderer
		sceneDelegate = new SceneDelegate( index, sceneId );

		// Create two tasks (render setup & render) to the RenderIndex
		sceneDelegate->AddRenderSetupTask(renderSetupId);
		sceneDelegate->AddRenderTask(renderId);

		// I can move these into return values for SceneDelegate::AddRenderSetupTask && AddRenderTask functions 
		pxr::HdTaskSharedPtr renderSetupTask = index->GetTask(renderSetupId);
		pxr::HdTaskSharedPtr renderTask      = index->GetTask(renderId);
	
		tasks = {renderSetupTask, renderTask};
	}

	void OnPaintGL() override
	{
		pxr::GarchGLDebugWindow::OnPaintGL();

		// clear to blue
		glClearColor(0.1f, 0.1f, 0.3f, 1.0 );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);

		// execute the render tasks
		engine.Execute( index, &tasks );
	}

private:
	SceneDelegate *sceneDelegate;
	pxr::HdTaskSharedPtrVector tasks;
	pxr::HdEngine engine;
	pxr::HdRenderIndex *index;
	std::shared_ptr<pxr::HdStRenderDelegate> renderDelegate;
};

int main(int argc, char** argv)
{
	if (false)
	{
		pxr::TfDebug::Enable(pxr::HD_MDI);
	}

	// create a window, GLContext & extensions
	DebugWindow window("hydra - simple Object", 1280, 720);
	window.Init();

	// display the window & run the *event* loop until the window is closed
	window.Run();

	return 0;
}