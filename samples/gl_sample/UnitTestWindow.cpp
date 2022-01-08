#include "UnitTestWindow.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/diagnostic.h"

UnitTestWindow::UnitTestWindow(int w, int h,
                               const OnInitFunc &onInit, const OnDrawFunc &onDraw, const OnUninitFunc &onUninit,
                               const Input &input)
    : GarchGLDebugWindow("UsdImagingGL Test", w, h),
      _onInit(onInit), _onDraw(onDraw), _onUninit(onUninit), _input(input)
{
}

UnitTestWindow::~UnitTestWindow()
{
}

/* virtual */
void UnitTestWindow::OnInitializeGL()
{
    // pxr::GlfGlewInit();
    pxr::GlfRegisterDefaultDebugOutputMessageCallback();
    pxr::GlfContextCaps::InitInstance();

    _onInit(GetWidth(), GetHeight());
}

/* virtual */
void UnitTestWindow::OnUninitializeGL()
{
    _onUninit();
}

/* virtual */
void UnitTestWindow::OnPaintGL()
{
    //
    // Update the draw target's size and execute the unit test with
    // the draw target bound.
    //
    int width = GetWidth();
    int height = GetHeight();

    auto fbo = _onDraw(width, height);

    //
    // Blit the resulting color buffer to the window (this is a noop
    // if we're drawing offscreen).
    //
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

    glBlitFramebuffer(0, 0, width, height,
                      0, 0, width, height,
                      GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

/* virtual */
void UnitTestWindow::OnKeyRelease(int key)
{
    switch (key)
    {
    case 'q':
        ExitApp();
        return;
    }
    _input.KeyRelease(key);
}

/* virtual */
void UnitTestWindow::OnMousePress(int button,
                                  int x, int y, int modKeys)
{
    _input.MousePress(button, x, y, modKeys);
}

/* virtual */
void UnitTestWindow::OnMouseRelease(int button,
                                    int x, int y, int modKeys)
{
    _input.MouseRelease(button, x, y, modKeys);
}

/* virtual */
void UnitTestWindow::OnMouseMove(int x, int y, int modKeys)
{
    _input.MouseMove(x, y, modKeys);
}
