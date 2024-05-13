T31SDK =/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/5.4.0

INSTALLPATH=/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0
#export INSTALLPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_472/mips-gcc472-glibc216-64bit"
TOOLCHAIN=${INSTALLPATH}/bin
CROSS_COMPILE=${TOOLCHAIN}/mips-linux-gnu-
CC=${CROSS_COMPILE}gcc
CXX=${CROSS_COMPILE}g++
LD=${CROSS_COMPILE}ld
AR=${CROSS_COMPILE}ar
STRIP=${CROSS_COMPILE}strip
NM=${CROSS_COMPILE}nm
RANLIB=${CROSS_COMPILE}ranlib
# CFLAGS="-static -O2  -march=mips32r2 -uClibc++  -std=c++11 -I${INSTALLPATH}/include -I${INSTALLPATH}/_install/include/"
# export CPPFLAGS="-static -O2  -uClibc++ -std=c++11 -I${INSTALLPATH}/include -I${INSTALLPATH}/_install/include/ -I${INSTALLPATH}/_install/include/ncursesw/"
# export LDFLAGS="-static -uClibc++ -L${INSTALLPATH}/lib -L${INSTALLPATH}/_install/lib/"



TARGET=runAI # define the name of the executable ./../net/src/*.cpp ../../http/src/*.cpp ../../net/uv_multiplex/src/*.cpp ../../http_parser/*.cpp  


SOURCES=$(wildcard *.cpp ../src/*.cpp ../../base/src/*.cpp ../../base/token/*.cpp  ../../net/src/*.cpp ../../http/src/*.cpp ../../net/uv_multiplex/src/*.cpp ../../http_parser/*.cpp )

#../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp
CXXFLAGS   +=  -O3 -std=c++11  -DNDEBUG -fpermissive

CPPFLAGS   += -I../../facedetec/include/ -I../../base/token/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../net/uv_multiplex/include/  -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser -I../xailient-sdk/include/ -I../../openssl/buildt31/include -I${T31SDK}/include -I${T31SDK}/samples/libimp-sample/

LDFLAGS +=  -L../xailient-sdk/lib -L${INSTALLPATH}/mips-linux-gnu/lib -muclibc

#-L${T31SDK}/lib/uclibc -lalog -limp

LDLIBS    += -lxailient-fi -lpthread -lrt -lm

SLIBS    += ../../libuv/build/libuv_a.a  ../../openssl/buildt31/lib/libssl.a  ../../openssl/buildt31/lib/libcrypto.a ${T31SDK}/lib/uclibc/libalog.a  ${T31SDK}/lib/uclibc/libimp.a


# /workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0/mips-linux-gnu/libc/uclibc/usr/lib/libm.a 

#-uClibc++ -std=c++11  -Wl,-Bstatic -lm -Wl,-Bdynamic -Wl,--dynamic-linker=/lib/ld-uClibc.so.0 


################################################################################
### DO NOT EDIT THE FOLLOWING LINES ############################################


# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(SLIBS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

clean:
	rm -f  $(OBJS)

################################################################################
################################################################################
