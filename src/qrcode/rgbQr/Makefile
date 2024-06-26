
CC = gcc
OUT_FILE_NAME = zbar.a

CFLAGS=  -g -Wall -c -DZBAR_WIN=0

INC = -I../zbar -I../decoder  -I../processor  -I../qrcode  -I../video  -I../window -I../include  -I../  

SRCS  = $(wildcard ../zbar/*.c ../decoder/*.c ../processor/*.c ../qrcode/*.c )


OBJ = $(SRCS:.c=.o)

$(OUT_FILE_NAME): $(OBJ)
	@echo "[Link (Static)]"
	@ar rcs $@ $^

.c.o:
	@echo [Compile] $<
	@$(CC) -c $(INC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) 

.PHONY: $(OUT_FILE_NAME)


#all: $(OUT_FILE_NAME)
     





CXX     = g++
TARGET=runQR # define the name of the executable
SOURCES    = $(wildcard *.cpp  )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-attributes -Wno-fpermissive -Wno-deprecated-declarations
 

CPPFLAGS   := -I../../fmp4/include/ -I../zbar 
              
LDFLAGS   := 
#/export/views/video/zbar/libavutil/libavutil.a /export/views/video/zbar/libavcodec/libavcodec.a
LDLIBS    := -lpthread -lm -ldl 

#SLIBS     := $(OUT_FILE_NAME) 

SLIBS     := $(OUT_FILE_NAME)



# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(OUT_FILE_NAME) $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJS) $(SLIBS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)
	rm -f $(OUT_FILE_NAME)
