runConfTest 
./runConfTest: can't load library 'libxailient-fi.so'
 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mnt




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
export CFLAGS="-O2 -muclibc -fPIC -Wno-error -I${INSTALLPATH}/include -I${INSTALLPATH}/mips-linux-gnu/include/ -I${SDKPATH}/include/"
export CPPFLAGS="-O2 -muclibc -fPIC  -I${INSTALLPATH}/include -I${INSTALLPATH}/mips-linux-gnu/include/ I${SDKPATH}/include/"
export LDFLAGS="-O2 -muclibc -L${INSTALLPATH}/lib -L${INSTALLPATH}/mips-linux-gnu/lib/ -L${SDKPATH}/lib/uclibc/"




./Configure linux-mips32 no-async  no-shared no-dso --prefix=/workspace/MagicAI/src/openssl/build/

./Configure linux-x86_64 --prefix=/workspace/MagicAI/src/openssl/buildx64/