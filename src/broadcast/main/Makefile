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


#LIVE555_ROOT = /workspace/live
#FFMPEG_ROOT = /workspace/ffmpeg
#WEBRTC_PATH = /data/delete/mediaserverdocker/webrtc_android/src/
WEBRTC_PATH = /workspace/webrtc/src/


TARGET=runWebrtc # define the name of the executable
SOURCES=$(wildcard *.cpp ../src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../base/token/*.cpp ../../net/src/*.cpp ../../http/src/*.cpp ../../http_parser/*.cpp ../../signal/src/*.cpp ../../facedetec/src/configuration.cpp )

CXXFLAGS   := -g -fno-rtti -std=c++11 -Wno-inconsistent-missing-override -Wno-uninitialized -DNDEBUG -fpermissive

CPPFLAGS   := -I../../facedetec/include/ -I/workspace/ffmpeg/  -I../include -I../../signal/include -I../../base/token/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ -I../webrtc/include  -DOPENSSL_IS_BORINGSSL -DWEBRTC_POSIX  -DWEBRTC_INCLUDE_DIRS=/workspace/webrtc_android/src   \
               -I${WEBRTC_PATH} -I${WEBRTC_PATH}/third_party/abseil-cpp -I../xailient-sdk/include/ -I/workspace/MagicAI/src/openssl/build/include -I${T31SDK}/include -I${T31SDK}/samples/libimp-sample/ -DHTTPSSL

#LDFLAGS   := -L/export/views/video/ffmpeg/Obj/

LDFLAGS +=  -L../xailient-sdk/lib -L${INSTALLPATH}/mips-linux-gnu/lib -muclibc


LDLIBS    := -lpthread -lm -ldl -lz -lfdk-aac -L/workspace/MagicAI/src/webrtc/build -ljingle_peerconnection_so

SLIBS     := ${FFMPEG_ROOT}/libavformat/libavformat.a  /workspace/MagicAI/src/webrtc/build/libssl.a /workspace/MagicAI/src/webrtc/build/libcrypto.a  ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libswresample/libswresample.a ${LIVE555_ROOT}/liveMedia/libliveMedia.a  ${LIVE555_ROOT}/groupsock/libgroupsock.a   ${LIVE555_ROOT}/BasicUsageEnvironment/libBasicUsageEnvironment.a  ${LIVE555_ROOT}/UsageEnvironment/libUsageEnvironment.a /usr/local/lib/libx264.a



################################################################################
### DO NOT EDIT THE FOLLOWING LINES ############################################


# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJS) $(SLIBS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

clean:
	rm -f  $(OBJS)

################################################################################
################################################################################
