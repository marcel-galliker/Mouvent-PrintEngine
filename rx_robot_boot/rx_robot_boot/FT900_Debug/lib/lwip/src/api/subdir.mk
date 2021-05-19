################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/lwip/src/api/api_lib.c \
../lib/lwip/src/api/api_msg.c \
../lib/lwip/src/api/err.c \
../lib/lwip/src/api/netbuf.c \
../lib/lwip/src/api/netdb.c \
../lib/lwip/src/api/netifapi.c \
../lib/lwip/src/api/sockets.c \
../lib/lwip/src/api/tcpip.c 

OBJS += \
./lib/lwip/src/api/api_lib.o \
./lib/lwip/src/api/api_msg.o \
./lib/lwip/src/api/err.o \
./lib/lwip/src/api/netbuf.o \
./lib/lwip/src/api/netdb.o \
./lib/lwip/src/api/netifapi.o \
./lib/lwip/src/api/sockets.o \
./lib/lwip/src/api/tcpip.o 

C_DEPS += \
./lib/lwip/src/api/api_lib.d \
./lib/lwip/src/api/api_msg.d \
./lib/lwip/src/api/err.d \
./lib/lwip/src/api/netbuf.d \
./lib/lwip/src/api/netdb.d \
./lib/lwip/src/api/netifapi.d \
./lib/lwip/src/api/sockets.d \
./lib/lwip/src/api/tcpip.d 


# Each subdirectory must supply rules for building sources it contributes
lib/lwip/src/api/%.o: ../lib/lwip/src/api/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: FT9xx GCC Compiler'
	ft32-elf-gcc -D__FT900__ -DDEBUG -I"C:/Program Files (x86)/Bridgetek/FT9xx Toolchain/Toolchain/hardware/include" -I"../Includes" -I"../lib" -I"../lib/lwip/src/arch" -I"../lib/lwip/src/include" -I../../../rx_robot_ctrl/includes -Og -g -fvar-tracking -fvar-tracking-assignments -Wall -c -fmessage-length=0 -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


