#include "gui.h"
#include "platform.h"
#include <GL/glew.h>

int main(int, char **) {

  Gui gui;

  Platform platform;
  auto window = platform.CreateWindow();
  if (!window) {
    return 1;
  }

  // Main loop
  while (auto frame = platform.BeginFrame()) {
    gui.Update();

    // Rendering
    glViewport(0, 0, frame->Width, frame->Height);
    glClearColor(gui.clear_color[0] * gui.clear_color[3],
                 gui.clear_color[1] * gui.clear_color[3],
                 gui.clear_color[2] * gui.clear_color[3], gui.clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);

    platform.EndFrame();
  }


  return 0;
}
