
CC = gcc
OUT_FILE_NAME = zbar

CFLAGS=  -g -Wall -c -DZBAR_WIN=1 -DZBAR_VID=1

TARGETS=$(OUT_FILE_NAME) $(TARGET)

INC = -I../zbar -I../decoder  -I../processor  -I../qrcode  -I../video  -I../window -I../include  -I../

OBJ_DIR=../zbar

OUT_DIR=./

# Enumerating of every *.cpp as *.o and using that as dependency.	
# filter list of .c files in a directory.
# FILES =dump_l.c \
#	kter.c \
#
# $(OUT_FILE_NAME): $(patsubst %.c,$(OBJ_DIR)/%.o,$(wildcard $(FILES))) 


.DEFAULT_GOAL=all
.PHONY: all
all: $(TARGETS) main




# Enumerating of every *.cpp as *.o and using that as dependency ../window/*.c 
$(OUT_FILE_NAME): $(patsubst %.c,%.o,$(wildcard ../zbar/*.c ../decoder/*.c ../processor/*.c ../qrcode/*.c ../video/*.c  ../window/*.c  ))
	ar -r -o $(OUT_DIR)/$@ $^



#Compiling every *.cpp to *.o
%.o: %.c 
	$(CC) -c $(INC) $(CFLAGS) -o $@  $<
	
dirmake:
	@mkdir -p $(OUT_DIR)
	
# clean:
# 	rm -f $(OBJ_DIR)/*.o $(OUT_DIR)/$(OUT_FILE_NAME) Makefile.bak
# 	rm -f  $(OBJS)

# rebuild: clean buil






SLIBS = $(OUT_FILE_NAME)



CC=gcc
CFLAGSM=$(INC) -g -DZBAR_WIN=1 -DZBAR_VID=1

ODIR=obj
LDIR =../lib

LIBS=-lm -lpthread -lv4l2 -lX11 -lXv 


main: main.c zbar
	gcc -o main main.c $(CFLAGSM)  $(SLIBS) $(LIBS)


clean:
	rm -f  $(OBJS)
	rm -f $(ODIR)/*.o 
	rm -f $(OUT_FILE_NAME)
	rm -f main
	rm -f $(OBJ_DIR)/*.o $(OUT_DIR)/$(OUT_FILE_NAME) Makefile.bak
	rm -f  main.o



# CXX     = g++
# TARGET=runfmp4 # define the name of the executable
# SOURCES    = $(wildcard *.cpp  )

# CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-attributes -Wno-fpermissive -Wno-deprecated-declarations
 

# CPPFLAGS   := -I../../fmp4/include/ -I../zbar 
              
# LDFLAGS   := -L/export/views/video/zbar/Obj/
# #/export/views/video/zbar/libavutil/libavutil.a /export/views/video/zbar/libavcodec/libavcodec.a
# LDLIBS    := -lpthread -lm -ldl -lssl -lcrypto -lfdk-aac

# #SLIBS     := $(OUT_FILE_NAME) 

# SLIBS     := $(OUT_FILE_NAME)



# # define list of objects
# OBJSC=$(SOURCES:.cpp=.o)
# OBJS=$(OBJSC:.cpp=.o)

# # the target is obtained linking all .o files
# all: $(SOURCES) $(TARGET)

# $(TARGET): $(OBJS)
# 	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJS) $(SLIBS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

# purge: clean
# 	rm -f $(TARGET)
# 	rm -f $(OUT_FILE_NAME)
