# USD の C++ によるサンプル集(Windows)

<https://ousttrue.github.io/usd_cpp_samples/>


## [WIP] mesno builld にしてみた

```
# --prefix is fullpath
> meson setup builddir --prefix $(pwd)/prefix 
# install to prefix
> meson install -C builddir

> ./prefix/bin/hello.exe

> ./prefix/bin/gl_sample.exe usd_source/extras/usd/tutorials/authoringVariants/HelloWorld.usda
```

