
T31SDK =/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/5.4.0
INSTALLPATH=/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0
#export INSTALLPATH="/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_472/mips-gcc472-glibc216-64bit"



#
# Makefile for cross-compiling t31 object detection sdk
# example: INSTALLPATH=/third_party/tools/cross_compile/mips-gcc540-glibc222-32bit-r3.3.0 JZDL_SDK_FOLDER=/xailient-sdks/sdks/embedded-sdk/dependencies/hw/ingenic/jzdl/MIPS32/5.4.0/ make
#

.PHONY: all clean

# The customer is required to:
# - Set the INSTALLPATH environment variable to the path of the cross-compiler toolchain
# - Set the JZDL_SDK_FOLDER environment variable to the path of the JZDL SDK

LIBTYPE ?= muclibc

# ifndef JZDL_SDK_FOLDER
# 	$(error Expected environment variable JZDL_SDK_FOLDER is not set)
# endif

JZDL_SDK_FOLDER =/workspace/magik-toolkit/InferenceKit/jzdl/5.4.0

ifeq ($(LIBTYPE), muclibc)
LIB_PREFIX := uclibc
else
LIB_PREFIX := glibc
endif

# depending if we are cross compiling or not we set flags differently
ifndef INSTALLPATH
	CXX = g++
	AR = ar
	STRIP = strip

	# Compiler and linker flags
	CXXFLAGS := -D_GLIBCXX_USE_CXX11_ABI=0 -Wall -O3 -lpthread -lrt -ldl -std=gnu++11

else
	CXX = $(INSTALLPATH)/bin/mips-linux-gnu-g++
	AR = $(INSTALLPATH)/bin/mips-linux-gnu-ar
	STRIP = $(INSTALLPATH)/bin/mips-linux-gnu-strip

	# Compiler and linker flags
	CXXFLAGS := -O3 -std=c++11  -DNDEBUG -fpermissive
	#-D_GLIBCXX_USE_CXX11_ABI=0 -Wall -EL -O3 -march=mips32r2 -mmxu2 -flax-vector-conversions -lpthread -lrt -ldl -std=gnu++14

endif


ifeq ($(LIBTYPE), muclibc)
	CXXFLAGS += -muclibc
endif

# jzdl SDK includes
CXXFLAGS += -I $(JZDL_SDK_FOLDER)/include
CXXFLAGS += -I ../include

# Link against Xailent SDK library
LDFLAGS += -L../lib -lxailientsdk -lxailientsdkutils -muclibc
LDFLAGS += -L$(JZDL_SDK_FOLDER)/lib/$(LIB_PREFIX)

# Standard libraries
LDFLAGS += -ldl -lrt -lm -lpthread -ljzdl.m

# Target binary
TARGET = ref_app

# Source files
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default rule
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

strip: 
	${STRIP}  $(TARGET)



