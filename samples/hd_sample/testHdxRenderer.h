#pragma once
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3f.h"
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include "pxr/imaging/hdSt/renderDelegate.h"
#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hgi/hgi.h"
#include "pxr/imaging/glf/drawTarget.h"

PXR_NAMESPACE_OPEN_SCOPE
class Hdx_UnitTestDelegate;
PXR_NAMESPACE_CLOSE_SCOPE

class My_TestGLDrawing
{
public:
    My_TestGLDrawing();

    struct PickParam
    {
        pxr::GfVec2d location;
        pxr::GfVec4d viewport;
    };

    void DrawScene(int width, int height, PickParam const *pickParam = nullptr);
    pxr::SdfPath PickScene(int pickX, int pickY, int *outInstanceIndex = nullptr);
    void InitTest(int width, int height);
    void UninitTest();
    void DrawTest(int width, int height);
    void OffscreenTest();
    void MousePress(int button, int x, int y, int modKeys);
    void RunTest(int argc, char *argv[]);
    void MouseRelease(int button, int x, int y, int modKeys);
    void MouseMove(int x, int y, int modKeys);
    void KeyRelease(int key);
    void Idle() {}
    bool WriteToFile(std::string const &attachment,
                     std::string const &filename) const;

    void ParseArgs(int argc, char *argv[]);

private:
    void SetCameraRotate(float rx, float ry)
    {
        _rotate[0] = rx;
        _rotate[1] = ry;
    }
    void SetCameraTranslate(pxr::GfVec3f t)
    {
        _translate = t;
    }
    pxr::GfVec3f GetCameraTranslate() const
    {
        return _translate;
    }
    pxr::GfMatrix4d GetViewMatrix() const;
    pxr::GfMatrix4d GetProjectionMatrix(int width, int height) const;
    pxr::GfFrustum GetFrustum(int width, int height) const;
    pxr::GfVec2i GetMousePos() const { return pxr::GfVec2i(_mousePos[0], _mousePos[1]); }

private:
    float _rotate[2];
    pxr::GfVec3f _translate;
    int _mousePos[2];
    bool _mouseButton[3];

    // Hgi and HdDriver should be constructed before HdEngine to ensure they
    // are destructed last. Hgi may be used during engine/delegate destruction.
    pxr::HgiUniquePtr _hgi;
    std::unique_ptr<pxr::HdDriver> _driver;
    pxr::HdEngine _engine;
    pxr::HdStRenderDelegate _renderDelegate;
    pxr::HdRenderIndex *_renderIndex;
    pxr::Hdx_UnitTestDelegate *_delegate;
    pxr::TfToken _reprName;
    int _refineLevel;
    pxr::GlfDrawTargetRefPtr _drawTarget;
};
