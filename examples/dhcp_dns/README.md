# DHCP + DNS Example

## Overview

This example obtains an IPv4 address via DHCP and then resolves a domain name to an IPv4 address using DNS.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable connected to a network with a DHCP server

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0 | DHCP |
| 1 | DNS |

## How to Use

1. Define `EXAMPLE_DHCP_DNS` in `Core/Inc/main.h`:

```c
#define EXAMPLE_DHCP_DNS
```

2. Build, flash, and open a serial terminal at 115200 bps.

`Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from `SysTick_Handler()`, which drives DHCP and DNS timeout handling.

## Expected Output

```text
=== DHCP + DNS Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration

 MAC         : 00:08:DC:12:34:56
 IP          : 192.168.11.xxx
 Subnet Mask : 255.255.255.0
 Gateway     : 192.168.11.1
 DNS         : 192.168.11.1
==========================================================

 DNS success
 Target domain : example.com
 IP of target  : 93.184.216.34
```

## Configuration

The following can be modified in `app_main.c`:

- `g_dns_target_domain` - Domain name to resolve
- `g_net_info` - MAC address and fallback network settings
- `DHCP_RETRY_COUNT` / `DNS_RETRY_COUNT` - Retry limits
