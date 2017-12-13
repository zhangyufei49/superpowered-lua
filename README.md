# Superepowered-lua

A luabinding project for [superpowered](https://github.com/superpoweredSDK/Low-Latency-Android-Audio-iOS-Audio-Engine.git)

## Build

### OSX

OSX uses CMake to build.
You should install lua and CMake first.
Then set your paths to CMakeLists.txt. Default use lua5.3 and '/usr/local' as install directory.

```bash
mkdir build
cd build
cmake ..
make
make install
```

Then you can goto the **Run test project** theme.

### Android
TODO

### iOS
TODO

## Run test project

Only has OSX test project.

```bash
cd test
make -f Makefile.osx
```

If build failed you can use the prebuild **run-lua** to testing.

```bash
cd test
./run-lua-prebuild ./test.lua
```


