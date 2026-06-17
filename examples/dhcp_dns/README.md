# DHCP & DNS Example

## Overview

This example demonstrates how to obtain an IP address automatically via DHCP and resolve a domain name to an IP address using DNS on the STM32F412 + W6300 SoM.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to a network with a DHCP server)

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1      | DNS   |

## How to Use

1. Define `EXAMPLE_DHCP_DNS` in `main.c`:

```c
#define EXAMPLE_DHCP_DNS
```

2. Add the timer tick to `SysTick_Handler()` in `stm32f4xx_it.c`:

```c
void SysTick_Handler(void)
{
    HAL_IncTick();

    extern void app_dhcp_dns_time_tick(void);
    app_dhcp_dns_time_tick();
}
```

3. Build, flash, and open a serial terminal (115200 bps) to see the output.

## Expected Output

```
=== DHCP + DNS Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration

 MAC         : 00:08:DC:12:34:56
 IP          : 192.168.11.XXX
 Subnet Mask : 255.255.255.0
 Gateway     : 192.168.11.1
 DNS         : 192.168.11.1
==========================================================

 DNS success
 Target domain : example.com
 IP of target  : 93.184.216.34
```

## Configuration

The following can be modified in `app_dhcp_dns.c`:

- `g_dns_target_domain` — Domain name to resolve (default: `example.com`)
- `g_net_info` — Network settings such as MAC address and fallback IP
- `DHCP_RETRY_COUNT` / `DNS_RETRY_COUNT` — Number of retries before failure