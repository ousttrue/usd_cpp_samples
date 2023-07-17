#include "HDHost.h"
#include <iostream>
#include <pxr/imaging/garch/glDebugWindow.h>

class UnitTestWindow : public pxr::GarchGLDebugWindow {

  HDHost _host;

public:
  UnitTestWindow(int w, int h, const char *usdFile)
      : GarchGLDebugWindow("UsdImagingGL Test", w, h) {

    _host.Load(usdFile);
  }

  ~UnitTestWindow() {
    std::cout << "UnitTestWindow::~UnitTestWindow" << std::endl;
  }

  void OnUninitializeGL() { _host.Shutdown(); }

  void OnPaintGL() {
    //
    // Update the draw target's size and execute the unit test with
    // the draw target bound.
    //
    int width = GetWidth();
    int height = GetHeight();
    _host.Draw(width, height);
  }

  virtual void OnKeyRelease(int key) {
    switch (key) {
    case 'q':
      ExitApp();
    }
  }

  virtual void OnMousePress(int button, int x, int y, int modKeys) {
    _host.MousePress(button, x, y, modKeys);
  }

  virtual void OnMouseRelease(int button, int x, int y, int modKeys) {
    _host.MouseRelease(button, x, y, modKeys);
  }

  virtual void OnMouseMove(int x, int y, int modKeys) {
    _host.MouseMove(x, y, modKeys);
  }
};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }

  auto context = UnitTestWindow(640, 480, argv[1]);
  context.Init();
  context.Run();

  return 0;
}
