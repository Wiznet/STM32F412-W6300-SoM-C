# PPPoE Example

## Overview

PPPoE (Point-to-Point Protocol over Ethernet) client for the STM32F412 + W6300 SoM. Connects to a PPPoE server and obtains an IP address via PAP/CHAP authentication. Based on the WIZnet-PICO-C pppoe example.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to a PPPoE-enabled network or server)

## Setup

The PPPoE source files are already included in this example directory and the
CubeIDE project source path. If you recreate the project, add `examples/pppoe`
to the source and include paths.

## How to Use

1. Define `EXAMPLE_PPPOE` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_PPPOE
/* USER CODE END Private defines */
```

2. Set PPPoE credentials in `app_main.c`:

```c
uint8_t pppoe_id[6] = "W6300";
uint8_t pppoe_id_len = 5;
uint8_t pppoe_pw[7] = "WIZnet";
uint8_t pppoe_pw_len = 6;
```

3. Build, flash, and open a serial terminal (115200 bps).

## Expected Output

```
=== PPPoE Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
==========================================================
 W6300 network configuration
 ...
==========================================================

 PPPoE connecting...

<<<< PPPoE Success >>>>
 Assigned IP address : X.X.X.X

==================================================
    AFTER PPPoE, Net Configuration Information
==================================================
 MAC address    : 0:8:dc:12:34:56
 SUBNET MASK    : X.X.X.X
 G/W IP ADDRESS : X.X.X.X
 SOURCE IP ADDR : X.X.X.X
```

## Configuration

The following can be modified in `app_main.c`:

- `g_net_info` - Initial network configuration
- `pppoe_id` / `pppoe_id_len` - PPPoE username
- `pppoe_pw` / `pppoe_pw_len` - PPPoE password

## Note

- PPPoE uses MACRAW socket internally; no additional socket allocation is needed.
- No DHCP or timer tick is required for this example.
- The `PPP_MAX_RETRY_COUNT` is defined in `PPPoE.h` (default: 5).
