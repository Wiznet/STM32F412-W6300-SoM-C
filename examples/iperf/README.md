# iperf2 Example

## Overview

TCP throughput test for the STM32F412 + W6300 SoM. The board runs an **iperf
version 2** server on TCP port **5001**. The PC-side `iperf` client pushes data
and reports the measured throughput; the board simply receives and discards it.

Measures roughly **70 Mbps** on a 100 Mbit link with the default settings.

Supports both **DHCP** and **static IP**.

Ported from [WIZnet-PICO-IPERF3-C](https://github.com/WIZnet-ioNIC/WIZnet-PICO-IPERF3-C)
`examples/iperf2`.

> **iperf version 2 only** - not compatible with `iperf3`.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to the same network as the test PC)
- iperf **v2** installed on the PC

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1      | iperf |

Enabling this example also selects a throughput-tuned socket buffer layout in
`port/wizchip_qspi.c` (socket 1 RX = 16 KB). Other examples keep the default
2 KB per socket.

## How to Use

1. Define `EXAMPLE_IPERF` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_IPERF
/* USER CODE END Private defines */
```

2. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
// #define NET_MODE    NETINFO_STATIC
```

3. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP timeout handling.

4. Build and flash, then read the board IP from the serial output.

5. On the PC, run the client against that IP:

```
iperf -c <board_ip> -p 5001 -i 2 -t 10
```

## Expected Output

Board (UART, 115200 8N1):

```
=== iperf2 Example (TCP server, port 5001) ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
==========================================================
 W6300 network configuration
 ...
==========================================================
 DHCP leased time : 7200 seconds
 iperf2 server ready. On the PC (iperf v2) run:
   iperf -c 192.168.11.7 -i 1 -t 10

 DHCP success
```

PC:

```
[  1] local 192.168.11.2 port 64654 connected with 192.168.11.7 port 5001
[ ID] Interval       Transfer     Bandwidth
[  1] 0.00-2.00 sec  16.6 MBytes  69.7 Mbits/sec
...
[  1] 0.00-10.03 sec  79.8 MBytes  66.7 Mbits/sec
```

## Performance

Measured on STM32F412RET6 @ 100 MHz over a 100 Mbit link:

| QSPI mode / clock | socket 1 RX | recv chunk | throughput |
|---|---|---|---|
| SINGLE, 2 MHz | 2 KB | 2 KB | 5.1 Mbps |
| QUAD, 50 MHz | 2 KB | 2 KB | 17.9 Mbps |
| QUAD, 25 MHz | 16 KB | 2 KB | 34.6 Mbps |
| QUAD, 50 MHz | 16 KB | 2 KB | 38.0 Mbps |
| QUAD, 50 MHz | 16 KB | 16 KB | **~70 Mbps** (defaults) |

The recv chunk matters more than the QSPI clock: doubling the clock from 25 to
50 MHz gained about 3 Mbps, while raising the chunk from 2 KB to 16 KB gained
over 30. All figures above require `SF_TCP_NODELAY`; without it the same
settings measure under 1 Mbps.

## Configuration

In `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `PORT_IPERF` - server port (default 5001)
- `IPERF_BUF_MAX_SIZE` - bytes pulled per `recv()`. The single biggest
  throughput knob: each `recv()` carries a fixed register overhead, so larger
  chunks amortise it. Must not exceed the socket 1 RX buffer.

In `port/wizchip_qspi.c`:

- `memsize` - socket TX/RX buffer sizes (`EXAMPLE_IPERF` branch)

In `Core/Src/main.c`:

- `hqspi.Init.ClockPrescaler` - QSPI clock, `HCLK / (prescaler + 1)`. Mirror any
  change in the `.ioc` so CubeMX regeneration keeps it.

> **Do not remove `SF_TCP_NODELAY` from the `socket()` call.** Without it the
> W6300 falls back to a ~200 ms delayed ACK once the RX buffer exceeds one MSS,
> and throughput drops to well under 1 Mbps. The comment at that call explains
> the mechanism.
