#pragma once
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/rotation.h>

class CameraView {
  float _rotate[2] = {0, 0};
  float _translate[3] = {0.0f, 0.0f, -100.0f};
  int _mousePos[2] = {0, 0};
  bool _mouseButton[3] = {false, false, false};

public:
  pxr::GfMatrix4d ViewMatrix() const {
    pxr::GfMatrix4d viewMatrix(1.0);
    viewMatrix *= pxr::GfMatrix4d().SetRotate(
        pxr::GfRotation(pxr::GfVec3d(0, 1, 0), _rotate[0]));
    viewMatrix *= pxr::GfMatrix4d().SetRotate(
        pxr::GfRotation(pxr::GfVec3d(1, 0, 0), _rotate[1]));
    viewMatrix *= pxr::GfMatrix4d().SetTranslate(
        pxr::GfVec3d(_translate[0], _translate[1], _translate[2]));
    return viewMatrix;
  }

  void MousePress(int button, int x, int y, int modKeys) {
    _mouseButton[button] = 1;
    _mousePos[0] = x;
    _mousePos[1] = y;
  }

  void MouseRelease(int button, int x, int y, int modKeys) {
    _mouseButton[button] = 0;
  }

  void MouseMove(int x, int y, int modKeys) {
    int dx = x - _mousePos[0];
    int dy = y - _mousePos[1];

    if (_mouseButton[0]) {
      _rotate[0] += dx;
      _rotate[1] += dy;
    } else if (_mouseButton[1]) {
      _translate[0] += dx;
      _translate[1] -= dy;
    } else if (_mouseButton[2]) {
      _translate[2] += dx;
    }

    _mousePos[0] = x;
    _mousePos[1] = y;
  }
};
