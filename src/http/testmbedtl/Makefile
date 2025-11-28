################################################################################
### CUSTOMIZE BELOW HERE  #######################################################
#imp go to ./src/webrtcserver/deps/mbedtls/buildx64.sh and run it before make
CC	= gcc
CXX     = g++
TARGET=runHttp # define the name of the executable
SOURCES=$(wildcard *.cpp ../src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../net/src/*.cpp ../../net/srcmbedtls/*.cpp ../../net/uv_multiplex/src/*.cpp ../../http_parser/*.cpp  ../../base/token/*.cpp  )

CXXFLAGS   := -g  -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized
 

CPPFLAGS   := -I../../webrtcserver/deps/mbedtls/include -I../../mbedtls/ -I../../json/include/  -I../../base/token/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../net/incmbedtls/ -I../../net/uv_multiplex/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ \
-DHTTPSSL=1 -DUSE_MBEDTLS=1
              


LDLIBS    := -lpthread -lm -ldl  

SLIBS     := ../../webrtcserver/deps/mbedtls/build/library/libmbedtls.a ../../webrtcserver/deps/mbedtls/build/library/libmbedx509.a ../../webrtcserver/deps/mbedtls/build/library/libmbedcrypto.a

################################################################################
### DO NOT EDIT THE FOLLOWING LINES ############################################


# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) $(SLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

clean:
	rm -f  $(OBJS)

################################################################################
################################################################################
