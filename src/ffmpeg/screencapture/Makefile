### CUSTOMIZE BELOW HERE #######################################################
FFMPEG_ROOT = /usr/local/google/home/aumrao/workspace/ffmpeg

CC	= gcc
CXX     = g++
TARGET=Screencapture # define the name of the executable
SOURCES=$(wildcard *.cpp  )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized 

CPPFLAGS   := -I${FFMPEG_ROOT}/
              
LDFLAGS   := -L/usr/lib/x86_64-linux-gnu


LDLIBS    := -lpthread -lm -ldl -lm -ldl -lz -lX11 -lxcb -lbz2


#avformat -lavcodec -lavutil -lswresample -lswscale -lavdevice -lavutil 

#SLIBS     := ${FFMPEG_ROOT}/libavformat/libavformat.a ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libswresample/libswresample.a ${FFMPEG_ROOT}/libswscale/libswscale.a /usr/local/lib/libx264.a
SLIBS      := ${FFMPEG_ROOT}/libavdevice/libavdevice.a  ${FFMPEG_ROOT}/libavformat/libavformat.a ${FFMPEG_ROOT}/libavfilter/libavfilter.a ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libswresample/libswresample.a  ${FFMPEG_ROOT}/libswscale/libswscale.a ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libpostproc/libpostproc.a /usr/local/lib/libx264.a

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
