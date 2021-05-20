################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include lib/lwip/src/netif/subdir.mk
-include lib/lwip/src/core/ipv4/subdir.mk
-include lib/lwip/src/core/subdir.mk
-include lib/lwip/src/arch/subdir.mk
-include lib/lwip/src/api/subdir.mk
-include Sources/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
rx_robot_boot.bin \

SECONDARY_SIZE += \
SIZE \


# All Target
all: rx_robot_boot.elf secondary-outputs

# Tool invocations
rx_robot_boot.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: FT9xx GCC Linker'
	ft32-elf-gcc -L"C:/Program Files (x86)/Bridgetek/FT9xx Toolchain/Toolchain/hardware/lib/Debug" -Wl,--gc-sections -Wl,--entry=_start -o "rx_robot_boot.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

rx_robot_boot.bin: rx_robot_boot.elf
	@echo 'Invoking: FT9xx Flash File Generator'
	ft32-elf-objcopy --output-target binary "rx_robot_boot.elf"  "rx_robot_boot.bin"
	@echo 'Finished building: $@'
	@echo ' '

SIZE: rx_robot_boot.elf
	@echo 'Invoking: FT9xx Display Image Size'
	ft32-elf-size --format=berkeley -x "rx_robot_boot.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(C_DEPS) rx_robot_boot.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
