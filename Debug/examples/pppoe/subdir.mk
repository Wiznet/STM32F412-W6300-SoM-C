################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../examples/pppoe/PPPoE.c \
../examples/pppoe/app_main.c \
../examples/pppoe/md5.c 

OBJS += \
./examples/pppoe/PPPoE.o \
./examples/pppoe/app_main.o \
./examples/pppoe/md5.o 

C_DEPS += \
./examples/pppoe/PPPoE.d \
./examples/pppoe/app_main.d \
./examples/pppoe/md5.d 


# Each subdirectory must supply rules for building sources it contributes
examples/pppoe/%.o examples/pppoe/%.su examples/pppoe/%.cyclo: ../examples/pppoe/%.c examples/pppoe/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Zx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Libraries/ioLibrary_Driver/Ethernet -I../Libraries/ioLibrary_Driver/Ethernet/W6300 -I../Libraries/ioLibrary_Driver/Internet -I../port -I../examples/dhcp_dns -I../examples/loopback -I../examples/udp -I../examples/udp_multicast -I../examples/tcp_server_multi_socket -I../examples/sntp -I../examples/netbios -I../examples/http_server -I../examples/mqtt -I../examples/tftp -I../examples/pppoe -I../examples/network_install -I../examples/upnp -I../Libraries/ioLibrary_Driver/Application/loopback -I../Libraries/ioLibrary_Driver/Application/multicast -I../Libraries/ioLibrary_Driver/Internet/DHCP -I../Libraries/ioLibrary_Driver/Internet/DNS -I../Libraries/ioLibrary_Driver/Internet/SNTP -I../Libraries/ioLibrary_Driver/Internet/TFTP -I../Libraries/ioLibrary_Driver/Internet/MQTT -I../Libraries/ioLibrary_Driver/Internet/SNMP -I../Libraries/ioLibrary_Driver/Internet/httpServer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-examples-2f-pppoe

clean-examples-2f-pppoe:
	-$(RM) ./examples/pppoe/PPPoE.cyclo ./examples/pppoe/PPPoE.d ./examples/pppoe/PPPoE.o ./examples/pppoe/PPPoE.su ./examples/pppoe/app_main.cyclo ./examples/pppoe/app_main.d ./examples/pppoe/app_main.o ./examples/pppoe/app_main.su ./examples/pppoe/md5.cyclo ./examples/pppoe/md5.d ./examples/pppoe/md5.o ./examples/pppoe/md5.su

.PHONY: clean-examples-2f-pppoe

