# TCP Server over SSL Example

## Overview

TLS 1.2 echo server for the STM32F412 + W6300 SoM. Listens for incoming TLS client connections, performs the TLS handshake, sends a welcome message, and echoes received data back over the encrypted channel.

Uses mbedTLS 3.6 LTS with the ATECC608C-TNGTLS for hardware-backed ECDSA signing, device certificate authentication, and entropy (RNG). The server certificate and private key are embedded in the firmware (ECDSA P-256 test certificate from mbedTLS).

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to the same network as the test client)

## Software

- [OpenSSL](https://www.openssl.org/) for testing as a TLS client from a PC

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1      | TLS   |

## How to Use

1. Define `EXAMPLE_TCP_SERVER_OVER_SSL` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_TCP_SERVER_OVER_SSL
/* USER CODE END Private defines */
```

2. I2C2 is already configured in the `.ioc` for the on-board ATECC608C-TNGTLS
secure element. If you regenerate the CubeMX project, keep I2C2 enabled at
100 kHz and make sure `MX_I2C2_Init()` runs before `app_main()`.

3. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
// #define NET_MODE    NETINFO_STATIC
```

4. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP timeout handling.

5. Build, flash, and open a serial terminal (115200 bps).

6. Connect to the server from your PC using OpenSSL:

```bash
openssl s_client -connect 192.168.11.2:443 -tls1_2
```

Replace `192.168.11.2` with the board's actual IP address (shown in the serial terminal after DHCP).

Type a message and press Enter to see it echoed back.

## Expected Output

### Serial Terminal

```
========================================
 TCP Server over SSL Example
 STM32F412 + W6300 + ATECC608C
========================================

[ATECC608] Init OK
QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

========================================
 TLS 1.2 Echo Server
========================================
[TLS] RNG seeded OK
[TLS] Server cert loaded
[TLS] Server key loaded
[TLS] Server init complete
[TLS] Listening on port 443

[TLS] Client connected
[TLS] Starting handshake...
[TLS] Handshake OK!
[TLS] Ciphersuite: TLS-ECDHE-ECDSA-WITH-AES-128-GCM-SHA256
[TLS] TLS version: TLSv1.2
[TLS] Received 12 bytes: Hello World
[TLS] Echo sent 12 bytes
```

### OpenSSL Client

```
CONNECTED(00000003)
---
Hello from W6300 TLS Server
Hello World
Hello World
```

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `TLS_SERVER_PORT` - Listening port (default: 443)

## TLS Settings

- **Server certificate**: embedded ECDSA P-256 test certificate (mbedTLS PolarSSL Test EC). Replace `srv_cert_pem` and `srv_key_pem` in `tls_server.c` with your own certificate for production use.
- **Client certificate verification**: disabled (`VERIFY_NONE`). The server does not request a client certificate.
- **Entropy source**: ATECC608C hardware RNG (`atcab_random()`), seeded into mbedTLS via `entropy_atecc608.c`.

### Replacing the Server Certificate

Generate your own ECDSA P-256 certificate and key:

```bash
openssl ecparam -genkey -name prime256v1 -out server_key.pem
openssl req -new -x509 -key server_key.pem -out server.pem -days 365 -subj "/CN=W6300-TLS-Server"
```

Then format each PEM file as a C string and replace `srv_cert_pem` / `srv_key_pem` in `tls_server.c`. Each line should be wrapped as `"line\r\n"`.
