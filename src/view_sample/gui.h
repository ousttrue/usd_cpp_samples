#pragma once

class Gui {
  struct GuiImpl *m_impl;

public:
  float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

  Gui();
  ~Gui();
  void Update();
};
