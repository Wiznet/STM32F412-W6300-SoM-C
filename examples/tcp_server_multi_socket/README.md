# TCP Server Multi Socket Example

## Overview

TCP server that accepts multiple simultaneous client connections using W6300's hardware sockets. All TCP server sockets use the same port and echo received data back to the client (loopback).

When several WIZnet sockets listen on the same port, the lowest socket number accepts the next connection first. This example rotates the listen window after each accepted connection so socket 1 does not keep taking every new client.

Socket 0 is reserved for DHCP. Sockets 1-7 are used for TCP server.

All sockets must have non-zero TX/RX memory assigned in `wizchip_initialize()`. This project assigns 2KB TX and 2KB RX to each socket, using the W6300 16KB TX and 16KB RX memory evenly across sockets 0-7.

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable connected to the same network as the test PC

## Socket Allocation

| Socket | Usage      | Port |
|--------|------------|------|
| 0      | DHCP       | -    |
| 1      | TCP Server | 5000 |
| 2      | TCP Server | 5000 |
| 3      | TCP Server | 5000 |
| ...    | ...        | ...  |
| 7      | TCP Server | 5000 |

## How to Use

1. Define `EXAMPLE_TCP_SERVER_MULTI_SOCKET` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_TCP_SERVER_MULTI_SOCKET
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP timeout handling.

4. Build, flash, and open a serial terminal (115200 bps).

5. Test with Hercules:
   - Open a TCP Client tab and connect to `<board IP>:5000`
   - Open another TCP Client tab and connect to `<board IP>:5000`
   - Send data from each client - each gets its own echo back independently

## Expected Output

```text
=== TCP Server Multi Socket Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

1:Listen, TCP server loopback, port [5000]
2:Listen, TCP server loopback, port [5000]
3:Listen, TCP server loopback, port [5000]
...
1:Connected - 192.168.11.100 : 50000
socket1 from:192.168.11.100 port: 50000  message:Hello
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `PORT_TCP_SERVER` - TCP server port shared by all loopback sockets (default: 5000)
- `memsize` in `port/wizchip_qspi.c` - per-socket TX/RX memory allocation
