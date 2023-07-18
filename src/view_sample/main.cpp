#include "gui.h"
#include "platform.h"
#include "usd_renderer.h"
#include <GL/glew.h>
#include <grapho/camera/camera.h>
#include <imgui.h>

int main(int argc, char **argv) {

  if (argc < 2) {
    return 1;
  }

  Gui gui;

  Platform platform;
  auto window = platform.CreateWindow();
  if (!window) {
    return 2;
  }

  UsdRenderer r;
  if(!r.Load(argv[1]))
  {
    return 3;
  }

  grapho::camera::Camera camera;
  // camera.Transform.Translation = {0, 0, 100};
  // camera.Projection.NearZ = 10.0f;
  // camera.Projection.FarZ = 10000.0f;

  // Main loop
  auto &io = ImGui::GetIO();
  while (auto frame = platform.BeginFrame()) {
    gui.Update();

    // camera
    camera.Projection.SetSize(io.DisplaySize.x, io.DisplaySize.y);
    if (!io.WantCaptureMouse) {
      if (io.MouseDown[ImGuiMouseButton_Right]) {
        camera.YawPitch(static_cast<int>(io.MouseDelta.x),
                        static_cast<int>(io.MouseDelta.y));
      }
      if (io.MouseDown[ImGuiMouseButton_Middle]) {
        camera.Shift(static_cast<int>(io.MouseDelta.x),
                     static_cast<int>(io.MouseDelta.y));
      }
      camera.Dolly(static_cast<int>(io.MouseWheel));
    }
    camera.Update();

    // Rendering
    glViewport(0, 0, frame->Width, frame->Height);
    glClearColor(gui.clear_color[0] * gui.clear_color[3],
                 gui.clear_color[1] * gui.clear_color[3],
                 gui.clear_color[2] * gui.clear_color[3], gui.clear_color[3]);
    // glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    r.Render(&camera.Transform.Translation.x, &camera.ViewMatrix._11,
             frame->Width, frame->Height, &camera.ProjectionMatrix._11);

    platform.EndFrame();
  }

  return 0;
}
