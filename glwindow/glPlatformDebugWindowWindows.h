// COPY from pxr/imaging/garch
#pragma once
#include <Windows.h>

class GarchGLDebugWindow;

/// \class Garch_GLPlatformDebugWindow
///
class Garch_GLPlatformDebugWindow
{
public:
    Garch_GLPlatformDebugWindow(GarchGLDebugWindow *w);

    void Init(const char *title, int width, int height, int nSamples=1);
    void Run();
    void ExitApp();

private:
    static Garch_GLPlatformDebugWindow *_GetWindowByHandle(HWND);
    static LRESULT WINAPI _MsgProc(HWND hWnd, UINT msg,
                                   WPARAM wParam, LPARAM lParam);

    bool _running;
    GarchGLDebugWindow *_callback;
    HWND  _hWND;
    HDC   _hDC;
    HGLRC _hGLRC;
    static LPCSTR _className;
};
