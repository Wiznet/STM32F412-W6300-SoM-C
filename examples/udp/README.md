# UDP Example

## Overview

UDP Server / Client echo-back test for the STM32F412 + W6300 SoM. Two modes available (uncomment one at a time in the main loop):

- **UDP Server** (default) - listens on port 5000, echoes received data back to sender
- **UDP Client** - sends to a remote UDP server, echoes received data

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to the same network as the test PC)

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1      | UDP   |

## How to Use

1. Define `EXAMPLE_UDP` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_UDP
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. Select UDP mode by uncommenting one in the main loop:

```c
/* UDP Server (default) */
loopback_udps(SOCKET_UDP, g_udp_buf, PORT_UDP);

/* UDP Client */
//loopback_udpc(SOCKET_UDP, g_udp_buf, g_udp_destip, PORT_UDP_DEST);
```

4. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP timeout handling.

5. Build, flash, and test with [Hercules](https://www.hw-group.com/software/hercules-setup-utility) UDP panel.

## Expected Output

```
=== UDP Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `g_udp_destip` / `PORT_UDP_DEST` - UDP Client destination
- `PORT_UDP` - Listening port number (default: 5000)
