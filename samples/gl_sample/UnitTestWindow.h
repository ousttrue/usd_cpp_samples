#pragma once

#include <pxr/imaging/garch/glDebugWindow.h>
#include <pxr/imaging/glf/drawTarget.h>

PXR_NAMESPACE_OPEN_SCOPE

class UsdImagingGL_UnitTestGLDrawing;

PXR_NAMESPACE_CLOSE_SCOPE

class UsdImagingGL_UnitTestWindow : public pxr::GarchGLDebugWindow
{
public:
    typedef UsdImagingGL_UnitTestWindow This;

public:
    UsdImagingGL_UnitTestWindow(pxr::UsdImagingGL_UnitTestGLDrawing *unitTest,
                                int w, int h);
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
    pxr::UsdImagingGL_UnitTestGLDrawing *_unitTest;
    pxr::GlfDrawTargetRefPtr _drawTarget;
};
