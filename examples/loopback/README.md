# Loopback Example

## Overview

Echo-back loopback test for the STM32F412 + W6300 SoM. Any data received is sent back to the sender. This is the most basic network connectivity test: if loopback works, the W6300 QSPI link and socket layer are functioning correctly.

Three modes available (uncomment one at a time in the main loop):

- **TCP Server** (default) - listens on port 5000
- **TCP Client** - connects to a remote server and echoes
- **UDP** - receives UDP packets and echoes back

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to the same network as the test PC)

## Socket Allocation

| Socket | Usage    |
|--------|----------|
| 0      | DHCP     |
| 1      | Loopback |

## How to Use

1. Define `EXAMPLE_LOOPBACK` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_LOOPBACK
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
// #define NET_MODE    NETINFO_STATIC
```

3. Select loopback mode by uncommenting one in the main loop:

```c
/* TCP Server (default) */
loopback_tcps(SOCKET_LOOPBACK, g_loopback_buf, PORT_LOOPBACK);

/* TCP Client */
// loopback_tcpc(SOCKET_LOOPBACK, g_loopback_buf, g_tcp_client_destip, PORT_TCP_CLIENT_DEST);

/* UDP */
// loopback_udps(SOCKET_LOOPBACK, g_loopback_buf, PORT_LOOPBACK);
```

4. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP timeout handling.

5. Build, flash, and test with [Hercules](https://www.hw-group.com/software/hercules-setup-utility).

## Expected Output

```
=== Loopback Example ===

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
- `g_tcp_client_destip` - TCP Client destination IP
- `PORT_LOOPBACK` / `PORT_TCP_CLIENT_DEST` - Port numbers
