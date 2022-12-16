# libtt

Is a library for Type Theory

# How to build using devbox

First you need to build the devbox docker image via:

```
$ ./test/devbox/build-image.sh
```

Then you can run conan and cmake as normal within a devbox environment:

```
$ ./test/devbox/devbox.sh
...$ conan install . --install-folder build
...$ conan profile update settings.compiler.libcxx=libstdc++11 default
...$ cmake . -Bbuild -DCMAKE_MODULE_PATH=$PWD/build
...$ cmake --build build
...$ ctest --test-dir build -VV
...$ exit
```

Alteernatively:

```
$ ./test/devbox/devbox.sh conan install . --install-folder build
$ ./test/devbox/devbox.sh conan profile update settings.compiler.libcxx=libstdc++11 default
$ ./test/devbox/devbox.sh cmake . -Bbuild -DCMAKE_MODULE_PATH=$PWD/build
$ ./test/devbox/devbox.sh cmake --build build
$ ./test/devbox/devbox.sh ctest --test-dir build -VV
```
