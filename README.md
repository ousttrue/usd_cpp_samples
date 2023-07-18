# USD の C++ によるサンプル集(Windows)

<https://ousttrue.github.io/usd_cpp_samples/>


## mesno builld

using `/subprojects/usd.wrap`

```
# --prefix is fullpath
> meson setup builddir --prefix $(pwd)/prefix 
# install to prefix
> meson install -C builddir

> ./prefix/bin/hello.exe

> ./prefix/bin/gl_sample.exe usd_source/extras/usd/tutorials/authoringVariants/HelloWorld.usda
```

