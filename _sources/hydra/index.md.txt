# Hydra Framework

-   (2019)<https://graphics.pixar.com/usd/files/Siggraph2019_Hydra.pdf>

> Hydra is an open source framework to transport live scene graph data to renderers.

-   (202009)[USD Hydra 解読](https://qiita.com/ousttrue/items/3b44377e6e24c034649c)
-   <https://github.com/ousttrue/HydraHost>

RenderIndex がシーンとレンダラーを仲介する。

```{blockdiag}
blockdiag {
    S_Usd[label = "USDImaging(SceneDelegate)"];
    S_Tiny[label = "Tiny(SceneDelegate)"];
    R_Tiny[label = "HdStorm(RenderDelegate)"];
    R_Storm[label = "HdStorm(RenderDelegate)"];
    R_Embree[label = "HdEmbree(RenderDelegate)"];
    S_Usd -> RenderIndex;
    S_Tiny -> RenderIndex;
    RenderIndex -> R_Tiny;
    RenderIndex -> R_Storm;
    RenderIndex -> R_Embree;
}
```

```{toctree}
HdEngine
Tiny
UsdImagingGLEngine
```
