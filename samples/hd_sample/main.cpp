#include "testHdxRenderer.h"
#include <pxr/imaging/garch/glDebugWindow.h>
#include <pxr/base/tf/errorMark.h>
#include <iostream>
#include <functional>

struct Callback
{
    std::function<void(int width, int height)> OnInitializeGL = [](int, int) {};
    std::function<void()> OnUninitializeGL = []() {};
    std::function<void(int width, int height)> OnPaintGL = [](int, int) {};
    std::function<void(int key)> OnKeyRelease = [](int) {};
    std::function<void(int button, int x, int y, int modKeys)> OnMousePress = [](int, int, int, int) {};
    std::function<void(int button, int x, int y, int modKeys)> OnMouseRelease = [](int, int, int, int) {};
    std::function<void(int x, int y, int modKeys)> OnMouseMove = [](int, int, int) {};
};

class HdSt_UnitTestWindow : public pxr::GarchGLDebugWindow
{
    Callback _callback;

public:
    HdSt_UnitTestWindow(int w, int h, const Callback &callback)
        : pxr::GarchGLDebugWindow("Hd Test", w, h), _callback(callback)
    {
    }

    ~HdSt_UnitTestWindow()
    {
    }

    void OnInitializeGL() override
    {
        _callback.OnInitializeGL(GetWidth(), GetHeight());
    }

    void OnUninitializeGL() override
    {
        _callback.OnUninitializeGL();
    }

    void OnPaintGL() override
    {
        _callback.OnPaintGL(GetWidth(), GetHeight());
    }

    void OnKeyRelease(int key) override
    {
        switch (key)
        {
        case 'q':
            ExitApp();
            return;
        }
        _callback.OnKeyRelease(key);
    }

    void OnMousePress(int button, int x, int y, int modKeys) override
    {
        _callback.OnMousePress(button, x, y, modKeys);
    }

    void OnMouseRelease(int button, int x, int y, int modKeys) override
    {
        _callback.OnMouseRelease(button, x, y, modKeys);
    }

    void OnMouseMove(int x, int y, int modKeys) override
    {
        _callback.OnMouseMove(x, y, modKeys);
    }
};

int main(int argc, char *argv[])
{
    pxr::TfErrorMark mark;

    {
        Drawing drawing;

        Callback callback;
        callback.OnInitializeGL = [&drawing](int width, int height) {
            drawing.Init(width, height);
        };
        callback.OnUninitializeGL = [&drawing]() {
            drawing.Uninit();
        };
        callback.OnPaintGL = [&drawing](int width, int height) {
            drawing.Draw(width, height);
        };
        callback.OnMousePress = [&drawing](int b, int x, int y, int m) { drawing.MousePress(b, x, y, m); };
        callback.OnMouseRelease = [&drawing](int b, int x, int y, int m) { drawing.MouseRelease(b, x, y, m); };
        callback.OnMouseMove = [&drawing](int x, int y, int m) { drawing.MouseMove(x, y, m); };
        callback.OnKeyRelease = [&drawing](int k) { drawing.KeyRelease(k); };

        HdSt_UnitTestWindow window(640, 480, callback);
        window.Init();
        window.Run();
    }

    if (mark.IsClean())
    {
        std::cout << "OK" << std::endl;
        return EXIT_SUCCESS;
    }
    else
    {
        std::cout << "FAILED" << std::endl;
        return EXIT_FAILURE;
    }
}
