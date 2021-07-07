# Superepowered-lua

A luabinding project for [superpowered](https://github.com/superpoweredSDK/Low-Latency-Android-Audio-iOS-Audio-Engine.git)

## Support Platform:
* Android
* iOS
* OSX
* Linux

## Build

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

## Run test project

OSX/Linux test project:

```bash
cd test
make
```


There is no test project about Android or iOS for now.
