# UsdImagingGLEngine

ハイレベルインタフェース。
{doc}`HdEngine` で `UsdImagingDelegate` を駆動するラッパー。

stage を OpenGL にレンダリングする。

```c++
  // load usd
  pxr::UsdStageRefPtr stage;
  if (argc > 1) {
    stage = pxr::UsdStage::Open(argv[1]);
  }
  if (!stage) {
    return 2;
  }

  // camera
  pxr::GfCamera camera;
  float dist = 100.0f;
  pxr::GfMatrix4d t;
  t.SetTranslate(pxr::GfVec3d::ZAxis() * dist);
  camera.SetTransform(t);
  camera.SetFocusDistance(dist);

  // renderer
  pxr::UsdImagingGLEngine engine;
  // param
  pxr::UsdImagingGLRenderParams params;
  params.enableLighting = false;

  int width, height;
  while (w.IsRunning(&width, &height)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    engine.SetRenderViewport(pxr::GfVec4d(0, 0, width, height));
    engine.Render(stage->GetPseudoRoot(), params);
    engine.SetCameraState(camera.GetFrustum().ComputeViewMatrix(),
                          camera.GetFrustum().ComputeProjectionMatrix());

    w.SwapBuffers();
  }
```

## usdtweak

`UsdImagingGLEngine` と `ImGui` を合体させたツール。

* <https://github.com/cpichard/usdtweak>
