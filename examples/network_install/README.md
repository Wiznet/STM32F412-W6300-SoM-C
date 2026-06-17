# Network Install Example

## Overview

Network initialization and PHY link verification for the STM32F412 + W6300 SoM. Checks the Ethernet PHY link status, prints link speed and duplex mode, and waits for a ping to confirm connectivity. Based on the WIZnet-PICO-C network_install example.

This is the simplest connectivity test — use it to verify the basic W6300 hardware and network cable connection.

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

2. Adjust `g_net_info` in `app_main.c` to match your network.

3. Build, flash, and open a serial terminal (115200 bps).

4. Once you see "Try ping the IP", ping the board from your PC:

```bash
ping 192.168.11.2
```

## Expected Output

```
=== Network Install Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
==========================================================
 W6300 network configuration
 ...
==========================================================

 Link OK of Internal PHY.
 The 100 Mbps speed of Internal PHY.
 The Full-Duplex Duplex Mode of the Internal PHY.

 Try ping the IP : 192.168.11.2
```

## Configuration

The following can be modified in `app_main.c`:

- `g_net_info` — MAC, IP, gateway, subnet (static IP only)

## Note

- No DHCP or timer tick is required for this example.
- If PHY link fails, check the Ethernet cable connection.