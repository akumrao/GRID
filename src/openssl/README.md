
Steps to build openssl for android and arm64 on linux64

run my-build.sh

NDK OS Variant	Host Tag
macOS	darwin-x86_64
Linux	linux-x86_64
64-bit Windows	windows-x86_64

ABI	Triple
armeabi-v7a	armv7a-linux-androideabi
arm64-v8a	aarch64-linux-android
x86	i686-linux-android
x86-64	x86_64-linux-android

















# openssl for  android

Compile openssl and curl for Android

## Prerequisites

Make sure you have `Android NDK` installed.

You may also need to install `autoconf` and `libtool` toolchains as well as build essentials.

```

## Usage

```bash
git clone https://github.com/robertying/openssl-curl-android.git
cd openssl-curl-android
git submodule update --init --recursive

export NDK=your_android_ndk_root_here # e.g. $HOME/Library/Android/sdk/ndk/23.0.7599858
export HOST_TAG=see_this_table_for_info # e.g. darwin-x86_64, see https://developer.android.com/ndk/guides/other_build_systems#overview
export MIN_SDK_VERSION=23 # or any version you want

chmod +x ./build.sh
./build.sh
```

All compiled libs are located in `build/openssl` and `build/curl` directory.

Use NDK to link those libs, part of `Android.mk` example:

```makefile
include $(CLEAR_VARS)
LOCAL_MODULE := curl
LOCAL_SRC_FILES := build/curl/$(TARGET_ARCH_ABI)/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)
```

include $(CLEAR_VARS)
LOCAL_MODULE := ssl
LOCAL_SRC_FILES := libs/openssl/$(TARGET_ARCH_ABI)/lib/libssl.a
LOCAL_EXPORT_CFLAGS := -I$(LOCAL_PATH)/libs/openssl/$(TARGET_ARCH_ABI)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := crypto
LOCAL_SRC_FILES := libs/openssl/$(TARGET_ARCH_ABI)/lib/libcrypto.a
LOCAL_EXPORT_CFLAGS := -I$(LOCAL_PATH)/libs/openssl/$(TARGET_ARCH_ABI)/include
include $(PREBUILT_STATIC_LIBRARY)


