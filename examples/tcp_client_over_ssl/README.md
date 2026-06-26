# TCP Client over SSL Example

## Overview

TLS 1.2 client for the STM32F412 + W6300 SoM. Connects to a TLS server, sends a hello message, and echoes received data back until the server closes the connection.

Uses mbedTLS 3.6 LTS with the ATECC608C-TNGTLS for hardware-backed ECDSA signing, device certificate authentication, and entropy (RNG). The W6300 hardwired TCP/IP chip handles the underlying TCP connection.

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
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

2. I2C2 is already configured in the `.ioc` for the on-board ATECC608C-TNGTLS
secure element. If you regenerate the CubeMX project, keep I2C2 enabled at
100 kHz and make sure `MX_I2C2_Init()` runs before `app_main()`.

3. mTLS is enabled by default in `tls_client.h`, which configures the
ATECC608C device certificate for client authentication. Comment out
`ENABLE_MTLS` to test without client authentication:

```c
#define ENABLE_MTLS
```

4. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
// #define NET_MODE    NETINFO_STATIC
```

5. Set the TLS server IP in `app_main.c`:

```c
static uint8_t g_tls_server_ip[] = {192, 168, 11, 2};
#define TLS_SERVER_PORT        443
```

6. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`, which drives DHCP timeout handling.

7. Generate a test server certificate and start the TLS server on your PC:

```bash
openssl ecparam -genkey -name prime256v1 -out server_key.pem
openssl req -new -x509 -key server_key.pem -out server.pem -days 365 -subj "/CN=test"
openssl s_server -accept 443 -cert server.pem -key server_key.pem -tls1_2
```

8. Build, flash, and open a serial terminal (115200 bps).

## Expected Output

### Serial Terminal (mTLS enabled)

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
[TLS] Device cert loaded
[TLS] Signer cert loaded
[TLS] PK -> ATECC608 slot 0
[TLS] mTLS: enabled
[TLS] Init complete
[TLS] Connecting to 192.168.11.2:443 ...
[TLS] TCP connected
[TLS] Starting TLS handshake...
[TLS] Handshake OK!
[TLS] Ciphersuite: TLS-ECDHE-ECDSA-WITH-AES-128-GCM-SHA256
[TLS] TLS version: TLSv1.2
[TLS] Sent 43 bytes: Hello from STM32F412-W6300-SoM TLS Client
```

### Serial Terminal (mTLS disabled)

```
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

```bash
# Without mTLS after commenting out ENABLE_MTLS
openssl s_server -accept 443 -cert server.pem -key server_key.pem -tls1_2

# With mTLS (default)
openssl s_server -accept 443 -cert server.pem -key server_key.pem -tls1_2 -verify 1
```

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

- `ENABLE_MTLS` - defined in `tls_client.h` by default; comment it out to disable mTLS
- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `g_tls_server_ip` - TLS server IP address
- `TLS_SERVER_PORT` - TLS server port (default: 443)

## TLS Settings

- **Server certificate verification**: disabled (`VERIFY_NONE`). To enable, load a CA certificate into `s_cacert` and change `authmode` to `MBEDTLS_SSL_VERIFY_REQUIRED` in `tls_client.c`.
- **mTLS (client certificate)**: controlled by `ENABLE_MTLS` in `tls_client.h`. When enabled, the ATECC608C-TNGTLS device certificate and private key (slot 0) are used for client authentication. The OpenSSL server must use `-verify 1` to request the client certificate.
- **Entropy source**: ATECC608C hardware RNG (`atcab_random()`), seeded into mbedTLS via `entropy_atecc608.c`.
