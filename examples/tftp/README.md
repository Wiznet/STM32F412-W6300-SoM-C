# TFTP Client Example

## Overview

TFTP client that reads a file from a TFTP server on the network. Based on the WIZnet-PICO-C tftp example.

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to the same network as the TFTP server)

## Software

- [SolarWinds TFTP Server](https://www.solarwinds.com/free-tools/free-tftp-server) or any TFTP server

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1      | TFTP  |

## Setup

The CubeIDE project already includes the ioLibrary TFTP source. If you recreate
the project, add `Libraries/ioLibrary_Driver/Internet/TFTP` to the source and
include paths.

## How to Use

1. Define `EXAMPLE_TFTP` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_TFTP
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. Set the TFTP server IP and file name in `app_main.c`:

```c
#define TFTP_SERVER_IP         "192.168.11.2"
#define TFTP_SERVER_FILE_NAME  "tftp_test_file.txt"
```

4. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP and TFTP timeout handling.

5. Start the TFTP server on your PC and place a test file (e.g. `tftp_test_file.txt`) in the root directory.

6. Build, flash, and open a serial terminal (115200 bps).

## Expected Output

```
=== TFTP Client Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 TFTP server IP : 192.168.11.2
 File name      : tftp_test_file.txt
 Sending read request...
 TFTP read success : tftp_test_file.txt
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `TFTP_SERVER_IP` - TFTP server IP address
- `TFTP_SERVER_FILE_NAME` - File to read from the server
- `TFTP_SERVER_PORT` - Server port, set in `tftp.h` (default: 69)
