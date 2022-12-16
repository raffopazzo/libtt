FROM ubuntu:20.04

# prevent tzdata from hanging
RUN ln -snf /usr/share/zoneinfo/Europe/London /etc/localtime \
 && echo Europe/London > /etc/timezone

# required to install gcc11
# RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test

# install gcc11, cmake and conan
# NB cmake is available via apt but there is also one via pip, which is version 3.25 instead of 3.16
RUN apt-get update \
 && apt-get install -y software-properties-common \
 && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
 && apt-get install -y g++-11 python3-pip \
 && pip3 install conan cmake \
 && apt-get remove -y gcc-9 \
 && update-alternatives \
    --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-11 \
    --slave /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-11 \
    --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-11 \
 && rm -fr /var/lib/apt/lists
