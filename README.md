# STM32F412 + W6300 SoM Reference Examples

Ethernet examples for an STM32F412-based W6300 SoM board using
[ioLibrary_Driver](https://github.com/Wiznet/ioLibrary_Driver) and STM32 HAL.

## Hardware

- STM32F412 + W6300 SoM
- WIZnet W6300 hardwired TCP/IP Ethernet chip
- QSPI interface between STM32F412 and W6300
- USART3 serial output at 115200 bps

## Development Environment

These examples were developed and tested with:

- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) v1.15.1 or later
- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)
- [Hercules](https://www.hw-group.com/software/hercules-setup-utility) or another TCP/UDP test tool

## Directory Structure

```text
STM32F412-W6300-SoM-C/
|-- Core/                              # CubeMX generated application entry
|   |-- Inc/
|   |   |-- main.h                     # Example selection (#define EXAMPLE_XXX)
|   |   `-- app_main.h                 # Common example entry points
|   `-- Src/
|       |-- main.c                     # Calls app_main()
|       `-- stm32f4xx_it.c             # Calls app_timer_tick() from SysTick
|-- Drivers/                           # STM32 HAL/CMSIS drivers
|-- examples/
|   |-- dhcp_dns/                      # DHCP + DNS
|   |-- loopback/                      # TCP/UDP echo
|   |-- udp/                           # UDP server/client
|   |-- udp_multicast/                 # UDP multicast receiver
|   |-- tcp_server_multi_socket/       # Multi-socket TCP server
|   |-- sntp/                          # SNTP time sync
|   |-- netbios/                       # NetBIOS name service
|   |-- http_server/                   # HTTP server
|   |-- mqtt/                          # MQTT publish/subscribe
|   |-- tftp/                          # TFTP client
|   |-- pppoe/                         # PPPoE client
|   |-- network_install/               # Network init and PHY check
|   |-- upnp/                          # UPnP IGD control point
|   |-- tcp_client_over_ssl/           # TLS 1.2 client (mbedTLS + ATECC608C)
|   `-- tcp_server_over_ssl/           # TLS 1.2 echo server (mbedTLS + ATECC608C)
|-- Libraries/
|   |-- ioLibrary_Driver/              # WIZnet Ethernet driver submodule
|   |-- mbedtls/                       # mbedTLS 3.6 LTS submodule
|   `-- cryptoauthlib/                 # Microchip CryptoAuthLib submodule
|-- port/
|   |-- atecc608/                      # ATECC608C driver, entropy source, I2C HAL
|   |-- mbedtls/                       # Custom mbedtls_config.h (shadows library default)
|   |-- wizchip_qspi.c/.h             # W6300 QSPI port layer
|   |-- wizchip_dhcp.c/.h             # DHCP wrapper
|   `-- wizchip_tls.c/.h              # mbedTLS BIO callbacks for W6300 sockets
|-- STM32F412-W6300-SoM-C.ioc          # CubeMX project file
|-- STM32F412-W6300-SoM-C.launch       # CubeIDE debug launch config
`-- README.md
```

## Examples

Each example lives under `examples/` and provides its own `app_main.c`.
Select exactly one example macro in `Core/Inc/main.h`.

| Example | Macro | Description |
|---------|-------|-------------|
| `dhcp_dns` | `EXAMPLE_DHCP_DNS` | Obtain IP via DHCP and resolve a domain name via DNS |
| `loopback` | `EXAMPLE_LOOPBACK` | TCP server/client and UDP echo loopback |
| `udp` | `EXAMPLE_UDP` | UDP server/client echo-back |
| `udp_multicast` | `EXAMPLE_UDP_MULTICAST` | UDP multicast receiver with IGMP |
| `tcp_server_multi_socket` | `EXAMPLE_TCP_SERVER_MULTI_SOCKET` | Multi-socket TCP server on sequential ports |
| `sntp` | `EXAMPLE_SNTP` | Get current time from an SNTP server |
| `netbios` | `EXAMPLE_NETBIOS` | NetBIOS Name Service responder |
| `http_server` | `EXAMPLE_HTTP_SERVER` | HTTP server with a static web page |
| `mqtt` | `EXAMPLE_MQTT` | MQTT publish and subscribe |
| `tftp` | `EXAMPLE_TFTP` | TFTP client file read |
| `pppoe` | `EXAMPLE_PPPOE` | PPPoE client connection |
| `network_install` | `EXAMPLE_NETWORK_INSTALL` | Network init, PHY link check, and ping test |
| `upnp` | `EXAMPLE_UPNP` | UPnP IGD discovery and port forwarding |
| `tcp_client_over_ssl` | `EXAMPLE_TCP_CLIENT_OVER_SSL` | TLS 1.2 client (mbedTLS + ATECC608C), supports mTLS |
| `tcp_server_over_ssl` | `EXAMPLE_TCP_SERVER_OVER_SSL` | TLS 1.2 echo server (mbedTLS + ATECC608C) |

### TLS Examples

Both TLS examples require **I2C2** enabled in STM32CubeMX for ATECC608C communication and use the ATECC608C hardware RNG as the entropy source.

- **tcp_client_over_ssl**: Connects to an OpenSSL test server. Supports optional **mTLS** (mutual TLS) via `ENABLE_MTLS` in `tls_client.h` — when enabled, the ATECC608C-TNGTLS device certificate and private key (slot 0) are sent to the server for client authentication.
- **tcp_server_over_ssl**: Listens for TLS client connections using an embedded test certificate. Echoes received data back over the encrypted channel.

See each example's own README for setup details, OpenSSL commands, and expected output.

## Getting Started

### 1. Clone

```bash
git clone --recurse-submodules https://github.com/wiznet-grace/STM32F412-W6300-SoM-C.git
```

If the repository was cloned without submodules:

```bash
git submodule update --init --recursive
```

### 2. Open in STM32CubeIDE

Import the project with:

```text
File -> Import -> Existing Projects into Workspace -> select the cloned directory
```

### 3. Check Preprocessor Defines

The Debug configuration is set up for W6300 QSPI mode. If you recreate the
CubeIDE configuration or change build settings, make sure these symbols exist:

| Define | Value | Description |
|--------|-------|-------------|
| `_WIZCHIP_` | `W6300` | WIZnet chip selection |
| `_WIZCHIP_QSPI_MODE_` | `QSPI_QUAD_MODE` | QSPI bus mode |

Supported QSPI mode values are:

| Value | Mode |
|-------|------|
| `QSPI_QUAD_MODE` | Quad SPI |
| `QSPI_DUAL_MODE` | Dual SPI |
| `QSPI_SINGLE_MODE` | Single SPI |

### 4. Select One Example

Open `Core/Inc/main.h` and enable one example macro:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_DHCP_DNS
//#define EXAMPLE_LOOPBACK
//#define EXAMPLE_UDP
//#define EXAMPLE_UDP_MULTICAST
//#define EXAMPLE_TCP_SERVER_MULTI_SOCKET
//#define EXAMPLE_SNTP
//#define EXAMPLE_NETBIOS
//#define EXAMPLE_HTTP_SERVER
//#define EXAMPLE_MQTT
//#define EXAMPLE_TFTP
//#define EXAMPLE_PPPOE
//#define EXAMPLE_NETWORK_INSTALL
//#define EXAMPLE_UPNP
//#define EXAMPLE_TCP_CLIENT_OVER_SSL
//#define EXAMPLE_TCP_SERVER_OVER_SSL
/* USER CODE END Private defines */
```

### 5. Configure Network

Most examples define `NET_MODE` in their `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

Static IP settings are stored in each example's `g_net_info` structure.

### 6. SysTick Timer

`Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from
`SysTick_Handler()`. Each selected example implements `app_timer_tick()` for
the timers it needs, such as DHCP, DNS, HTTP server, or UPnP timeouts.

### 7. Build and Flash

Build the Debug configuration in STM32CubeIDE and flash with ST-Link.

### 8. Serial Monitor

Open a serial terminal at 115200 bps to view example output.

## Port Layer

The MCU-dependent W6300 code is in `port/`:

```text
port/
|-- atecc608/
|   |-- atca_config.h          # CryptoAuthLib project config
|   |-- atecc608.c/.h          # ATECC608C init, serial, cert, ECDSA test
|   |-- entropy_atecc608.c/.h  # mbedTLS entropy source (ATECC608 RNG)
|   `-- hal_stm32_i2c.c       # CryptoAuthLib I2C HAL for STM32
|-- mbedtls/
|   `-- mbedtls_config.h       # Custom mbedTLS config (shadows library default)
|-- wizchip_qspi.c/.h          # W6300 QSPI read/write, DMA + polling hybrid
|-- wizchip_dhcp.c/.h          # DHCP client wrapper
`-- wizchip_tls.c/.h           # mbedTLS BIO send/recv for W6300 sockets
```

Transfers shorter than `QSPI_DMA_THRESHOLD` use polling; longer transfers use
DMA for efficiency.

## Libraries

| Library | Location | Description |
|---------|----------|-------------|
| ioLibrary_Driver | `Libraries/ioLibrary_Driver` | WIZnet hardwired TCP/IP driver submodule |
| mbedTLS | `Libraries/mbedtls` | mbedTLS 3.6 LTS — TLS, crypto, X.509 (submodule) |
| CryptoAuthLib | `Libraries/cryptoauthlib` | Microchip CryptoAuthentication Library (submodule) |
| STM32 HAL | `Drivers/STM32F4xx_HAL_Driver` | STM32F4 HAL drivers |
| CMSIS | `Drivers/CMSIS` | Arm CMSIS headers |

## License

This project is licensed under the MIT License. See `LICENSE`.

Third-party components keep their own licenses:

- STM32 HAL: see `Drivers/STM32F4xx_HAL_Driver/LICENSE.txt`
- CMSIS: Apache-2.0, see `Drivers/CMSIS/LICENSE.txt`
- ioLibrary_Driver: MIT-style WIZnet license, see `Libraries/ioLibrary_Driver/license.txt`

## References

- [W6300 Datasheet](https://docs.wiznet.io/Product/Chip/Ethernet/W6300)
- [STM32F412 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0402-stm32f412-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [ioLibrary_Driver GitHub](https://github.com/Wiznet/ioLibrary_Driver)
- [WIZnet-PICO-C](https://github.com/WIZnet-ioNIC/WIZnet-PICO-C)
