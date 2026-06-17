# STM32F412 + W6300 SoM Reference Examples

Ethernet examples for STM32F412 + W6300 SoM using [**ioLibrary_Driver**](https://github.com/WIZnet-ioNIC/ioLibrary_Driver) and STM32 HAL.

## Hardware

The ethernet examples use:

- **STM32F412 + W6300 SoM** — System on Module built on STM32F412 MCU and WIZnet's W6300 hardwired TCP/IP ethernet chip (QSPI interface)

## Development Environment

These examples were developed and tested using:

- [**STM32CubeIDE**](https://www.st.com/en/development-tools/stm32cubeide.html) (v1.15.1+)
- [**STM32CubeMX**](https://www.st.com/en/development-tools/stm32cubemx.html)
- [**Hercules**](https://www.hw-group.com/software/hercules-setup-utility) — TCP/UDP test tool

## Directory Structure

```
STM32F412-W6300-SoM-C/
├── Core/                              # CubeMX generated (HAL init, main.c)
│   ├── Inc/
│   │   ├── main.h                     # Example selection (#define EXAMPLE_XXX)
│   │   └── app_main.h                 # Common entry point declaration
│   └── Src/
│       ├── main.c                     # Calls app_main()
│       └── stm32f4xx_it.c             # SysTick → app_timer_tick()
├── Drivers/                           # STM32 HAL drivers (CubeMX generated)
├── examples/
│   ├── dhcp_dns/                      # DHCP + DNS
│   ├── loopback/                      # TCP/UDP echo (server, client)
│   ├── udp/                           # UDP server / client
│   ├── udp_multicast/                 # UDP multicast receiver
│   ├── tcp_server_multi_socket/       # Multi-socket TCP server
│   ├── sntp/                          # SNTP time sync
│   ├── netbios/                       # NetBIOS name service
│   ├── http/                          # HTTP server
│   ├── mqtt/                          # MQTT publish & subscribe
│   ├── tftp/                          # TFTP client
│   ├── pppoe/                         # PPPoE client
│   ├── network_install/               # Network init & PHY check
│   ├── upnp/                          # UPnP IGD control point
│   ├── tcp_client_over_ssl/           # TLS client (ATECC608)
│   └── tcp_server_over_ssl/           # TLS server (ATECC608)
├── Libraries/
│   ├── ioLibrary_Driver/              # WIZnet ethernet driver (submodule)
│   └── ATECC608/                      # ATECC608 crypto driver
├── port/
│   ├── wizchip_qspi.c/.h             # W6300 QSPI port layer
│   └── wizchip_dhcp.c/.h             # DHCP wrapper
├── STM32F412-W6300-SoM.ioc           # CubeMX project file
└── README.md
```

## Examples

Examples are available at `examples/` directory. Each example has its own `app_main.c` and `README.md`.

| Example | Description |
|---------|-------------|
| **dhcp_dns** | Obtain IP via DHCP, resolve domain name via DNS |
| **loopback** | TCP Server / TCP Client / UDP echo-back loopback |
| **udp** | UDP server / client echo-back |
| **udp_multicast** | UDP multicast group receiver (IGMP) |
| **tcp_server_multi_socket** | Multi-socket TCP server on sequential ports |
| **sntp** | Get current time from SNTP server |
| **netbios** | NetBIOS Name Service (NBNS) responder |
| **http** | HTTP server with static web page |
| **mqtt** | MQTT publish & subscribe (Paho client) |
| **tftp** | TFTP client file read |
| **pppoe** | PPPoE client connection |
| **network_install** | Network init, PHY link check, ping test |
| **upnp** | UPnP IGD discovery & port forwarding |
| **tcp_client_over_ssl** | TLS client with ATECC608 (coming soon) |
| **tcp_server_over_ssl** | TLS server with ATECC608 (coming soon) |

> **Note:** `tcp_client_over_ssl` and `tcp_server_over_ssl` require the ATECC608 security chip.

## Getting Started

### 1. Clone

```bash
git clone --recurse-submodules https://github.com/user/STM32F412-W6300-SoM-C.git
```

### 2. Open in STM32CubeIDE

Import the project: **File → Import → Existing Projects into Workspace** → select the cloned directory.

### 3. Select Example

Open `Core/Inc/main.h` and define **one** example macro in the `USER CODE BEGIN Private defines` section:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_DHCP_DNS
//#define EXAMPLE_LOOPBACK
//#define EXAMPLE_UDP
//#define EXAMPLE_UDP_MULTICAST
//#define EXAMPLE_TCP_SERVER_MULTI_SOCKET
//#define EXAMPLE_SNTP
//#define EXAMPLE_NETBIOS
//#define EXAMPLE_HTTP
//#define EXAMPLE_MQTT
//#define EXAMPLE_TFTP
//#define EXAMPLE_PPPOE
//#define EXAMPLE_NETWORK_INSTALL
//#define EXAMPLE_UPNP
//#define EXAMPLE_TCP_CLIENT_OVER_SSL
//#define EXAMPLE_TCP_SERVER_OVER_SSL
/* USER CODE END Private defines */
```

### 4. Configure Network

Each example's `app_main.c` has a `NET_MODE` define to switch between DHCP and static IP:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

### 5. SysTick Timer

Most examples require `app_timer_tick()` to be called from `SysTick_Handler()`. Add the following to `stm32f4xx_it.c`:

```c
void SysTick_Handler(void)
{
    HAL_IncTick();

    extern void app_timer_tick(void);
    app_timer_tick();
}
```

### 6. Build and Flash

Click **Build** (Ctrl+B) in STM32CubeIDE, then flash via ST-Link.

### 7. Serial Monitor

Open a serial terminal at **115200 bps** to view output.

## Port Layer

The MCU-dependent code is in the `port/` directory:

```
port/
├── wizchip_qspi.c      # W6300 QSPI read/write (DMA + polling hybrid)
├── wizchip_qspi.h      # QSPI port declarations, mode selection
├── wizchip_dhcp.c       # DHCP client wrapper
└── wizchip_dhcp.h       # DHCP wrapper declarations
```

The QSPI port supports three modes (configurable in `wizchip_qspi.h`):

```c
#define _WIZCHIP_QSPI_MODE_  QSPI_QUAD_MODE    // default
//#define _WIZCHIP_QSPI_MODE_  QSPI_DUAL_MODE
//#define _WIZCHIP_QSPI_MODE_  QSPI_SINGLE_MODE
```

Transfers shorter than `QSPI_DMA_THRESHOLD` (default: 16 bytes) use polling; longer transfers use DMA for efficiency.

## Libraries

| Library | Description |
|---------|-------------|
| [**ioLibrary_Driver**](https://github.com/WIZnet-ioNIC/ioLibrary_Driver) | WIZnet hardwired TCP/IP driver (W5100S, W5500, W6100, W6300) |
| **mbedtls** | TLS/SSL library (for SSL examples) |
| **ATECC608** | Microchip ATECC608 secure element driver (for SSL examples) |

## References

- [**W6300 Datasheet**](https://docs.wiznet.io/Product/iEthernet/W6300)
- [**STM32F412 Reference Manual**](https://www.st.com/resource/en/reference_manual/rm0402-stm32f412-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [**ioLibrary_Driver GitHub**](https://github.com/WIZnet-ioNIC/ioLibrary_Driver)
- [**WIZnet-PICO-C**](https://github.com/WIZnet-ioNIC/WIZnet-PICO-C) — Original reference (RP2040/RP2350)