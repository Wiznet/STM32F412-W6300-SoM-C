################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include port/subdir.mk
-include examples/upnp/subdir.mk
-include examples/udp_multicast/subdir.mk
-include examples/udp/subdir.mk
-include examples/tftp/subdir.mk
-include examples/tcp_server_multi_socket/subdir.mk
-include examples/sntp/subdir.mk
-include examples/pppoe/subdir.mk
-include examples/network_install/subdir.mk
-include examples/netbios/subdir.mk
-include examples/mqtt/subdir.mk
-include examples/loopback/subdir.mk
-include examples/http_server/subdir.mk
-include examples/dhcp_dns/subdir.mk
-include Libraries/ioLibrary_Driver/Internet/httpServer/subdir.mk
-include Libraries/ioLibrary_Driver/Internet/TFTP/subdir.mk
-include Libraries/ioLibrary_Driver/Internet/SNTP/subdir.mk
-include Libraries/ioLibrary_Driver/Internet/SNMP/subdir.mk
-include Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/subdir.mk
-include Libraries/ioLibrary_Driver/Internet/MQTT/subdir.mk
-include Libraries/ioLibrary_Driver/Internet/DNS/subdir.mk
-include Libraries/ioLibrary_Driver/Internet/DHCP/subdir.mk
-include Libraries/ioLibrary_Driver/Ethernet/W6300/subdir.mk
-include Libraries/ioLibrary_Driver/Ethernet/W6100/subdir.mk
-include Libraries/ioLibrary_Driver/Ethernet/W5500/subdir.mk
-include Libraries/ioLibrary_Driver/Ethernet/W5300/subdir.mk
-include Libraries/ioLibrary_Driver/Ethernet/W5200/subdir.mk
-include Libraries/ioLibrary_Driver/Ethernet/W5100S/subdir.mk
-include Libraries/ioLibrary_Driver/Ethernet/W5100/subdir.mk
-include Libraries/ioLibrary_Driver/Ethernet/subdir.mk
-include Libraries/ioLibrary_Driver/Application/multicast/subdir.mk
-include Libraries/ioLibrary_Driver/Application/loopback/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := STM32F412-W6300-SoM-C
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
STM32F412-W6300-SoM-C.elf \

MAP_FILES += \
STM32F412-W6300-SoM-C.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
STM32F412-W6300-SoM-C.list \


# All Target
all: main-build

# Main-build Target
main-build: STM32F412-W6300-SoM-C.elf secondary-outputs

# Tool invocations
STM32F412-W6300-SoM-C.elf STM32F412-W6300-SoM-C.map: $(OBJS) $(USER_OBJS) C:\Users\Grace\STM32CubeIDE\workspace_1.15.1\STM32F412-W6300-SoM-C\STM32F412ZGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "STM32F412-W6300-SoM-C.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\Grace\STM32CubeIDE\workspace_1.15.1\STM32F412-W6300-SoM-C\STM32F412ZGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="STM32F412-W6300-SoM-C.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

STM32F412-W6300-SoM-C.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "STM32F412-W6300-SoM-C.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) STM32F412-W6300-SoM-C.elf STM32F412-W6300-SoM-C.list STM32F412-W6300-SoM-C.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
