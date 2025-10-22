#!/bin/bash

#export NDK=$HOME/Android/Sdk/ndk/21.0.6113669
export NDK=$HOME/Android/Sdk/ndk/25.1.8937393

export HOST_TAG=linux-x86_64
#export HOST_TAG=darwin-x86_64

export MIN_SDK_VERSION=23

export CFLAGS="-Os"
export LDFLAGS="-Wl,-Bsymbolic"

./build.sh
