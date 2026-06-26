# UDP Multicast Example

## Overview

UDP Multicast receiver for the STM32F412 + W6300 SoM. The board joins a multicast group and receives data sent to that group using the ioLibrary_Driver `multicast.h` API.

Two modes available (uncomment one in the main loop):

- **multicast_recv** (default) - receive and print multicast data
- **multicast_loopback** - receive and echo back multicast data

Supports both **DHCP** and **static IP**.

> **W6300 Note:** Unlike W5500, W6300 requires the multicast destination hardware address (`setSn_DHAR`) to be set before the socket is opened. This is handled automatically by `multicast_set_dhar()` in this example.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to a network that supports multicast)

## Socket Allocation

| Socket | Usage     |
|--------|-----------|
| 0      | DHCP      |
| 1      | Multicast |

## How to Use

1. Define `EXAMPLE_UDP_MULTICAST` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_UDP_MULTICAST
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. Configure the multicast group in `app_main.c`:

```c
static uint8_t g_multicast_ip[] = {239, 0, 0, 1};
static uint16_t g_multicast_port = 5000;
```

4. The CubeIDE project already includes the ioLibrary multicast source. If you
recreate the project, add `Libraries/ioLibrary_Driver/Application/multicast`
to the source and include paths.

5. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP timeout handling.

6. Build, flash, and test:
   - Open Hercules UDP panel
   - Set destination to `239.0.0.1:5000`
   - Send data; the board prints the received message on the serial terminal

## Expected Output

```
=== UDP Multicast Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 Multicast group : 239.0.0.1:5000

1:Multicast Recv start
1:Opened, UDP Multicast Socket
1:Multicast Group IP - 239.0.0.1
1:Multicast Group Port - 5000

recv size : 11
Hello World
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `g_multicast_ip` - Multicast group address (default: `239.0.0.1`)
- `g_multicast_port` - Multicast group port (default: 5000)
