# UPnP Example

## Overview

UPnP (Universal Plug and Play) IGD control point for the STM32F412 + W6300 SoM. Discovers an Internet Gateway Device on the network via SSDP, then provides an interactive serial menu for port forwarding (AddPortMapping / DeletePortMapping) and LED control. Based on the WIZnet-PICO-C upnp example.

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to a router with UPnP/IGD support)

## Socket Allocation

| Socket | Usage            |
|--------|------------------|
| 0      | DHCP             |
| 1      | UPnP (SSDP/HTTP) |
| 2      | TCP loopback     |
| 3      | Event listener   |

## Setup

The UPnP source files are already included in this example directory and the
CubeIDE project source path. If you recreate the project, add `examples/upnp`
to the source and include paths.

## How to Use

1. Define `EXAMPLE_UPNP` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_UPNP
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP and UPnP timeout handling.

4. Build, flash, and open a serial terminal (115200 bps).

5. The board will discover the IGD on your network and show an interactive menu.

## Expected Output

```
=== UPnP Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 Send SSDP..
 GetDescription Success!!
 SetEventing Success!!

====================== WIZnet Chip Control Point ===================
This Application is basic example of UART interface with
Windows Hyper Terminal.
 1 - Set LED On
 2 - Set LED Off
 3 - AddPort
 4 - DeletePort
 5 - Run Loopback
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `PORT_TCP` / `PORT_UDP` - Loopback ports (default: 8000 / 5000)
- `SOCKET_UPNP` - Base socket number for UPnP

## Note

- `my_time` (declared in `UPnP.c`) is incremented every 1 second by `app_timer_tick()` for timeout handling.
- LED control uses a stub function (prints to serial). Replace `setUserLEDStatus()` with actual GPIO control for your board.
- Requires a router with UPnP/IGD support enabled.
