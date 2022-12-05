# libtt

Is a library for Type Theory

# How to build using devbox

```
$ mkdir build
$ devbox conan install . --install-folder build
$ devbox cmake . -Bbuild -DCMAKE_MODULE_PATH=$PWD/build
$ devbox cmake --build build
$ devbox ctest --test-dir build -VV
```
