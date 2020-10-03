#pragma once
#include "unitTestDelegate.h"
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/imaging/hdSt/unitTestGLDrawing.h>
#include "pxr/imaging/hdSt/renderDelegate.h"
#include <pxr/imaging/hd/tokens.h>
#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hdx/renderTask.h"
#include "pxr/imaging/hdx/renderSetupTask.h"
#include "pxr/imaging/hgi/hgi.h"


class My_TestGLDrawing : public pxr::HdSt_UnitTestGLDrawing
{
public:
    My_TestGLDrawing()
    {
        SetCameraRotate(0, 0);
        SetCameraTranslate(pxr::GfVec3f(0));
        _reprName = pxr::HdReprTokens->hull;
        _refineLevel = 0;
    }

    struct PickParam
    {
        pxr::GfVec2d location;
        pxr::GfVec4d viewport;
    };

    void DrawScene(PickParam const *pickParam = nullptr);

    pxr::SdfPath PickScene(int pickX, int pickY, int *outInstanceIndex = nullptr);

    // HdSt_UnitTestGLDrawing overrides
    void InitTest() override;
    void UninitTest() override;
    void DrawTest() override;
    void OffscreenTest() override;

    void MousePress(int button, int x, int y, int modKeys) override;

protected:
    void ParseArgs(int argc, char *argv[]) override;

private:
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
};
