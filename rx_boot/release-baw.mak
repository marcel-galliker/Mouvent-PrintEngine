#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Release-baw

#Toolchain
CC := /opt/poky/1.5/sysroots/i686-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc
CXX := /opt/poky/1.5/sysroots/i686-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++
LD := $(CXX)
AR := /opt/poky/1.5/sysroots/i686-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-ar
OBJCOPY := /opt/poky/1.5/sysroots/i686-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-objcopy

#Additional flags
PREPROCESSOR_MACROS := NDEBUG RELEASE linux baw
INCLUDE_DIRS := ../rx_common_lib
LIBRARY_DIRS := 
LIBRARY_NAMES := 
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 

CFLAGS := -ggdb -ffunction-sections -O3 -pthread
CXXFLAGS := -ggdb -ffunction-sections -O3 -pthread
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections -pthread
COMMONFLAGS := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
IS_LINUX_PROJECT := 1
