#pragma once
#include "unitTestGLDrawing.h"
#include <pxr/usd/usd/stage.h>

class My_TestGLDrawing : public pxr::UsdImagingGL_UnitTestGLDrawing
{
public:
    // UsdImagingGL_UnitTestGLDrawing overrides
    virtual void InitTest();
    virtual void DrawTest(bool offscreen);
    virtual void ShutdownTest();

    virtual void MousePress(int button, int x, int y, int modKeys);
    virtual void MouseRelease(int button, int x, int y, int modKeys);
    virtual void MouseMove(int x, int y, int modKeys);

private:
    pxr::UsdStageRefPtr _stage;
    std::shared_ptr<pxr::UsdImagingGLEngine> _engine;
    pxr::GlfSimpleLightingContextRefPtr _lightingContext;

    float _rotate[2] = {0, 0};
    float _translate[3] = {0, 0, 0};
    int _mousePos[2] = {0, 0};
    bool _mouseButton[3] = {false, false, false};
};
