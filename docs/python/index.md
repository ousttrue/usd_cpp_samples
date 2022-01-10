# python の usd モジュール

* <https://graphics.pixar.com/usd/release/tut_referencing_layers.html>
* [Working with USD Python Libraries](https://developer.nvidia.com/usd/tutorials)

```{toctree}
impl
```

* <https://developer.nvidia.com/usd/tutorials>

```python
from pxr import Usd

stage = Usd.Stage.Open('path_to_file.usd')
```

* <https://graphics.pixar.com/usd/release/tut_usd_tutorials.html>


## ビルド済み

* (python-3.6)<https://developer.nvidia.com/usd>
* (python-3.9)<https://pypi.org/project/usd-core/>

## usdview

* <https://github.com/PixarAnimationStudios/USD/tree/release/pxr/usdImaging/usdviewq>

```py
from pxr import Usd

stage = Usd.Stage.Open('path_to_file.usd')
```

## typings

適当でも作っておくと便利。
