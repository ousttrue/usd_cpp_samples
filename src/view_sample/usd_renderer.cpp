#include "usd_renderer.h"
#include <DirectXMath.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>

static pxr::GfMatrix4d m(const float m[16]) {
  return {
      m[0],  m[1],  m[2],  m[3],  //
      m[4],  m[5],  m[6],  m[7],  //
      m[8],  m[9],  m[10], m[11], //
      m[12], m[13], m[14], m[15], //
  };
}

struct UsdRendererImpl {
  pxr::UsdStageRefPtr Stage;
  pxr::UsdImagingGLEngine Renderer;

  bool Load(const std::filesystem::path &path) {
    Stage = pxr::UsdStage::Open(path.string());
    return Stage != nullptr;
  }

  void Render(const float *cam_pos, const float *view_matrix, int width,
              int height, const float *projection_matrix) {

    {
      pxr::GlfSimpleLightVector lights;
      {
        pxr::GlfSimpleLight l;
        l.SetAmbient(pxr::GfVec4f(0, 0, 0, 0));
        l.SetPosition(pxr::GfVec4f(cam_pos[0], cam_pos[1], cam_pos[2], 1.f));
        lights.push_back(l);
      }

      pxr::GlfSimpleMaterial material;
      material.SetAmbient(pxr::GfVec4f(.1f, .1f, .1f, 1.f));
      material.SetSpecular(pxr::GfVec4f(.6f, .6f, .6f, .6f));
      material.SetShininess(16.0);

      Renderer.SetRenderViewport(pxr::GfVec4d(0, 0, width, height));

      Renderer.SetCameraState(m(view_matrix), m(projection_matrix));
      pxr::GfVec4f ambient(pxr::GfVec4f(.1f, .1f, .1f, 1.f));
      Renderer.SetLightingState(lights, material, ambient);
    }

    pxr::UsdImagingGLRenderParams RenderParams;
    RenderParams.drawMode =
        pxr::UsdImagingGLDrawMode::DRAW_WIREFRAME_ON_SURFACE;
    RenderParams.enableLighting = true;
    RenderParams.enableIdRender = false;
    RenderParams.frame = 0;
    RenderParams.complexity = 1;
    RenderParams.cullStyle =
        pxr::UsdImagingGLCullStyle::CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED;
    RenderParams.enableSceneMaterials = false;
    RenderParams.highlight = true;

    auto root = Stage->GetPseudoRoot();
    Renderer.Render(root, RenderParams);
  }
};

UsdRenderer::UsdRenderer() : m_impl(new UsdRendererImpl) {}

UsdRenderer::~UsdRenderer() { delete m_impl; }

bool UsdRenderer::Load(const std::filesystem::path &path) {
  return m_impl->Load(path);
}

void UsdRenderer::Render(const float *pos, const float *view_matrix, int width,
                         int height, const float *projection_matrix) {
  m_impl->Render(pos, view_matrix, width, height, projection_matrix);
}
