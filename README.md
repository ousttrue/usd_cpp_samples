# USD の C++ によるサンプル集(Windows)

## USD BUILD のインストール

Debug

```shell
> python build_scripts\build_usd.py --debug --build-monolithic --no-tests --no-examples --no-tutorials --no-docs --no-python --usd-imaging %USD_BUILD_DEBUG%
```

Release

```shell
> python build_scripts\build_usd.py --build-monolithic --no-tests --no-examples --no-tutorials --no-docs --no-python --usd-imaging %USD_BUILD_RELEASE%
```
