################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.c \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.c \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.c \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.c \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.c \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.c \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.c \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.c \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.c \
../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.c 

OBJS += \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.o \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.o \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.o \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.o \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.o \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.o \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.o \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.o \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.o \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.o 

C_DEPS += \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.d \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.d \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.d \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.d \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.d \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.d \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.d \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.d \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.d \
./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/%.o Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/%.su Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/%.cyclo: ../Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/%.c Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Zx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Libraries/ioLibrary_Driver/Ethernet -I../Libraries/ioLibrary_Driver/Ethernet/W6300 -I../Libraries/ioLibrary_Driver/Internet -I../port -I../examples/dhcp_dns -I../examples/loopback -I../examples/udp -I../examples/udp_multicast -I../examples/tcp_server_multi_socket -I../examples/sntp -I../examples/netbios -I../examples/http_server -I../examples/mqtt -I../examples/tftp -I../examples/pppoe -I../examples/network_install -I../examples/upnp -I../Libraries/ioLibrary_Driver/Application/loopback -I../Libraries/ioLibrary_Driver/Application/multicast -I../Libraries/ioLibrary_Driver/Internet/DHCP -I../Libraries/ioLibrary_Driver/Internet/DNS -I../Libraries/ioLibrary_Driver/Internet/SNTP -I../Libraries/ioLibrary_Driver/Internet/TFTP -I../Libraries/ioLibrary_Driver/Internet/MQTT -I../Libraries/ioLibrary_Driver/Internet/SNMP -I../Libraries/ioLibrary_Driver/Internet/httpServer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libraries-2f-ioLibrary_Driver-2f-Internet-2f-MQTT-2f-MQTTPacket-2f-src

clean-Libraries-2f-ioLibrary_Driver-2f-Internet-2f-MQTT-2f-MQTTPacket-2f-src:
	-$(RM) ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.su ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.su ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.su ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.su ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.su ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.su ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.su ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.su ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.su ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.cyclo ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.d ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.o ./Libraries/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.su

.PHONY: clean-Libraries-2f-ioLibrary_Driver-2f-Internet-2f-MQTT-2f-MQTTPacket-2f-src

