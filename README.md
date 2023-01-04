# libtt

Is a library for Type Theory

# How to build using devbox

First you need to build the devbox docker image via:

(NB if you're behind a proxy you might need to edit the Dockerfile)

```
$ ./tools/devbox/build-image.sh
```

Then you can run conan and cmake as normal within a devbox environment:

```
$ ./tools/devbox/devbox.sh
...$ conan install . --install-folder build
...$ conan profile update settings.compiler.libcxx=libstdc++11 default
...$ cmake . -Bbuild -DCMAKE_MODULE_PATH=$PWD/build
...$ cmake --build build
...$ ctest --test-dir build -VV
...$ exit
```

Alteernatively:

```
$ ./tools/devbox/devbox.sh conan install . --install-folder build
$ ./tools/devbox/devbox.sh conan profile update settings.compiler.libcxx=libstdc++11 default
$ ./tools/devbox/devbox.sh cmake . -Bbuild -DCMAKE_MODULE_PATH=$PWD/build
$ ./tools/devbox/devbox.sh cmake --build build
$ ./tools/devbox/devbox.sh ctest --test-dir build -VV
```
