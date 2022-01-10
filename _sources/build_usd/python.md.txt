# python module

    build_usd.py '--build-variant' 'debug' '--no-tests' '--no-examples' '--no-tutorials' '--no-tools' '--no-docs' '--python' '--prefer-safety-over-speed' '--no-debug-python' '--no-ptex' '--no-openvdb' '--no-usdview' '--no-embree' '--no-prman' '--no-openimageio' '--no-opencolorio' '--no-alembic' '--no-hdf5' '--no-draco' '--no-materialx' 'C:\Users\oustt\Desktop\usd_cpp_samples/build_usd' 

    Building with settings:
      USD source directory          usd_cpp_samples\build_scripts\..\_external\usd
      USD install directory         usd_cpp_samples\build_usd
      3rd-party source directory    usd_cpp_samples\build_usd\src
      3rd-party install directory   usd_cpp_samples\build_usd
      Build directory               usd_cpp_samples\build_usd\build
      CMake generator               Default
      CMake toolset                 Default
      Downloader                    curl

      Building                      Shared libraries
        Variant                     Debug
        Imaging                     On
          Ptex support:             Off
          OpenVDB support:          Off
          OpenImageIO support:      Off
          OpenColorIO support:      Off
          PRMan support:            Off
        UsdImaging                  On
          usdview:                  Off
        Python support              On
          Python Debug:             Off
          Python 3:                 On
        Documentation               Off
        Tests                       Off
        Examples                    Off
        Tutorials                   Off
        Tools                       Off
        Alembic Plugin              Off
          HDF5 support:             Off
        Draco Plugin                Off
        MaterialX Plugin            Off

      Dependencies                  None

## boost.python

['b2', '--prefix=usd_cpp_samples\\build_usd', '--build-dir=usd_cpp_samples\\build_usd\\build', '-j16', 'address-model=64', 'link=shared', 'runtime-link=shared', 'threading=multi', 'variant=debug', '--with-atomic', '--with-program_options', '--with-regex', '--with-python', '--user-config=python-config.jam', '-a', 'toolset=msvc-14.2', '--debug-configuration', 'install']'

-   python-3.10 が使いたいので boost のバージョンを最新(1.77)にあげてみる
-   `C:/Python310/Lib/site-packages/cmake/data/share/cmake-3.22/Modules/FindBoost.cmake` に列挙されているうちで最新は `1.77`

build フォルダをクリアしないと boost の置き換えがうまくいかないかも。`clean build` すればうまくいった。

## usdview

build するのに

| pyside             |                 |
| ------------------ | --------------- |
| pyside-uic         | python3 + Qt5 ? |
| python2-pyside-uic | python2 + Qt4 ? |
| pyside-uic-2.7     | python2 + Qt4 ? |

のいずれかがいる。

<https://fereria.github.io/reincarnation_tech/11_Pipeline/01_USD/00_install_USD/>

## python

{doc}`/python/index`