#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Debug

#Toolchain
CC := gcc
CXX := g++
LD := $(CXX)
AR := ar
OBJCOPY := objcopy

#Additional flags
PREPROCESSOR_MACROS := DEBUG
INCLUDE_DIRS :=../Common
LIBRARY_DIRS :=
LIBRARY_NAMES :=
ADDITIONAL_LINKER_INPUTS :=
MACOS_FRAMEWORKS :=

CFLAGS := -ggdb -ffunction-sections -O0 -pthread
CXXFLAGS := -ggdb -ffunction-sections -O0 -pthread
ASFLAGS :=
LDFLAGS := -Wl,-gc-sections -pthread
COMMONFLAGS :=

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
IS_LINUX_PROJECT := 1
