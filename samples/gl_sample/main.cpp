#include <iostream>
#include "UnitTestWindow.h"
#include "GLEngine.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usdImaging/usdImaging/delegate.h>

class CameraView
{
    float _rotate[2] = {0, 0};
    float _translate[3] = {0.0f, 0.0f, -100.0f};
    int _mousePos[2] = {0, 0};
    bool _mouseButton[3] = {false, false, false};

public:
    pxr::GfMatrix4d ViewMatrix() const
    {
        pxr::GfMatrix4d viewMatrix(1.0);
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(0, 1, 0), _rotate[0]));
        viewMatrix *= pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1, 0, 0), _rotate[1]));
        viewMatrix *= pxr::GfMatrix4d().SetTranslate(pxr::GfVec3d(_translate[0], _translate[1], _translate[2]));
        return viewMatrix;
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
};


static int
_GetRefineLevel(float c)
{
    // TODO: Change complexity to refineLevel when we refactor UsdImaging.
    //
    // Convert complexity float to refine level int.
    int refineLevel = 0;

    // to avoid floating point inaccuracy (e.g. 1.3 > 1.3f)
    c = std::min(c + 0.01f, 2.0f);

    if (1.0f <= c && c < 1.1f)
    {
        refineLevel = 0;
    }
    else if (1.1f <= c && c < 1.2f)
    {
        refineLevel = 1;
    }
    else if (1.2f <= c && c < 1.3f)
    {
        refineLevel = 2;
    }
    else if (1.3f <= c && c < 1.4f)
    {
        refineLevel = 3;
    }
    else if (1.4f <= c && c < 1.5f)
    {
        refineLevel = 4;
    }
    else if (1.5f <= c && c < 1.6f)
    {
        refineLevel = 5;
    }
    else if (1.6f <= c && c < 1.7f)
    {
        refineLevel = 6;
    }
    else if (1.7f <= c && c < 1.8f)
    {
        refineLevel = 7;
    }
    else if (1.8f <= c && c <= 2.0f)
    {
        refineLevel = 8;
    }
    else
    {
        using namespace pxr;
        TF_CODING_ERROR("Invalid complexity %f, expected range is [1.0,2.0]\n",
                        c);
    }
    return refineLevel;
}

class Impl
{
    pxr::UsdStageRefPtr _stage;
    pxr::SdfPath _rootPath;
    GLEngine *_engine = nullptr;

    pxr::UsdImagingDelegate *_sceneDelegate = nullptr;
    bool _isPopulated = false;

public:
    Impl(const char *usdFile)
    {
        _stage = pxr::UsdStage::Open(usdFile);
        _rootPath = _stage->GetPseudoRoot().GetPath();
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
        if (_sceneDelegate)
        {
            delete _sceneDelegate;
            _sceneDelegate = nullptr;
        }
        std::cout << "GLDrawing::ShutdownTest()\n";
    }

    uint32_t Draw(int width, int height, const pxr::GfMatrix4d &viewMatrix)
    {
        const double aspectRatio = double(width) / height;
        pxr::GfFrustum frustum;
        frustum.SetPerspective(60.0, aspectRatio, 1, 100000.0);

        RenderFrameInfo frameInfo;
        frameInfo.clearDepth = {1.0f};
        frameInfo.viewport = pxr::GfVec4d(0, 0, width, height);
        frameInfo.modelViewMatrix = viewMatrix;
        if (pxr::UsdGeomGetStageUpAxis(_stage) == pxr::UsdGeomTokens->z)
        {
            // rotate from z-up to y-up
            frameInfo.modelViewMatrix = pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d(1.0, 0.0, 0.0), -90.0)) * frameInfo.modelViewMatrix;
        }
        frameInfo.projectionMatrix = frustum.ComputeProjectionMatrix();

        if (!_engine)
        {
            std::cout << "Using HD Renderer.\n";
            _engine = new GLEngine(std::bind(&Impl::CreateSceneDelegate, this, std::placeholders::_1, pxr::SdfPath::AbsoluteRootPath()));
        }

        PrepareBatch();
        // XXX(UsdImagingPaths): Is it correct to map USD root path directly
        // to the cachePath here?
        // const SdfPath cachePath = root.GetPath();
        auto paths = {
            _sceneDelegate->ConvertCachePathToIndexPath(_rootPath)};

        return _engine->RenderFrame(frameInfo, paths);
    }

    void CreateSceneDelegate(pxr::HdRenderIndex *renderIndex, const pxr::SdfPath &delegateId)
    {
        _sceneDelegate = new pxr::UsdImagingDelegate(renderIndex, delegateId);
    }

    // bool _CanPrepareBatch(
    //     const pxr::UsdPrim &root,
    //     const pxr::UsdImagingGLRenderParams &params)
    // {
    //     using namespace pxr;
    //     HD_TRACE_FUNCTION();

    //     if (!TF_VERIFY(root, "Attempting to draw an invalid/null prim\n"))
    //         return false;

    //     if (!root.GetPath().HasPrefix(_rootPath))
    //     {
    //         TF_CODING_ERROR("Attempting to draw path <%s>, but engine is rooted"
    //                         "at <%s>\n",
    //                         root.GetPath().GetText(),
    //                         _rootPath.GetText());
    //         return false;
    //     }

    //     return true;
    // }

    void PrepareBatch()
    {
        using namespace pxr;

        HD_TRACE_FUNCTION();

        TF_VERIFY(_sceneDelegate);

        // if (_CanPrepareBatch(root, params))
        {
            if (!_isPopulated)
            {
                // _sceneDelegate->SetUsdDrawModesEnabled(params.enableUsdDrawModes);
                pxr::SdfPathVector _excludedPrimPaths;
                _sceneDelegate->Populate(
                    _stage->GetPrimAtPath(_rootPath),
                    _excludedPrimPaths);
                pxr::SdfPathVector _invisedPrimPaths;
                _sceneDelegate->SetInvisedPrimPaths(_invisedPrimPaths);
                _isPopulated = true;
            }

            // _PreSetTime(root, params);
            {
                using namespace pxr;
                HD_TRACE_FUNCTION();

                // Set the fallback refine level, if this changes from the existing value,
                // all prim refine levels will be dirtied.
                const int refineLevel = _GetRefineLevel(/*params.complexity*/1.0f);
                _sceneDelegate->SetRefineLevelFallback(refineLevel);

                // Apply any queued up scene edits.
                _sceneDelegate->ApplyPendingUpdates();
            }

            // SetTime will only react if time actually changes.
            // _sceneDelegate->SetTime(params.frame);
        }
    }
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return 1;
    }

    Impl driver(argv[1]);
    CameraView view;

    // create GL Context / window
    Input input;
    input.OnMousePress = [&view](int button, int x, int y, int mod) {
        view.MousePress(button, x, y, mod);
    };
    input.OnMouseRelease = [&view](int button, int x, int y, int mod) {
        view.MouseRelease(button, x, y, mod);
    };
    input.OnMouseMove = [&view](int button, int x, int y) {
        view.MouseMove(button, x, y);
    };
    // input.OnKeyRelease = [&driver](int key) {
    //     driver.KeyRelease(key);
    // };
    auto context = UnitTestWindow(
        640, 480,
        [&driver](int w, int h) {
        },
        [&driver, &view](int w, int h) {
            return driver.Draw(w, h, view.ViewMatrix());
        },
        [&driver]() { driver.Shutdown(); },
        input);

    context.Init();
    context.Run();

    std::cout << "OK" << std::endl;
}
