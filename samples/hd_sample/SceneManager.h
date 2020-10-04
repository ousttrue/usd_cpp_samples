#pragma once
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3f.h"
#include <pxr/imaging/hd/tokens.h>

PXR_NAMESPACE_OPEN_SCOPE
class HdRenderIndex;
class HdSceneDelegate;
class Hdx_UnitTestDelegate;
PXR_NAMESPACE_CLOSE_SCOPE

class SceneManager
{
    pxr::Hdx_UnitTestDelegate *_delegate;
    pxr::TfToken _reprName = pxr::HdReprTokens->hull;

    float _rotate[2] = {0, 0};
    pxr::GfVec3f _translate = pxr::GfVec3f(0, 0, 0);
    int _mousePos[2] = {0, 0};
    bool _mouseButton[3] = {false, false, false};

public:
    SceneManager();
    ~SceneManager();
    void Uninit();
    void CreateDelegate(pxr::HdRenderIndex *renderIndex);
    pxr::HdSceneDelegate *Prepare(int width, int height);
    void MousePress(int button, int x, int y, int modKeys);
    void MouseRelease(int button, int x, int y, int modKeys);
    void MouseMove(int x, int y, int modKeys);
    void SetCameraRotate(float rx, float ry);
    void SetCameraTranslate(pxr::GfVec3f t);
    pxr::GfVec3f GetCameraTranslate() const;
    pxr::GfMatrix4d GetViewMatrix() const;
    pxr::GfMatrix4d GetProjectionMatrix(int width, int height) const;
    pxr::GfFrustum GetFrustum(int width, int height) const;
    pxr::GfVec2i GetMousePos() const { return pxr::GfVec2i(_mousePos[0], _mousePos[1]); };
};
