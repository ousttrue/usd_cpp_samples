#pragma once

#include <pxr/pxr.h>
PXR_NAMESPACE_OPEN_SCOPE
class HdRenderIndex;
class HdSceneDelegate;
PXR_NAMESPACE_CLOSE_SCOPE

class Drawing
{
    class My_TestGLDrawing *_impl = nullptr;

public:
    Drawing();
    ~Drawing();
    pxr::HdRenderIndex* Init(int width, int height);
    void Uninit();
    void Draw(int width, int height, pxr::HdSceneDelegate *sceneDelegate);
    void KeyRelease(int key);
};
