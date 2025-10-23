################################################################################
### CUSTOMIZE BELOW HERE #######################################################
workpace="/workspace/MagicAI/src"
CC	= gcc
CXX     = g++
TARGET=runtestSecTcpClient # define the name of the executable
SOURCES=$(wildcard *.cpp  )

CXXFLAGS   := -g  -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized


CPPFLAGS   := -I../../json/include/  -I../../base/include/ -I../../mbedtls/include -I../../net/incmbedtls/  -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ \

              
LDFLAGS   := -L/data/sound/lib -L/usr/lib/x86_64-linux-gnu

LDLIBS    := -lpthread -lm -ldl 

SLIBS     := $(workpace)/mbedtls/build/library/libmbedtls.a $(workpace)/mbedtls/build/library/libmbedx509.a $(workpace)/mbedtls/build/library/libmbedcrypto.a



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

