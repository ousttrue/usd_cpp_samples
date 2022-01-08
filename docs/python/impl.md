# 実装

`build_usd.py` で `--python` するとビルドされて、 `BUILD_DIR/lib/python` フォルダん展開される。

```
pxr
  __init__.py
  Usd/
    __init__.py
    _usd.pyd
```

のように展開される。

`pyd` は usd 関連の `dll` に依存するので、 `BUILD_DIR/bin` と `BUILD_DIR/lib` に追加で PATH を通す必要があるだろう。

```python
import pathlib
import sys
import os
HERE = pathlib.Path(__file__).absolute().parent
BUILD_DIR = HERE.parent / 'build_release'
PY_USD_DIR = BUILD_DIR / 'lib/python'
BIN_DIR0 = BUILD_DIR / 'bin'
BIN_DIR1 = BUILD_DIR / 'lib'

os.environ['PATH'] = f'{BIN_DIR0};{BIN_DIR1};{os.environ["PATH"]}'
sys.path.append(str(PY_USD_DIR))
```

## CMakeLists.txt

`PYMODULE_CPPFILES`

というところにソースファイルが列挙されている。

