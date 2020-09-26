#include "pxr/imaging/glf/glew.h"
#include "UnitTestWindow.h"
#include "unitTestGLDrawing.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/diagnostic.h"
#include "pxr/imaging/glf/drawTarget.h"
#include "pxr/imaging/garch/glDebugWindow.h"

UsdImagingGL_UnitTestWindow::UsdImagingGL_UnitTestWindow(
    pxr::UsdImagingGL_UnitTestGLDrawing *unitTest, int w, int h)
    : GarchGLDebugWindow("UsdImagingGL Test", w, h), _unitTest(unitTest)
{
}

UsdImagingGL_UnitTestWindow::~UsdImagingGL_UnitTestWindow()
{
}

/* virtual */
void UsdImagingGL_UnitTestWindow::OnInitializeGL()
{
    pxr::GlfGlewInit();
    pxr::GlfRegisterDefaultDebugOutputMessageCallback();
    pxr::GlfContextCaps::InitInstance();

    //
    // Create an offscreen draw target which is the same size as this
    // widget and initialize the unit test with the draw target bound.
    //
    _drawTarget = pxr::GlfDrawTarget::New(pxr::GfVec2i(GetWidth(), GetHeight()));
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
    _drawTarget = pxr::GlfDrawTargetRefPtr();

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
    _drawTarget->SetSize(pxr::GfVec2i(width, height));

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
    _drawTarget->SetSize(pxr::GfVec2i(GetWidth(), GetHeight()));

    _unitTest->DrawTest(true);

    _drawTarget->Unbind();
}

void UsdImagingGL_UnitTestWindow::Clear(const pxr::GfVec4f &fboClearColor, float clearDepth)
{
    glClearBufferfv(GL_COLOR, 0, fboClearColor.data());
    glClearBufferfv(GL_DEPTH, 0, &clearDepth);
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
