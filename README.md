# USD の C++ によるサンプル集(Windows)

<https://ousttrue.github.io/usd_cpp_samples/>


## [WIP] mesno builld にしてみた

```
# --prefix is fullpath
> meson setup builddir --prefix $(pwd)/prefix 
# install to prefix
> meson install -C builddir

> ./prefix/bin/hello.exe
```

## Imaging

ある程度ビルドできたが動かん。

### OpenSubdiv

- build 難航。

`pxr/imaging/hdSt/codeGen.cpp` で OpenSubdiv の 
shader を得ているところを コメントアウトして誤魔化した。
後回しでよし。

