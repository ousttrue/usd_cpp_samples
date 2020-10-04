#include "testHdxRenderer.h"
#include "unitTestDelegate.h"
#include <pxr/imaging/garch/glDebugWindow.h>
#include "pxr/imaging/hdx/renderTask.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3f.h"
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

static pxr::GfMatrix4d
_GetTranslate(float tx, float ty, float tz)
{
    pxr::GfMatrix4d m(1.0f);
    m.SetRow(3, pxr::GfVec4f(tx, ty, tz, 1.0));
    return m;
}

class SceneManager
{
    pxr::Hdx_UnitTestDelegate *_delegate;
    int _refineLevel = 0;
    pxr::TfToken _reprName = pxr::HdReprTokens->hull;

    float _rotate[2] = {0, 0};
    pxr::GfVec3f _translate = pxr::GfVec3f(0, 0, 0);
    int _mousePos[2] = {0, 0};
    bool _mouseButton[3] = {false, false, false};

public:
    SceneManager()
    {
        SetCameraRotate(0, 0);
        SetCameraTranslate(pxr::GfVec3f(0, 0, -20));
    }

    ~SceneManager()
    {
        Uninit();
    }

    void Uninit()
    {
        if (_delegate)
        {
            delete _delegate;
            _delegate = nullptr;
        }
    }

    void CreateDelegate(pxr::HdRenderIndex *renderIndex)
    {
        _delegate = new pxr::Hdx_UnitTestDelegate(renderIndex);

        _delegate->SetRefineLevel(_refineLevel);

        // prepare render task
        pxr::SdfPath renderSetupTask("/renderSetupTask");
        pxr::SdfPath renderTask("/renderTask");
        _delegate->AddRenderSetupTask(renderSetupTask);
        _delegate->AddRenderTask(renderTask);

        // render task parameters.
        pxr::HdxRenderTaskParams param = _delegate->GetTaskParam(
                                                      renderSetupTask, pxr::HdTokens->params)
                                             .Get<pxr::HdxRenderTaskParams>();
        param.enableLighting = true; // use default lighting
        _delegate->SetTaskParam(renderSetupTask, pxr::HdTokens->params, pxr::VtValue(param));
        _delegate->SetTaskParam(renderTask, pxr::HdTokens->collection,
                                pxr::VtValue(pxr::HdRprimCollection(pxr::HdTokens->geometry,
                                                                    pxr::HdReprSelector(_reprName))));

        // prepare scene
        // To ensure that the non-aggregated element index returned via picking,
        // we need to have at least two cubes with uniform colors.
        pxr::GfVec4f red(1, 0, 0, 1), green(0, 1, 0, 1), blue(0, 0, 1, 1),
            yellow(1, 1, 0, 1), magenta(1, 0, 1, 1), cyan(0, 1, 1, 1),
            white(1, 1, 1, 1), black(0, 0, 0, 1);

        pxr::GfVec4f faceColors[] = {red, green, blue, yellow, magenta, cyan};
        pxr::VtValue faceColor = pxr::VtValue(_BuildArray(&faceColors[0],
                                                          sizeof(faceColors) / sizeof(faceColors[0])));

        pxr::GfVec4f vertColors[] = {white, blue, green, yellow,
                                     black, blue, magenta, red};
        pxr::VtValue vertColor = pxr::VtValue(_BuildArray(&vertColors[0],
                                                          sizeof(vertColors) / sizeof(vertColors[0])));

        _delegate->AddCube(pxr::SdfPath("/cube0"), _GetTranslate(5, 0, 5),
                           /*guide=*/false, /*instancerId=*/pxr::SdfPath(),
                           /*scheme=*/pxr::PxOsdOpenSubdivTokens->catmullClark,
                           /*color=*/faceColor,
                           /*colorInterpolation=*/pxr::HdInterpolationUniform);
        _delegate->AddCube(pxr::SdfPath("/cube1"), _GetTranslate(-5, 0, 5),
                           /*guide=*/false, /*instancerId=*/pxr::SdfPath(),
                           /*scheme=*/pxr::PxOsdOpenSubdivTokens->catmullClark,
                           /*color=*/faceColor,
                           /*colorInterpolation=*/pxr::HdInterpolationUniform);
        _delegate->AddCube(pxr::SdfPath("/cube2"), _GetTranslate(-5, 0, -5));
        _delegate->AddCube(pxr::SdfPath("/cube3"), _GetTranslate(5, 0, -5),
                           /*guide=*/false, /*instancerId=*/pxr::SdfPath(),
                           /*scheme=*/pxr::PxOsdOpenSubdivTokens->catmullClark,
                           /*color=*/vertColor,
                           /*colorInterpolation=*/pxr::HdInterpolationVertex);

        {
            _delegate->AddInstancer(pxr::SdfPath("/instancerTop"));
            _delegate->AddCube(pxr::SdfPath("/protoTop"),
                               pxr::GfMatrix4d(1), false, pxr::SdfPath("/instancerTop"));

            std::vector<pxr::SdfPath> prototypes;
            prototypes.push_back(pxr::SdfPath("/protoTop"));

            pxr::VtVec3fArray scale(3);
            pxr::VtVec4fArray rotate(3);
            pxr::VtVec3fArray translate(3);
            pxr::VtIntArray prototypeIndex(3);

            scale[0] = pxr::GfVec3f(1);
            rotate[0] = pxr::GfVec4f(0);
            translate[0] = pxr::GfVec3f(3, 0, 2);
            prototypeIndex[0] = 0;

            scale[1] = pxr::GfVec3f(1);
            rotate[1] = pxr::GfVec4f(0);
            translate[1] = pxr::GfVec3f(0, 0, 2);
            prototypeIndex[1] = 0;

            scale[2] = pxr::GfVec3f(1);
            rotate[2] = pxr::GfVec4f(0);
            translate[2] = pxr::GfVec3f(-3, 0, 2);
            prototypeIndex[2] = 0;

            _delegate->SetInstancerProperties(pxr::SdfPath("/instancerTop"),
                                              prototypeIndex,
                                              scale, rotate, translate);
        }

        {
            _delegate->AddInstancer(pxr::SdfPath("/instancerBottom"));
            _delegate->AddCube(pxr::SdfPath("/protoBottom"),
                               pxr::GfMatrix4d(1), false, pxr::SdfPath("/instancerBottom"));

            std::vector<pxr::SdfPath> prototypes;
            prototypes.push_back(pxr::SdfPath("/protoBottom"));

            pxr::VtVec3fArray scale(3);
            pxr::VtVec4fArray rotate(3);
            pxr::VtVec3fArray translate(3);
            pxr::VtIntArray prototypeIndex(3);

            scale[0] = pxr::GfVec3f(1);
            rotate[0] = pxr::GfVec4f(0);
            translate[0] = pxr::GfVec3f(3, 0, -2);
            prototypeIndex[0] = 0;

            scale[1] = pxr::GfVec3f(1);
            rotate[1] = pxr::GfVec4f(0);
            translate[1] = pxr::GfVec3f(0, 0, -2);
            prototypeIndex[1] = 0;

            scale[2] = pxr::GfVec3f(1);
            rotate[2] = pxr::GfVec4f(0);
            translate[2] = pxr::GfVec3f(-3, 0, -2);
            prototypeIndex[2] = 0;

            _delegate->SetInstancerProperties(pxr::SdfPath("/instancerBottom"),
                                              prototypeIndex,
                                              scale, rotate, translate);
        }
    }

    pxr::HdSceneDelegate* Prepare(int width, int height)
    {
        pxr::GfMatrix4d viewMatrix = GetViewMatrix();

        pxr::GfFrustum frustum = GetFrustum(width, height);
        pxr::GfVec4d viewport(0, 0, width, height);

        pxr::GfMatrix4d projMatrix = frustum.ComputeProjectionMatrix();
        _delegate->SetCamera(viewMatrix, projMatrix);

        pxr::SdfPath renderSetupTask("/renderSetupTask");
        pxr::HdxRenderTaskParams param = _delegate->GetTaskParam(
                                                      renderSetupTask, pxr::HdTokens->params)
                                             .Get<pxr::HdxRenderTaskParams>();
        // param.enableIdRender = (pickParam != nullptr);
        param.viewport = viewport;
        _delegate->SetTaskParam(renderSetupTask, pxr::HdTokens->params, pxr::VtValue(param));

        return _delegate;
    }

    void MousePress(int button, int x, int y, int modKeys)
    {
        _mouseButton[button] = true;
        _mousePos[0] = x;
        _mousePos[1] = y;
    }

    void MouseRelease(int button, int x, int y, int modKeys)
    {
        _mouseButton[button] = false;
    }

    void MouseMove(int x, int y, int modKeys)
    {
        int dx = x - _mousePos[0];
        int dy = y - _mousePos[1];

        if (modKeys & pxr::GarchGLDebugWindow::Alt)
        {
            if (_mouseButton[0])
            {
                _rotate[1] += dx;
                _rotate[0] += dy;
            }
            else if (_mouseButton[1])
            {
                _translate[0] += 0.1 * dx;
                _translate[1] -= 0.1 * dy;
            }
            else if (_mouseButton[2])
            {
                _translate[2] += 0.1 * dx;
            }
        }

        _mousePos[0] = x;
        _mousePos[1] = y;
    }

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

    pxr::GfMatrix4d GetViewMatrix() const
    {
        pxr::GfMatrix4d viewMatrix;
        viewMatrix.SetIdentity();
        // rotate from z-up to y-up
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1.0, 0.0, 0.0), -90.0));
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(0, 1, 0), _rotate[1]));
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1, 0, 0), _rotate[0]));
        viewMatrix *= pxr::GfMatrix4d().SetTranslate(pxr::GfVec3d(_translate[0], _translate[1], _translate[2]));

        return viewMatrix;
    }

    pxr::GfMatrix4d GetProjectionMatrix(int width, int height) const
    {
        return GetFrustum(width, height).ComputeProjectionMatrix();
    }

    pxr::GfFrustum GetFrustum(int width, int height) const
    {
        double aspectRatio = double(width) / height;

        pxr::GfFrustum frustum;
        frustum.SetPerspective(45.0, aspectRatio, 1, 100000.0);
        return frustum;
    }
    pxr::GfVec2i GetMousePos() const { return pxr::GfVec2i(_mousePos[0], _mousePos[1]); }

};

int main(int argc, char *argv[])
{
    pxr::TfErrorMark mark;

    {
        Drawing drawing;
        SceneManager scene;

        Callback callback;
        callback.OnInitializeGL = [&drawing, &scene](int width, int height) {
            auto renderIndex = drawing.Init(width, height);
            scene.CreateDelegate(renderIndex);
        };
        callback.OnUninitializeGL = [&drawing, &scene]() {
            scene.Uninit();
            drawing.Uninit();
        };
        callback.OnPaintGL = [&drawing, &scene](int width, int height) {
            auto sceneDelegate = scene.Prepare(width, height);
            drawing.Draw(width, height, sceneDelegate);
        };
        callback.OnMousePress = [&scene](int b, int x, int y, int m) { scene.MousePress(b, x, y, m); };
        callback.OnMouseRelease = [&scene](int b, int x, int y, int m) { scene.MouseRelease(b, x, y, m); };
        callback.OnMouseMove = [&scene](int x, int y, int m) { scene.MouseMove(x, y, m); };
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
