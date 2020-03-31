#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Debug-soc

#Toolchain
CC := $(TOOLCHAIN_ROOT)/bin/arm-linux-gnueabihf-gcc.exe
CXX := $(TOOLCHAIN_ROOT)/bin/arm-linux-gnueabihf-g++.exe
LD := $(CXX)
AR := $(TOOLCHAIN_ROOT)/bin/arm-linux-gnueabihf-ar.exe
OBJCOPY := $(TOOLCHAIN_ROOT)/bin/arm-linux-gnueabihf-objcopy.exe

#Additional flags
PREPROCESSOR_MACROS := DEBUG linux soc
INCLUDE_DIRS := ../rx_ink_lib ../rx_common_lib
LIBRARY_DIRS := 
LIBRARY_NAMES := 
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 

CFLAGS := -ggdb -ffunction-sections -O0 -pthread -lrt
CXXFLAGS := -ggdb -ffunction-sections -O0 -pthread -lrt
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections -pthread -lrt
COMMONFLAGS := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
USE_DEL_TO_CLEAN := 1
CP_NOT_AVAILABLE := 1
IS_LINUX_PROJECT := 1
