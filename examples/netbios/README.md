# NetBIOS Example

## Overview

NetBIOS Name Service (NBNS) responder for the STM32F412 + W6300 SoM. It lets
the board respond to hostname queries on UDP port 137.

This example is based on the WIZnet-PICO-C NetBIOS example.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable connected to the same local network as the test PC

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0 | DHCP |
| 1 | NetBIOS |

## How to Use

1. Define `EXAMPLE_NETBIOS` in `Core/Inc/main.h`:

```c
#define EXAMPLE_NETBIOS
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. Build, flash, and open a serial terminal at 115200 bps.

4. Query the board name from a PC on the same network:

```bash
nbtstat -a W6300
```

`Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP timeout handling.

## Expected Output

```text
=== NetBIOS Example ===

 DHCP client running
 NetBIOS name service running

 DHCP success
==========================================================
 W6300 network configuration

 MAC         : 00:08:DC:12:34:56
 IP          : 192.168.11.xxx
 Subnet Mask : 255.255.255.0
 Gateway     : 192.168.11.1
 DNS         : 192.168.11.1
==========================================================
```

From a PC on the same network, query the board:

```bash
nbtstat -a W6300
```

## Configuration

The following can be modified:

- `NET_MODE` in `app_main.c` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` in `app_main.c` - MAC, static IP, gateway, subnet
- `NETBIOS_BOARD_NAME` in `netbios.c` - NetBIOS hostname
