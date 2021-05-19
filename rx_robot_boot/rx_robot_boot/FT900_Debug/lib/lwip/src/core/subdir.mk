################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/lwip/src/core/def.c \
../lib/lwip/src/core/dns.c \
../lib/lwip/src/core/inet_chksum.c \
../lib/lwip/src/core/init.c \
../lib/lwip/src/core/ip.c \
../lib/lwip/src/core/mem.c \
../lib/lwip/src/core/memp.c \
../lib/lwip/src/core/netif.c \
../lib/lwip/src/core/pbuf.c \
../lib/lwip/src/core/raw.c \
../lib/lwip/src/core/stats.c \
../lib/lwip/src/core/sys.c \
../lib/lwip/src/core/tcp.c \
../lib/lwip/src/core/tcp_in.c \
../lib/lwip/src/core/tcp_out.c \
../lib/lwip/src/core/timeouts.c \
../lib/lwip/src/core/udp.c 

OBJS += \
./lib/lwip/src/core/def.o \
./lib/lwip/src/core/dns.o \
./lib/lwip/src/core/inet_chksum.o \
./lib/lwip/src/core/init.o \
./lib/lwip/src/core/ip.o \
./lib/lwip/src/core/mem.o \
./lib/lwip/src/core/memp.o \
./lib/lwip/src/core/netif.o \
./lib/lwip/src/core/pbuf.o \
./lib/lwip/src/core/raw.o \
./lib/lwip/src/core/stats.o \
./lib/lwip/src/core/sys.o \
./lib/lwip/src/core/tcp.o \
./lib/lwip/src/core/tcp_in.o \
./lib/lwip/src/core/tcp_out.o \
./lib/lwip/src/core/timeouts.o \
./lib/lwip/src/core/udp.o 

C_DEPS += \
./lib/lwip/src/core/def.d \
./lib/lwip/src/core/dns.d \
./lib/lwip/src/core/inet_chksum.d \
./lib/lwip/src/core/init.d \
./lib/lwip/src/core/ip.d \
./lib/lwip/src/core/mem.d \
./lib/lwip/src/core/memp.d \
./lib/lwip/src/core/netif.d \
./lib/lwip/src/core/pbuf.d \
./lib/lwip/src/core/raw.d \
./lib/lwip/src/core/stats.d \
./lib/lwip/src/core/sys.d \
./lib/lwip/src/core/tcp.d \
./lib/lwip/src/core/tcp_in.d \
./lib/lwip/src/core/tcp_out.d \
./lib/lwip/src/core/timeouts.d \
./lib/lwip/src/core/udp.d 


# Each subdirectory must supply rules for building sources it contributes
lib/lwip/src/core/%.o: ../lib/lwip/src/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: FT9xx GCC Compiler'
	ft32-elf-gcc -D__FT900__ -DDEBUG -I"C:/Program Files (x86)/Bridgetek/FT9xx Toolchain/Toolchain/hardware/include" -I"../Includes" -I"../lib" -I"../lib/lwip/src/arch" -I"../lib/lwip/src/include" -I../../../rx_robot_ctrl/includes -Og -g -fvar-tracking -fvar-tracking-assignments -Wall -c -fmessage-length=0 -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


