# pxr::HdEngine

RenderIndex を介して、RenderDelegate を制御する。
RenderDelegate は RenderIndex から情報を pull する。

## Sync
## Prepare
## Commit
## Execute


## TaskController

```c++
_taskController = std::make_unique<pxr::HdxTaskController>(
_renderIndex.get(), _ComputeControllerPath(_renderDelegate));
```

## SceneDelegate から path のリストを得る

_stage と _sceneDelegate から path のリストを得る。

```c++
  pxr::SdfPathVector paths = {_sceneDelegate->ConvertCachePathToIndexPath(
      _stage->GetPseudoRoot().GetPath())};
```

## path から task を作る

_taskController が paths を受け取って task 化する

## task をレンダリングする

```c++
HD_API
void Execute(HdRenderIndex *index, HdTaskSharedPtrVector *tasks);
```

```c++
_engine->Execute(_renderIndex.get(), &tasks);
```
