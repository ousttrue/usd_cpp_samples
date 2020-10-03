#include <iostream>
#include "UnitTestWindow.h"
#include "GLEngine.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/bboxCache.h>


class Impl
{
    pxr::UsdStageRefPtr _stage;
    GLEngine *_engine = nullptr;

    float _rotate[2] = {0, 0};
    float _translate[3] = {0.0f, 0.0f, -100.0f};
    int _mousePos[2] = {0, 0};
    bool _mouseButton[3] = {false, false, false};

public:
    Impl(const char *usdFile)
    {
        _stage = pxr::UsdStage::Open(usdFile);
    }

    ~Impl()
    {
        Shutdown();
    }

    void Shutdown()
    {
        if (_engine)
        {
            delete _engine;
            _engine = nullptr;
        }
        std::cout << "GLDrawing::ShutdownTest()\n";
    }

    uint32_t Draw(int width, int height)
    {
        // TRACE_FUNCTION();

        RenderFrameInfo frameInfo;

        frameInfo.clearDepth = {1.0f};

        frameInfo.viewport = pxr::GfVec4d(0, 0, width, height);
        {
            pxr::GfMatrix4d viewMatrix(1.0);
            viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(0, 1, 0), _rotate[0]));
            viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1, 0, 0), _rotate[1]));
            viewMatrix *= pxr::GfMatrix4d().SetTranslate(pxr::GfVec3d(_translate[0], _translate[1], _translate[2]));

            frameInfo.modelViewMatrix = viewMatrix;
            if (pxr::UsdGeomGetStageUpAxis(_stage) == pxr::UsdGeomTokens->z)
            {
                // rotate from z-up to y-up
                frameInfo.modelViewMatrix =
                    pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1.0, 0.0, 0.0), -90.0)) *
                    frameInfo.modelViewMatrix;
            }

            const double aspectRatio = double(width) / height;
            pxr::GfFrustum frustum;
            frustum.SetPerspective(60.0, aspectRatio, 1, 100000.0);
            frameInfo.projectionMatrix = frustum.ComputeProjectionMatrix();
        }

        if (!_engine)
        {
            std::cout << "Using HD Renderer.\n";
            _engine = new GLEngine(_stage->GetPseudoRoot().GetPath());
        }

        return _engine->RenderFrame(frameInfo, _stage->GetPseudoRoot());
    }

    void MousePress(int button, int x, int y, int modKeys)
    {
        _mouseButton[button] = 1;
        _mousePos[0] = x;
        _mousePos[1] = y;
    }

    void MouseRelease(int button, int x, int y, int modKeys)
    {
        _mouseButton[button] = 0;
    }

    void MouseMove(int x, int y, int modKeys)
    {
        int dx = x - _mousePos[0];
        int dy = y - _mousePos[1];

        if (_mouseButton[0])
        {
            _rotate[0] += dx;
            _rotate[1] += dy;
        }
        else if (_mouseButton[1])
        {
            _translate[0] += dx;
            _translate[1] -= dy;
        }
        else if (_mouseButton[2])
        {
            _translate[2] += dx;
        }

        _mousePos[0] = x;
        _mousePos[1] = y;
    }

    void KeyRelease(int key)
    {
    }
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return 1;
    }

    Impl driver(argv[1]);

    // create GL Context / window
    Input input;
    input.OnMousePress = [&driver](int button, int x, int y, int mod) {
        driver.MousePress(button, x, y, mod);
    };
    input.OnMouseRelease = [&driver](int button, int x, int y, int mod) {
        driver.MouseRelease(button, x, y, mod);
    };
    input.OnMouseMove = [&driver](int button, int x, int y) {
        driver.MouseMove(button, x, y);
    };
    input.OnKeyRelease = [&driver](int key) {
        driver.KeyRelease(key);
    };
    auto context = UnitTestWindow(
        640, 480,
        [&driver](int w, int h) {},
        [&driver](int w, int h) {
            return driver.Draw(w, h);
        },
        [&driver]() { driver.Shutdown(); },
        input);

    context.Init();
    context.Run();

    std::cout << "OK" << std::endl;
}
