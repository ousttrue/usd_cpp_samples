#pragma once
#include <filesystem>

class UsdRenderer {
  struct UsdRendererImpl *m_impl;

public:
  UsdRenderer();
  ~UsdRenderer();
  bool Load(const std::filesystem::path &path);
  void Render(const float *camera_position, const float *view_matrix, int width,
              int height, const float *projection_matrix);
};
