// COPY from pxr/imaging/garch
#include "glPlatformDebugWindowWindows.h"
#include "glWindow.h"
#include <map>
#include <iostream>
#include <GL/gl.h>
#include <tchar.h>
#include <GL/wglext.h>
#include <assert.h>

#define TF_FATAL_ERROR(msg)              \
    {                                    \
        std::cerr << (msg) << std::endl; \
    }

namespace
{

    static std::map<HWND, Garch_GLPlatformDebugWindow *> &
    _GetWindowsMap()
    {
        static std::map<HWND, Garch_GLPlatformDebugWindow *> windows;
        return windows;
    }

} // namespace

// ---------------------------------------------------------------------------

LPCSTR Garch_GLPlatformDebugWindow::_className = "GarchGLDebugWindow";

Garch_GLPlatformDebugWindow::Garch_GLPlatformDebugWindow(GarchGLDebugWindow *w)
    : _running(false), _callback(w), _hWND(NULL), _hDC(NULL), _hGLRC(NULL)
{
}

void Garch_GLPlatformDebugWindow::Init(const char *title,
                                       int width, int height, int nSamples)
{
    // platform initialize
    WNDCLASSA wc;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (GetClassInfoA(hInstance, _className, &wc) == 0)
    {
        ZeroMemory(&wc, sizeof(WNDCLASSA));

        wc.lpfnWndProc = &Garch_GLPlatformDebugWindow::_MsgProc;
        wc.hInstance = hInstance;
        wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = _className;

        if (RegisterClassA(&wc) == 0)
        {
            TF_FATAL_ERROR("RegisterClass failed");
            exit(1);
        }
    }

    // XXX: todo: add support multi sampling

    DWORD flags = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    DWORD exFlags = 0;

    _hWND = CreateWindowExA(exFlags, _className,
                            title, flags, 100, 100, width, height,
                            (HWND)NULL, (HMENU)NULL, hInstance,
                            (LPVOID)NULL);
    if (_hWND == 0)
    {
        TF_FATAL_ERROR("CreateWindowEx failed");
        exit(1);
    }

    ShowWindow(_hWND, SW_SHOW);
    _GetWindowsMap()[_hWND] = this;
    _hDC = GetDC(_hWND);

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    int pixelformat = ChoosePixelFormat(_hDC, &pfd);

    if (SetPixelFormat(_hDC, pixelformat, &pfd) == 0)
    {
        TF_FATAL_ERROR("SetPixelFormat failed");
        exit(1);
    }

    _hGLRC = wglCreateContext(_hDC);
    if (_hGLRC == 0)
    {
        TF_FATAL_ERROR("wglCreateContext failed");
        exit(1);
    }
    wglMakeCurrent(_hDC, _hGLRC);

    //
    // upgrade WGLContext
    //
    auto wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    assert(wglCreateContextAttribsARB);

    // 使用する OpenGL のバージョンとプロファイルの指定
    static const int att[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        4,
        WGL_CONTEXT_MINOR_VERSION_ARB,
        6,
        WGL_CONTEXT_FLAGS_ARB,
        0,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    // 新しい HGLRC の作成
    {
        HGLRC hglrc = wglCreateContextAttribsARB(_hDC, NULL, att);
        wglMakeCurrent(_hDC, hglrc);

        // 古い HGLRC の削除と置き換え
        wglDeleteContext(_hGLRC);
        _hGLRC = hglrc;
    }

    _callback->OnInitializeGL();
}

static int
Garch_GetModifierKeys(WPARAM wParam)
{
    int keys = 0;
    if (wParam & MK_SHIFT)
    {
        keys |= GarchGLDebugWindow::Shift;
    }
    if (wParam & MK_CONTROL)
    {
        keys |= GarchGLDebugWindow::Ctrl;
    }
    if (HIBYTE(GetKeyState(VK_MENU)) & 0x80)
    {
        keys |= GarchGLDebugWindow::Alt;
    }
    return keys;
}

/* static */
Garch_GLPlatformDebugWindow *
Garch_GLPlatformDebugWindow::_GetWindowByHandle(HWND hWND)
{
    const auto &windows = _GetWindowsMap();
    auto it = windows.find(hWND);
    if (it != windows.end())
    {
        return it->second;
    }
    return NULL;
}

/* static */
LRESULT WINAPI
Garch_GLPlatformDebugWindow::_MsgProc(HWND hWnd, UINT msg,
                                      WPARAM wParam, LPARAM lParam)
{
    Garch_GLPlatformDebugWindow *window = Garch_GLPlatformDebugWindow::_GetWindowByHandle(hWnd);

    if (!window)
    {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    int x = LOWORD(lParam);
    int y = HIWORD(lParam);
    switch (msg)
    {
    case WM_SIZE:
        window->_callback->OnResize(
            HIWORD(lParam), LOWORD(lParam));
        break;
    case WM_LBUTTONDOWN:
        window->_callback->OnMousePress(
            /*button=*/0, x, y, Garch_GetModifierKeys(wParam));
        break;
    case WM_MBUTTONDOWN:
        window->_callback->OnMousePress(
            /*button=*/1, x, y, Garch_GetModifierKeys(wParam));
        break;
    case WM_RBUTTONDOWN:
        window->_callback->OnMousePress(
            /*button=*/2, x, y, Garch_GetModifierKeys(wParam));
        break;
    case WM_LBUTTONUP:
        window->_callback->OnMouseRelease(
            /*button=*/0, x, y, Garch_GetModifierKeys(wParam));
        break;
    case WM_MBUTTONUP:
        window->_callback->OnMouseRelease(
            /*button=*/1, x, y, Garch_GetModifierKeys(wParam));
        break;
    case WM_RBUTTONUP:
        window->_callback->OnMouseRelease(
            /*button=*/2, x, y, Garch_GetModifierKeys(wParam));
        break;
    case WM_MOUSEMOVE:
        window->_callback->OnMouseMove(
            x, y, Garch_GetModifierKeys(wParam));
        break;
    case WM_KEYUP:
        // We could try to do our own key translation here but for
        // now we handle WM_CHAR.
        //window->_callback->OnKeyRelease(XXX);
        break;
    case WM_CHAR:
        // Note -- this is send on key down, not up.
        window->_callback->OnKeyRelease(wParam);
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Garch_GLPlatformDebugWindow::Run()
{
    if (!_hWND)
    {
        return;
    }

    _running = true;

    MSG msg = {0};
    while (_running && msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // make current
            wglMakeCurrent(_hDC, _hGLRC);

            // XXX: this should be constant interval
            _callback->OnIdle();
            _callback->OnPaintGL();

            glFinish();

            SwapBuffers(_hDC);
        }
    }
    _callback->OnUninitializeGL();

    wglMakeCurrent(NULL, NULL);
    // release GL
    wglDeleteContext(_hGLRC);
    ReleaseDC(_hWND, _hDC);

    _GetWindowsMap().erase(_hWND);
    _hWND = 0;
}

void Garch_GLPlatformDebugWindow::ExitApp()
{
    _running = false;
}
