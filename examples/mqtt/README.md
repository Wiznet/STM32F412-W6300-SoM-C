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
static uint8_t g_mqtt_broker_ip[4] = {192, 168, 11, 100};
```

> With `NETINFO_STATIC` the broker address must differ from `g_net_info.ip`
> (`192.168.11.2` by default). If both are the same the board and the broker
> claim one address and the connection never completes.

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
- `MQTT_KEEP_ALIVE` - Keep-alive interval in seconds (default: 60)
- `MQTT_YIELD_TIMEOUT` - How long one `MQTTYield()` waits for an inbound packet, in ms (default: 100)
- `MQTT_RECONNECT_DELAY` - Delay between reconnect attempts in ms (default: 5000)

## Reconnection

The example keeps the session alive on its own. Each loop iteration checks the
socket state, and a lost connection, a yield error, or a failed publish closes
the socket and retries `MQTTConnect` + `MQTTSubscribe` every
`MQTT_RECONNECT_DELAY` ms:

```
 Connection lost
 Network connect failed
 Retry in 5 seconds
 MQTT connected
 Subscribed to 'subscribe_topic'
```

The socket state has to be checked explicitly because the Paho embedded client
reports a closed socket as "no packet arrived" rather than as an error.

DHCP keeps running too. After the initial lease the main loop services
`DHCP_run()` once per second so the lease is renewed; if the server hands out a
different address the MQTT session is torn down and reopened from the new one:

```
 DHCP IP changed
 MQTT connected
```

If the *initial* lease fails after `DHCP_RETRY_COUNT` attempts the example still
stops, as in the other examples here.

## Note

Mosquitto 2.0+ requires explicit authentication configuration. Create a `mosquitto.conf` with:

```
listener 1883
allow_anonymous true
```
