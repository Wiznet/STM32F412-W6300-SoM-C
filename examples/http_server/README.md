# HTTP Server Example

## Overview

Simple HTTP server for the STM32F412 + W6300 SoM. It serves a static web page on port 80. Open a browser and navigate to the board IP address to see the page.

This example is based on the WIZnet-PICO-C HTTP server example.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable connected to the same network as the test PC

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0 | DHCP |
| 1-4 | HTTP |

## How to Use

1. Define `EXAMPLE_HTTP_SERVER` in `Core/Inc/main.h`:

```c
#define EXAMPLE_HTTP_SERVER
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. Build, flash, and open a serial terminal at 115200 bps.

4. Open a web browser and navigate to:

```text
http://<board IP>/
```

`Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from `SysTick_Handler()`, which drives DHCP and HTTP server timers.

## Expected Output

```text
=== HTTP Server Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 HTTP server running on port 80
```

## Configuration

The following can be modified:

- `NET_MODE` in `app_main.c` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` in `app_main.c` - MAC, static IP, gateway, subnet
- `index_page` in `web_page.h` - HTML content of the served page
- `HTTP_SOCKET_MAX_NUM` in `app_main.c` - Number of concurrent HTTP sockets
