
set -e
TOP=$(pwd)
echo "TOP = $TOP"


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
rm -rf openssl/openssl-1.1.1f

if [[ ! -f openssl-1.1.1f.tar.gz ]]; then
    wget 'https://www.openssl.org/source/openssl-1.1.1f.tar.gz'
fi
tar xvf openssl-1.1.1f.tar.gz
mv openssl-1.1.1f openssl
cd openssl

#./Configure linux-mips32 no-async  no-shared no-dso --prefix="$TOP/src/openssl/buildt31"


./Configure linux-mips32 shared  no-asan no-async no-buildtest-c++ no-crypto-mdebug no-crypto-mdebug-backtrace no-devcryptoeng no-dso no-dynamic-engine no-ec_nistp_64_gcc_128 no-egd no-external-tests no-fuzz-afl no-fuzz-libfuzzer no-heartbeats no-md2 no-msan no-rc5 no-sctp no-ssl-trace no-ssl3 no-ssl3-method no-ubsan no-unit-test no-weak-ssl-ciphers no-zlib no-zlib-dynamic \
no-afalgeng no-async no-capieng  no-cms no-comp no-ct \
no-ui-console \
no-bf no-blake2 no-scrypt \
no-ssl-trace no-static-engine no-tests no-ts  no-buildtest-c++ no-crypto-mdebug \
no-dso no-makedepend no-aria no-async no-autoload-config no-blake2 no-bf no-camellia no-cast no-chacha  no-comp no-ct  no-dh no-ec2m no-engine no-filenames no-gost no-idea no-mdc2 no-md4 no-multiblock no-nextprotoneg no-ocsp no-ocb no-poly1305 no-psk no-rc2 no-rc4 no-rmd160 no-seed no-siphash no-sm2 no-sm3 no-sm4 no-srp  no-ts no-ui-console no-whirlpool no-asm -DOPENSSL_SMALL_FOOTPRINT \
no-siphash  no-sm2 no-sm3 no-sm4 no-whirlpool --prefix="$TOP/src/openssl/buildt31"
# no-dsa no-idea no-md4 no-mdc2      \
# no-asan no-async no-buildtest-c++ no-crypto-mdebug   no-dso no-dynamic-engine no-ec_nistp_64_gcc_128 no-egd  no-ubsan     \
# no-dso  no-comp no-ct no-des no-dgram no-dh no-dsa no-ec2m no-engine no-filenames no-gost no-idea 

# no-dgram    \ // it create problem

#no-sock \ // it create problem


make clean
make -j$(nproc)
make -j$(nproc) install

cd ../../


# RTSP_CROSS=$TOOLCHAIN/mips-linux-gnu-

# mkdir -p 3rdparty

# echo "Build ffmpeg"
# cd 3rdparty
# rm -rf ffmpeg
# rm -rf ffmpeg-3.4.10

# if [[ ! -f ffmpeg-3.4.10.tar.xz ]]; then
#     wget 'https://ffmpeg.org/releases/ffmpeg-3.4.10.tar.xz'
# fi
# tar xvf ffmpeg-3.4.10.tar.xz
# mv ffmpeg-3.4.10 ffmpeg
# cd ffmpeg
# ./configure --disable-zlib --target-os=linux --arch=mipsel --cpu=mips32r2 --disable-msa --ranlib="${RTSP_CROSS}ranlib" --nm="${RTSP_CROSS}nm" --ar="${RTSP_CROSS}ar" --cc="${RTSP_CROSS}gcc" --cxx="${RTSP_CROSS}g++" --strip="${RTSP_CROSS}strip" --prefix="$TOP/3rdparty/installt31" --enable-gpl  --cross-prefix=mips-linux-gnu --enable-cross-compile  --enable-version3
# make -j$(nproc)
# make install

# cd ../../

cp /workspace/MagicAI/src/webrtc/patch_t31_full.txt  /workspace/webrtc/src/


cd /workspace/webrtc/src/

git apply /workspace/MagicAI/src/webrtc/patch_t31_full.txt

cd /workspace/MagicAI/src/webrtc

./buildenv540uclibc.sh


cd /workspace/MagicAI/src/broadcast/main

git pull

make -f ./Makefile_t31 -j24

make -f ./Makefile_t31 strip




mkdir /workspace/MagicAI/bin

cp /workspace/MagicAI/src/webrtc/t31/libjingle_peerconnection_so.so    /workspace/MagicAI/bin


cp /workspace/MagicAI/src/webrtc/t31/libjingle_peerconnection_so.so    /workspace/MagicAI/bin

cp /workspace/MagicAI/src/broadcast/main/config.js   /workspace/MagicAI/bin

cp /workspace/MagicAI/src/broadcast/main/config.js   /workspace/MagicAI/bin

cp /workspace/MagicAI/src/broadcast/main/runWebrtc   /workspace/MagicAI/bin





