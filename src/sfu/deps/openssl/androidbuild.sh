#!/bin/bash

export NDK=/root/Android/Sdk/ndk/21.4.7075529
export HOST_TAG=linux-x86_64 # e.g. darwin-x86_64, see https://developer.android.com/ndk/guides/other_build_systems#overview
export MIN_SDK_VERSION=23

chmod +x ./build-openssl.sh
./build-openssl.sh

