################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/ioLibrary_Driver/Internet/AAC/AddressAutoConfig.c 

OBJS += \
./Libraries/ioLibrary_Driver/Internet/AAC/AddressAutoConfig.o 

C_DEPS += \
./Libraries/ioLibrary_Driver/Internet/AAC/AddressAutoConfig.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/ioLibrary_Driver/Internet/AAC/%.o Libraries/ioLibrary_Driver/Internet/AAC/%.su Libraries/ioLibrary_Driver/Internet/AAC/%.cyclo: ../Libraries/ioLibrary_Driver/Internet/AAC/%.c Libraries/ioLibrary_Driver/Internet/AAC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Zx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Libraries/ioLibrary_Driver/Ethernet -I../Libraries/ioLibrary_Driver/Internet -I../Libraries/ioLibrary_Driver/Application/loopback -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libraries-2f-ioLibrary_Driver-2f-Internet-2f-AAC

clean-Libraries-2f-ioLibrary_Driver-2f-Internet-2f-AAC:
	-$(RM) ./Libraries/ioLibrary_Driver/Internet/AAC/AddressAutoConfig.cyclo ./Libraries/ioLibrary_Driver/Internet/AAC/AddressAutoConfig.d ./Libraries/ioLibrary_Driver/Internet/AAC/AddressAutoConfig.o ./Libraries/ioLibrary_Driver/Internet/AAC/AddressAutoConfig.su

.PHONY: clean-Libraries-2f-ioLibrary_Driver-2f-Internet-2f-AAC

