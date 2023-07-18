#pragma once
#include <functional>
#include <optional>

struct Frame {
  int Width;
  int Height;
};

struct GLFWwindow;
class Platform {
  struct PlatformImpl *m_impl;

public:
  Platform();
  ~Platform();
  GLFWwindow *CreateWindow();
  std::optional<Frame> BeginFrame();
  void EndFrame();
  const char *GlslVersion() const;
  void UpdateViewports(const std::function<void()> &callback);
};
