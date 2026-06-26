# Network Install Example

## Overview

Network initialization and PHY link verification for the STM32F412 + W6300 SoM. Checks the Ethernet PHY link status, prints link speed and duplex mode, and waits for a ping to confirm connectivity. Based on the WIZnet-PICO-C network_install example.

This is the simplest connectivity test - use it to verify the basic W6300 hardware and network cable connection.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to the same network as the test PC)

## How to Use

1. Define `EXAMPLE_NETWORK_INSTALL` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_NETWORK_INSTALL
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

When using static IP, adjust `g_net_info` to match your network.

3. Build, flash, and open a serial terminal (115200 bps).

4. Once you see "Try ping the IP", ping the board from your PC:

```bash
ping <board IP>
```

## Expected Output

```
=== Network Install Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 Link OK of Internal PHY.
 The 100 Mbps speed of Internal PHY.
 The Full-Duplex Duplex Mode of the Internal PHY.

 Try ping the IP : <board IP>
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet

## Note

- DHCP is enabled by default. `Core/Src/stm32f4xx_it.c` already calls
  `app_timer_tick()` from `SysTick_Handler()` for DHCP timeout handling.
- If PHY link fails, check the Ethernet cable connection.
