// COPY from pxr/imaging/garch
#include "glWindow.h"
#include "glPlatformDebugWindowWindows.h"

GarchGLDebugWindow::GarchGLDebugWindow(const char *title, int width, int height)
    : _title(title)
    , _width(width)
    , _height(height)
{
    _private = new Garch_GLPlatformDebugWindow(this);
}

GarchGLDebugWindow::~GarchGLDebugWindow()
{
    delete _private;
}

void
GarchGLDebugWindow::Init()
{
    _private->Init(_title.c_str(), _width, _height);
}

void
GarchGLDebugWindow::Run()
{
    _private->Run();
}

void
GarchGLDebugWindow::ExitApp()
{
    _private->ExitApp();
}

/* virtual */
void
GarchGLDebugWindow::OnInitializeGL()
{
}

/* virtual */
void
GarchGLDebugWindow::OnUninitializeGL()
{
}

/* virtual */
void
GarchGLDebugWindow::OnResize(int w, int h)
{
    _width = w;
    _height = h;
}

/* virtual */
void
GarchGLDebugWindow::OnIdle()
{
}

/* virtual */
void
GarchGLDebugWindow::OnPaintGL()
{
}

/* virtual */
void
GarchGLDebugWindow::OnKeyRelease(int key)
{
}

/* virtual */
void
GarchGLDebugWindow::OnMousePress(int button, int x, int y, int modKeys)
{
}

/* virtual */
void
GarchGLDebugWindow::OnMouseRelease(int button, int x, int y, int modKeys)
{
}

/* virtual */
void
GarchGLDebugWindow::OnMouseMove(int x, int y, int modKeys)
{
}
