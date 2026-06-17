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

### ioLibrary TFTP Patch

If required by your ioLibrary_Driver version, apply the TFTP patch:

```bash
cd Libraries/ioLibrary_Driver
git apply ../../patches/0002_iolibrary_driver_tftp.patch
```

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
#define TFTP_SERVER_IP         "192.168.11.100"
#define TFTP_SERVER_FILE_NAME  "tftp_test_file.txt"
```

4. Add the TFTP library to the build (if not already added):
   - Source Location: `Libraries/ioLibrary_Driver/Internet/TFTP`
   - Include Path: `../Libraries/ioLibrary_Driver/Internet/TFTP`

5. Add the timer tick to `SysTick_Handler()` in `stm32f4xx_it.c`:

```c
void SysTick_Handler(void)
{
    HAL_IncTick();

    extern void app_timer_tick(void);
    app_timer_tick();
}
```

6. Start the TFTP server on your PC and place a test file (e.g. `tftp_test_file.txt`) in the root directory.

7. Build, flash, and open a serial terminal (115200 bps).

## Expected Output

```
=== TFTP Client Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 TFTP server IP : 192.168.11.100
 File name      : tftp_test_file.txt
 Sending read request...
 TFTP read success : tftp_test_file.txt
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` — `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` — MAC, static IP, gateway, subnet
- `TFTP_SERVER_IP` — TFTP server IP address
- `TFTP_SERVER_FILE_NAME` — File to read from the server
- `TFTP_SERVER_PORT` — Server port, set in `tftp.h` (default: 69)