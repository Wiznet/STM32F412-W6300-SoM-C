# HTTP Server Example

## Overview

Simple HTTP server for the STM32F412 + W6300 SoM. Serves a static web page on port 80. Open a browser and navigate to the board's IP address to see the page. Based on the WIZnet-PICO-C http server example.

Uses 4 sockets for concurrent HTTP connections.

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to the same network as the test PC)

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1–4    | HTTP  |

## How to Use

1. Define `EXAMPLE_HTTP` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_HTTP
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. Add the httpServer library to the build:
   - Source Location: `Libraries/ioLibrary_Driver/Internet/httpServer`
   - Include Path: `../Libraries/ioLibrary_Driver/Internet/httpServer`

4. When using DHCP, add the timer tick to `SysTick_Handler()` in `stm32f4xx_it.c`:

```c
void SysTick_Handler(void)
{
    HAL_IncTick();

    extern void app_timer_tick(void);
    app_timer_tick();
}
```

5. Build, flash, and open a serial terminal (115200 bps).

6. Open a web browser and navigate to `http://<board IP>/`.

## Expected Output

### Serial Terminal

```
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

### Web Browser

> **Hello from STM32F412 + W6300 SoM!**
>
> HTTP Server example is running.

## Configuration

The following can be modified:

- `NET_MODE` in `app_main.c` — `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` in `app_main.c` — MAC, static IP, gateway, subnet
- `index_page` in `web_page.h` — HTML content of the served page
- `HTTP_SOCKET_MAX_NUM` in `app_main.c` — Number of concurrent HTTP sockets (default: 4)