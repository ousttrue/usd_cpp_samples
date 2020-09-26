#pragma once

#include <pxr/imaging/garch/glDebugWindow.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <functional>

class UsdImagingGL_UnitTestWindow : public pxr::GarchGLDebugWindow
{
public:
    typedef UsdImagingGL_UnitTestWindow This;

    using OnInitFunc = std::function<void()>;
    using OnDrawFunc = std::function<void(bool, int, int)>;

public:
    UsdImagingGL_UnitTestWindow(int w, int h, const OnInitFunc &onInit, const OnDrawFunc &onDraw);
    virtual ~UsdImagingGL_UnitTestWindow();

    void DrawOffscreen();
    void Clear(const pxr::GfVec4f &fboClearColor, float clearDepth);

    bool WriteToFile(std::string const &attachment,
                     std::string const &filename);

    // GarchGLDebugWIndow overrides;
    virtual void OnInitializeGL();
    virtual void OnUninitializeGL();
    virtual void OnPaintGL();
    virtual void OnKeyRelease(int key);
    virtual void OnMousePress(int button, int x, int y, int modKeys);
    virtual void OnMouseRelease(int button, int x, int y, int modKeys);
    virtual void OnMouseMove(int x, int y, int modKeys);

private:
    pxr::GlfDrawTargetRefPtr _drawTarget;
    OnInitFunc _onInit;
    OnDrawFunc _onDraw;
};
