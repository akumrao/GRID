################################################################################
### CUSTOMIZE BELOW HERE #######################################################

CC	= gcc
CXX     = g++
TARGET=runUnitTest # define the name of the executable
SOURCES=$(wildcard testDtlsServer.cpp ../src/*.cpp ../dtls/*.cpp ../sctp/*.cpp  ../../json/src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp )

CXXFLAGS   := -g -std=c++17 -Wno-inconsistent-missing-override -Wno-uninitialized -DHTTPSSL

CPPFLAGS   := -I../../json/include/ -I../sctp/  -I../dtls/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ -I../../../src/openssl/buildx64/include -I../../webrtcserver/deps/usrsctp/usrsctplib/



LDLIBS    := -lpthread -lm -ldl

SLIBS     := ../../../src/openssl/buildx64/lib/libssl.a   ../../../src/openssl/buildx64/lib/libcrypto.a  ../../webrtcserver/build/deps/usrsctp/usrsctplib/libusrsctp.a


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
