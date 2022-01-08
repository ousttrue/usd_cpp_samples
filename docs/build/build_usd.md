# build_usd.py

* Windows10(64bit)
* `C:/Program Files/LLVM`
* vc build tools 2019
* cmake
* vswhere
* nasm

```{toctree}
dependencies/index
```

## minimum

```
PS usd_cpp_samples> build_scripts/build_usd.py 
--build-variant debug
'--no-tests' '--no-examples' '--no-tutorials' '--no-tools' '--no-docs' '--no-python' '--prefer-safety-over-speed' '--no-debug-python' '--no-imaging' '--no-ptex' '--no-openvdb' '--no-usdview' '--no-embree' '--no-prman' '--no-openimageio' '--no-opencolorio' '--no-alembic' '--no-hdf5' '--no-draco' '--no-materialx' build_usd

Building with settings:
  USD source directory          usd_cpp_samples
  USD install directory         usd_cpp_samples\usd
  3rd-party source directory    usd_cpp_samples\usd\src
  3rd-party install directory   usd_cpp_samples\usd
  Build directory               usd_cpp_samples\usd\build
  CMake generator               Default
  CMake toolset                 Default
  Downloader                    curl

  Building                      Shared libraries
    Variant                     Debug
    Imaging                     Off
      Ptex support:             Off
      OpenVDB support:          Off
      OpenImageIO support:      Off
      OpenColorIO support:      Off
      PRMan support:            Off
    UsdImaging                  Off
      usdview:                  Off
    Python support              Off
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

  Dependencies                  zlib, boost, TBB
  ```

## cmake

```py
import cmake
os.environ['PATH'] = f'{cmake.CMAKE_BIN_DIR};{os.environ["PATH"]}'
```

## vswhere

* <https://pypi.org/project/vswhere/>
* <https://github.com/microsoft/vswhere/wiki/Find-VC>

```py
def GetVisualStudioCompilerAndVersion():
    """Returns a tuple containing the path to the Visual Studio compiler
    and a tuple for its version, e.g. (14, 0). If the compiler is not found
    or version number cannot be determined, returns None."""
    if not Windows():
        return None

    import vswhere
    latest = vswhere.get_latest(products='*')
    match latest:
        case {
            'installationVersion': version,
            'installationPath': path,
        }:
            version = version.split('.')
            msvcCompiler = f'{path}\\VC\\Tools\\MSVC\\{version[0]}\\bin\\HostX64\\x64\\cl.exe'
            return msvcCompiler, tuple(int(v) for v in version[:2])
```
