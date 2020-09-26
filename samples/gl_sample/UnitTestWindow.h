#pragma once

#include <pxr/imaging/garch/glDebugWindow.h>
#include <functional>

struct Input
{
    std::function<void(int key)> OnKeyRelease;
    std::function<void(int button, int x, int y, int modKeys)> OnMousePress;
    std::function<void(int button, int x, int y, int modKeys)> OnMouseRelease;
    std::function<void(int x, int y, int modKeys)> OnMouseMove;

    void KeyRelease(int key)
    {
        if (OnKeyRelease)
        {
            OnKeyRelease(key);
        }
    }
    void MousePress(int button, int x, int y, int modKeys)
    {
        if (OnMouseRelease)
        {
            OnMousePress(button, x, y, modKeys);
        }
    }
    void MouseRelease(int button, int x, int y, int modKeys)
    {
        if (OnMouseRelease)
        {
            OnMouseRelease(button, x, y, modKeys);
        }
    }
    void MouseMove(int x, int y, int modKeys)
    {
        if (OnMouseMove)
        {
            OnMouseMove(x, y, modKeys);
        }
    }
};

class UnitTestWindow : public pxr::GarchGLDebugWindow
{
public:
    using OnInitFunc = std::function<void(int, int)>;
    using OnDrawFunc = std::function<uint32_t(bool, int, int)>;
    using OnUninitFunc = std::function<void()>;

public:
    UnitTestWindow(int w, int h,
                   const OnInitFunc &onInit, const OnDrawFunc &onDraw, const OnUninitFunc &onUninit, const Input &input);
    virtual ~UnitTestWindow();

    // GarchGLDebugWIndow overrides;
    virtual void OnInitializeGL();
    virtual void OnUninitializeGL();
    virtual void OnPaintGL();
    virtual void OnKeyRelease(int key);
    virtual void OnMousePress(int button, int x, int y, int modKeys);
    virtual void OnMouseRelease(int button, int x, int y, int modKeys);
    virtual void OnMouseMove(int x, int y, int modKeys);

private:
    OnInitFunc _onInit;
    OnDrawFunc _onDraw;
    OnUninitFunc _onUninit;
    Input _input;
};
