// COPY from pxr/imaging/garch

#pragma once
#include <string>

class Garch_GLPlatformDebugWindow;

/// \class GarchGLDebugWindow
///
/// Platform specific minimum GL widget for unit tests.
///
class GarchGLDebugWindow {
public:
    
    GarchGLDebugWindow(const char *title, int width, int height);
    
    virtual ~GarchGLDebugWindow();

    
    void Init();
    
    void Run();
    
    void ExitApp();

    int GetWidth() const { return _width; }
    int GetHeight() const { return _height; }

    enum Buttons {
        MyButton1 = 0,
        MyButton2 = 1,
        MyButton3 = 2
    };
    enum ModifierKeys {
        NoModifiers = 0,
        Shift = 1,
        Alt   = 2,
        Ctrl  = 4
    };

    
    virtual void OnInitializeGL();
    
    virtual void OnUninitializeGL();
    
    virtual void OnResize(int w, int h);
    
    virtual void OnIdle();
    
    virtual void OnPaintGL();
    
    virtual void OnKeyRelease(int key);
    
    virtual void OnMousePress(int button, int x, int y, int modKeys);
    
    virtual void OnMouseRelease(int button, int x, int y, int modKeys);
    
    virtual void OnMouseMove(int x, int y, int modKeys);

private:
    Garch_GLPlatformDebugWindow *_private;
    std::string _title;
    int _width, _height;
};
