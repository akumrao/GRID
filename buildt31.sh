
set -e
TOP=$(pwd)
echo "TOP = $TOP"


#./configure --host=mips-linux-gnu --disable-database --disable-db-install --with-fallbacks=vt100,vt102,vt300,screen,xterm,xterm-256color,tmux-256color,screen-256color --without-manpages --without-normal --without-progs --without-debug --without-test --enable-widec --prefix=${INSTALLPATH}/_install


#!/usr/bin/env bash

#export INSTALLPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_472/mips-gcc472-glibc216-64bit"
export INSTALLPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0"
#export SDKPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/4.7.2"
export SDKPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/5.4.0"
TOOLCHAIN=$INSTALLPATH/bin
CROSS_COMPILE=$TOOLCHAIN/mips-linux-gnu-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export AR=${CROSS_COMPILE}ar
export CFLAGS="-Os -fdata-sections -ffunction-sections -muclibc -fPIC -Wno-error -I${INSTALLPATH}/include -I${INSTALLPATH}/mips-linux-gnu/include/ -I${SDKPATH}/include/"
export CPPFLAGS="-Os -fdata-sections -ffunction-sections  -muclibc -fPIC  -I${INSTALLPATH}/include -I${INSTALLPATH}/mips-linux-gnu/include/ -I${SDKPATH}/include/"
export LDFLAGS="-Os -fdata-sections -ffunction-sections -muclibc -L${INSTALLPATH}/lib -L${INSTALLPATH}/mips-linux-gnu/lib/ -L${SDKPATH}/lib/uclibc/"


mkdir -p src/libuv/build
cd  src/libuv/build

cmake .. -DUSE_MUCLIBC=ON
make -j8


cd ../../../

cd src/openssl

rm -rf openssl/buildt31
rm -rf openssl/openssl
rm -rf openssl/openssl-1.1.1t

if [[ ! -f openssl-1.1.1t.tar.gz ]]; then
    wget 'https://www.openssl.org/source/openssl-1.1.1t.tar.gz'
fi
tar xvf openssl-1.1.1t.tar.gz
mv openssl-1.1.1t openssl
cd openssl

./Configure linux-mips32 no-async  no-shared no-dso --prefix="$TOP/src/openssl/buildt31"

make clean
make -j$(nproc)
make -j$(nproc) install

cd ../../


RTSP_CROSS=$TOOLCHAIN/mips-linux-gnu-

mkdir -p 3rdparty

echo "Build ffmpeg"
cd 3rdparty
rm -rf ffmpeg
rm -rf ffmpeg-3.4.10

if [[ ! -f ffmpeg-3.4.10.tar.xz ]]; then
    wget 'https://ffmpeg.org/releases/ffmpeg-3.4.10.tar.xz'
fi
tar xvf ffmpeg-3.4.10.tar.xz
mv ffmpeg-3.4.10 ffmpeg
cd ffmpeg
./configure --disable-zlib --target-os=linux --arch=mipsel --cpu=mips32r2 --disable-msa --ranlib="${RTSP_CROSS}ranlib" --nm="${RTSP_CROSS}nm" --ar="${RTSP_CROSS}ar" --cc="${RTSP_CROSS}gcc" --cxx="${RTSP_CROSS}g++" --strip="${RTSP_CROSS}strip" --prefix="$TOP/3rdparty/installt31" --enable-gpl  --cross-prefix=mips-linux-gnu --enable-cross-compile  --enable-version3
make -j$(nproc)
make install

cd ../../

