
#./configure --host=mips-linux-gnu --disable-database --disable-db-install --with-fallbacks=vt100,vt102,vt300,screen,xterm,xterm-256color,tmux-256color,screen-256color --without-manpages --without-normal --without-progs --without-debug --without-test --enable-widec --prefix=${INSTALLPATH}/_install


#!/usr/bin/env bash

#export INSTALLPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_472/mips-gcc472-glibc216-64bit"
export INSTALLPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_540/mips-gcc540-glibc222-32bit-r3.3.0"
#export SDKPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/4.7.2"
export SDKPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/5.4.0"
TOOLCHAIN=$INSTALLPATH/bin
CROSS_COMPILE=$TOOLCHAIN/mips-linux-gnu-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export AR=${CROSS_COMPILE}ar
export STRIP=${CROSS_COMPILE}strip
export CFLAGS="-Os -fdata-sections -ffunction-sections -muclibc -fPIC -Wno-error -I/workspace/MagicAI/src/openssl/buildt31/include -I${INSTALLPATH}/include -I${INSTALLPATH}/mips-linux-gnu/include/ -I${SDKPATH}/include/"
export CPPFLAGS="-Os -fdata-sections -ffunction-sections -muclibc -fPIC -I/workspace/MagicAI/src/openssl/buildt31/include -I${INSTALLPATH}/include -I${INSTALLPATH}/mips-linux-gnu/include/ -I${SDKPATH}/include/"
export LDFLAGS="-Os -fdata-sections -ffunction-sections -muclibc -L${INSTALLPATH}/lib -L${INSTALLPATH}/mips-linux-gnu/lib/ -L${SDKPATH}/lib/uclibc/"

export JZDL_SDK_FOLDER_INC="/workspace/magik-toolkit/InferenceKit/jzdl/5.4.0/include"
export JZDL_SDK_FOLDER_LIB="/workspace/magik-toolkit/InferenceKit/jzdl/5.4.0/lib/uclibc"


rm -rf t31
mkdir -p t31

cd t31

#cmake -DUSE_MUCLIBC=ON -DBUILD_TESTING=off -DANDROID_ABI=x86_64 ..
cmake -DUSE_MUCLIBC=ON -DBUILD_TESTING=off -DANDROID_ABI=mipsel ..

make -j$(nproc)
