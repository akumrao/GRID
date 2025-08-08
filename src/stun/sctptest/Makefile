################################################################################
### CUSTOMIZE BELOW HERE #######################################################

CC	= gcc
CXX     = g++
TARGET=sctp
SOURCES=$(wildcard main.cpp Connection.cpp ../src/*.cpp ../sctp/*.cpp ../../base/src/*.cpp  ../../net/src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp )

CXXFLAGS   := -g   -std=c++17 -Wno-inconsistent-missing-override -Wno-uninitialized


CPPFLAGS   := -I./ -I../include -I../sctp/ -I../../base/include/ -I../../net/include/  -I../../libuv/include -I../../libuv/src  -I../../facedetec/include/json/  -I../../webrtcserver/deps/usrsctp/usrsctplib/


              
LDFLAGS   := -L/usr/lib/x86_64-linux-gnu



LDLIBS    := -lpthread -lm -ldl -lssl -lcrypto -lz

#SLIBS     := 


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
