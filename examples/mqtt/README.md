# MQTT Publish & Subscribe Example

## Overview

MQTT client for the STM32F412 + W6300 SoM. Connects to an MQTT broker, subscribes to a topic, and periodically publishes a message. Based on the WIZnet-PICO-C mqtt publish_subscribe example.

Uses the Paho MQTT Embedded C library included in ioLibrary_Driver.

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to the same network as the MQTT broker)

## Software

- [Mosquitto](https://mosquitto.org/) MQTT broker running on a PC

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1      | MQTT  |

## How to Use

1. Define `EXAMPLE_MQTT` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_MQTT
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. Set the MQTT broker IP in `app_main.c`:

```c
static uint8_t g_mqtt_broker_ip[4] = {192, 168, 11, 2};
```

4. The CubeIDE project already includes the ioLibrary MQTT source. If you
recreate the project, add `Libraries/ioLibrary_Driver/Internet/MQTT` to the
source and include paths.

5. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP and MQTT timing.

6. Start the Mosquitto broker on your PC:

```bash
mosquitto -c mosquitto.conf -v
```

7. Build, flash, and open a serial terminal (115200 bps).

8. Subscribe to the publish topic from another terminal to see messages:

```bash
mosquitto_sub -h localhost -t publish_topic
```

9. Publish to the subscribe topic to send messages to the board:

```bash
mosquitto_pub -h localhost -t subscribe_topic -m "Hello W6300"
```

## Expected Output

### Serial Terminal

```
=== MQTT Publish & Subscribe Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 MQTT connected
 Subscribed to 'subscribe_topic'
 Publishing to 'publish_topic' every 10 seconds

 Published
 Published
 [SUB] Hello W6300
```

### Mosquitto Subscriber

```
Hello, World!
Hello, World!
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `g_mqtt_broker_ip` - MQTT broker IP address
- `PORT_MQTT` - Broker port (default: 1883)
- `MQTT_CLIENT_ID` - Client identifier
- `MQTT_USERNAME` / `MQTT_PASSWORD` - Broker credentials
- `MQTT_PUBLISH_TOPIC` / `MQTT_SUBSCRIBE_TOPIC` - Topic names
- `MQTT_PUBLISH_PAYLOAD` - Message content
- `MQTT_PUBLISH_PERIOD` - Publish interval in ms (default: 10000)

## Note

Mosquitto 2.0+ requires explicit authentication configuration. Create a `mosquitto.conf` with:

```
listener 1883
allow_anonymous true
```
