# SNTP Example

## Overview

Gets current date and time from an SNTP server for the STM32F412 + W6300 SoM. Uses `time.google.com` by default with Korea timezone (UTC+9). Based on the WIZnet-PICO-C sntp example.

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to a network with internet access)

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1      | SNTP  |

## How to Use

1. Define `EXAMPLE_SNTP` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_SNTP
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. The CubeIDE project already includes the ioLibrary SNTP source. If you
recreate the project, add `Libraries/ioLibrary_Driver/Internet/SNTP` to the
source and include paths.

4. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP and SNTP timeout handling.

5. Build, flash, and open a serial terminal (115200 bps).

## Expected Output

```
=== SNTP Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 SNTP server : 216.239.35.0
 Requesting time...
 2026-06-16, 14:30:25
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `g_sntp_server_ip` - SNTP server IP (default: `216.239.35.0` / time.google.com)
- `TIMEZONE` - Timezone offset (default: 40 = Korea UTC+9)
- `RECV_TIMEOUT` - SNTP response timeout in ms (default: 10000)
