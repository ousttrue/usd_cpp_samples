# USD の C++ によるサンプル集(Windows)

<https://ousttrue.github.io/usd_cpp_samples/>


## [WIP] mesno builld にしてみた

```
# prefix がフルパスになるようにしてください
> meson setup builddir --prefix $(pwd)/prefix 
# compile でなく install。plugin のフォルダ配置が動作に必要です。
> meson install -C builddir

> ./prefix/bin/hello.exe
```
