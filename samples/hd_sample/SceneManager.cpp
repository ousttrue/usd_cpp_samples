#include "SceneManager.h"
#include "unitTestDelegate.h"
#include "pxr/imaging/hdx/renderTask.h"
#include <pxr/imaging/garch/glDebugWindow.h>

static pxr::GfMatrix4d
_GetTranslate(float tx, float ty, float tz)
{
    pxr::GfMatrix4d m(1.0f);
    m.SetRow(3, pxr::GfVec4f(tx, ty, tz, 1.0));
    return m;
}

SceneManager::SceneManager()
{
    SetCameraRotate(0, 0);
    SetCameraTranslate(pxr::GfVec3f(0, 0, -20));
}

SceneManager::~SceneManager()
{
    Uninit();
}

void SceneManager::Uninit()
{
    if (_delegate)
    {
        delete _delegate;
        _delegate = nullptr;
    }
}

void SceneManager::CreateDelegate(pxr::HdRenderIndex *renderIndex)
{
    _delegate = new pxr::Hdx_UnitTestDelegate(renderIndex);

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
}

pxr::HdSceneDelegate *SceneManager::Prepare(int width, int height)
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

void SceneManager::MousePress(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = true;
    _mousePos[0] = x;
    _mousePos[1] = y;
}

void SceneManager::MouseRelease(int button, int x, int y, int modKeys)
{
    _mouseButton[button] = false;
}

void SceneManager::MouseMove(int x, int y, int modKeys)
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

void SceneManager::SetCameraRotate(float rx, float ry)
{
    _rotate[0] = rx;
    _rotate[1] = ry;
}

void SceneManager::SetCameraTranslate(pxr::GfVec3f t)
{
    _translate = t;
}

pxr::GfVec3f SceneManager::GetCameraTranslate() const
{
    return _translate;
}

pxr::GfMatrix4d SceneManager::GetViewMatrix() const
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

pxr::GfMatrix4d SceneManager::GetProjectionMatrix(int width, int height) const
{
    return GetFrustum(width, height).ComputeProjectionMatrix();
}

pxr::GfFrustum SceneManager::GetFrustum(int width, int height) const
{
    double aspectRatio = double(width) / height;

    pxr::GfFrustum frustum;
    frustum.SetPerspective(45.0, aspectRatio, 1, 100000.0);
    return frustum;
}
