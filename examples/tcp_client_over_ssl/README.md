# TCP Client over SSL Example

## Overview

TLS 1.2 client for the STM32F412 + W6300 SoM. Connects to a TLS server, sends a hello message, and echoes received data back until the server closes the connection.

Uses **mbedTLS 3.6 LTS** with the **ATECC608C-TNGTLS** hardware security element for entropy (RNG). The W6300 hardwired TCP/IP chip handles the underlying TCP connection.

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- ATECC608C-TNGTLS (I2C2, address 0x35)
- Ethernet cable (connected to the same network as the TLS server)

## Software

- [OpenSSL](https://www.openssl.org/) for running a test TLS server on a PC

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1      | TLS   |

## How to Use

1. Define `EXAMPLE_TCP_CLIENT_OVER_SSL` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_TCP_CLIENT_OVER_SSL
/* USER CODE END Private defines */
```

2. Enable **I2C2** in STM32CubeMX (`.ioc`) for ATECC608C communication:
   - Mode: I2C, 100 kHz, 7-bit addressing
   - After code generation, ensure `MX_I2C2_Init()` is called before `app_main()`

3. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
// #define NET_MODE    NETINFO_STATIC
```

4. Set the TLS server IP in `app_main.c`:

```c
static uint8_t g_tls_server_ip[] = {192, 168, 11, 2};
#define TLS_SERVER_PORT        443
```

5. Add the timer tick to `SysTick_Handler()` in `stm32f4xx_it.c`:

```c
void SysTick_Handler(void)
{
    HAL_IncTick();

    extern void app_timer_tick(void);
    app_timer_tick();
}
```

6. Generate a test server certificate and start the TLS server on your PC:

```bash
openssl ecparam -genkey -name prime256v1 -out server_key.pem
openssl req -new -x509 -key server_key.pem -out server.pem -days 365 -subj "/CN=test"
openssl s_server -accept 443 -cert server.pem -key server_key.pem -tls1_2
```

7. Build, flash, and open a serial terminal (115200 bps).

## Expected Output

### Serial Terminal

```
========================================
 TCP Client over SSL Example
 STM32F412 + W6300 + ATECC608C-TNGTLS
========================================

[ATECC608] Init OK

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

========================================
 TLS 1.2 Client Echo Test
========================================
[TLS] RNG seeded OK
[TLS] mTLS: disabled
[TLS] Init complete
[TLS] Connecting to 192.168.11.2:443 ...
[TLS] TCP connected
[TLS] Starting TLS handshake...
[TLS] Handshake OK!
[TLS] Ciphersuite: TLS-ECDHE-ECDSA-WITH-AES-128-GCM-SHA256
[TLS] TLS version: TLSv1.2
[TLS] Sent 43 bytes: Hello from STM32F412-W6300-SoM TLS Client
```

### OpenSSL Server

```
ACCEPT
-----BEGIN SSL SESSION PARAMETERS-----
...
-----END SSL SESSION PARAMETERS-----
Hello from STM32F412-W6300-SoM TLS Client
```

Type a message in the OpenSSL server terminal to echo it back to the board.

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` — `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` — MAC, static IP, gateway, subnet
- `g_tls_server_ip` — TLS server IP address
- `TLS_SERVER_PORT` — TLS server port (default: 443)

## TLS Settings

This example uses simplified TLS settings for initial bring-up:

- **Server certificate verification**: disabled (`VERIFY_NONE`). The client does not validate the server's certificate. To enable, load a CA certificate into `s_cacert` and change `authmode` to `MBEDTLS_SSL_VERIFY_REQUIRED` in `tls_client.c`.
- **mTLS (client certificate)**: disabled. The ATECC608C-TNGTLS can provide a device certificate for mutual TLS authentication. To enable, change `#if 0` to `#if 1` in `tls_client.c` and configure the server to request a client certificate.
- **Entropy source**: ATECC608C hardware RNG (`atcab_random()`), seeded into mbedTLS via `entropy_atecc608.c`.
