#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Release-lx

#Toolchain
CC := gcc
CXX := g++
LD := $(CXX)
AR := ar
OBJCOPY := objcopy

#Additional flags
PREPROCESSOR_MACROS := NDEBUG release linux
INCLUDE_DIRS := ../rx_ink_lib ../rx_common_lib ../Externals/TinyXML ./rx_rexroth_lib ../rx_rip_lib ../rx_tif_lib ../rx_pecore_lib ../Externals/libtiff ../Externals/mlpi/include ../Externals/matrix/linux/include ../Externals/libxl/include_c
LIBRARY_DIRS := /usr/lib ../Externals/libxl/lib64
LIBRARY_NAMES := mlpi xl
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 

CFLAGS := -pthread -ggdb -ffunction-sections -O0 -fshort-wchar
CXXFLAGS := -pthread -ggdb -ffunction-sections -O0 -fshort-wchar
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections -pthread
COMMONFLAGS := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
IS_LINUX_PROJECT := 1
