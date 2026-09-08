# STM32F412 + W6300 SoM 레퍼런스 예제

[English](README.md) | 한국어

[ioLibrary_Driver](https://github.com/Wiznet/ioLibrary_Driver), STM32 HAL, mbedTLS, CryptoAuthLib을 사용하는 STM32F412 + W6300 SoM 보드용 이더넷 및 TLS 예제입니다.

## 하드웨어

이 예제들은 다음을 통합한 STM32F412 + W6300 SoM 보드를 대상으로 합니다:

- STM32F412RET6 MCU (512 KB Flash, 256 KB SRAM, LQFP64)
- W6300 하드와이어드 TCP/IP 이더넷 칩 (QSPI 인터페이스)
- ATECC608C-TNGTLS 보안 소자 (I2C2, 7비트 주소 0x35)

### 핀 맵

| 기능 | STM32F412RET6 핀 | 페리페럴 신호 | 비고 |
|----------|-----------------|-------------------|------|
| W6300 QSPI CLK | PB2 | QUADSPI_CLK | QSPI 클럭 |
| W6300 QSPI CSn | PB6 | QUADSPI_BK1_NCS | 하드웨어 칩 셀렉트 |
| W6300 QSPI IO0 | PC9 | QUADSPI_BK1_IO0 | QSPI 데이터 라인 |
| W6300 QSPI IO1 | PC10 | QUADSPI_BK1_IO1 | QSPI 데이터 라인 |
| W6300 QSPI IO2 | PC8 | QUADSPI_BK1_IO2 | QSPI 데이터 라인 |
| W6300 QSPI IO3 | PA1 | QUADSPI_BK1_IO3 | QSPI 데이터 라인 |
| W6300 RSTn | PC0 | GPIO 출력 | Active-low 리셋 |
| W6300 INTn | PC1 | GPIO 입력 | Active-low 인터럽트 |
| ATECC608C SCL | PB10 | I2C2_SCL | 100 kHz I2C |
| ATECC608C SDA | PB9 | I2C2_SDA | 100 kHz I2C |
| Serial TX | PA9 | USART1_TX | 115200 bps 콘솔 |
| Serial RX | PA10 | USART1_RX | 115200 bps 콘솔 |
| HSE OSC_IN | PH0 | RCC_OSC_IN | 25 MHz 외부 크리스털 |
| HSE OSC_OUT | PH1 | RCC_OSC_OUT | 25 MHz 외부 크리스털 |

## 개발 환경

이 예제들은 다음 환경에서 개발 및 테스트되었습니다:

- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) v1.15.1 이상
- `.ioc`를 재생성하는 경우 [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) v6.12.1
- STM32Cube FW_F4 v1.28.3
- USART1 (PA9/PA10) 시리얼 터미널, 115200 bps, 8-N-1
- [Hercules](https://www.hw-group.com/software/hercules-setup-utility) 또는 다른 TCP/UDP 테스트 도구

## 디렉터리 구조

```text
STM32F412-W6300-SoM-C/
|-- Core/                              # CubeMX가 생성한 애플리케이션 진입부
|   |-- Inc/
|   |   |-- main.h                     # 예제 선택 (#define EXAMPLE_XXX)
|   |   `-- app_main.h                 # 공통 예제 진입점
|   `-- Src/
|       |-- main.c                     # app_main() 호출
|       `-- stm32f4xx_it.c             # SysTick에서 app_timer_tick() 호출
|-- Drivers/                           # STM32 HAL/CMSIS 드라이버
|-- examples/
|   |-- dhcp_dns/                      # DHCP + DNS
|   |-- loopback/                      # TCP/UDP 에코
|   |-- iperf/                         # iperf2 TCP 처리량 테스트
|   |-- udp/                           # UDP 서버/클라이언트
|   |-- udp_multicast/                 # UDP 멀티캐스트 수신
|   |-- tcp_server_multi_socket/       # 멀티 소켓 TCP 서버
|   |-- sntp/                          # SNTP 시각 동기화
|   |-- netbios/                       # NetBIOS 네임 서비스
|   |-- http_server/                   # HTTP 서버
|   |-- mqtt/                          # MQTT 발행/구독
|   |-- mqtts/                         # MQTT over TLS (AWS IoT Core)
|   |-- tftp/                          # TFTP 클라이언트
|   |-- pppoe/                         # PPPoE 클라이언트
|   |-- network_install/               # 네트워크 초기화 및 PHY 확인
|   |-- upnp/                          # UPnP IGD 제어점
|   |-- tcp_client_over_ssl/           # TLS 1.2 에코 클라이언트 (mbedTLS + ATECC608C)
|   `-- tcp_server_over_ssl/           # TLS 1.2 에코 서버 (mbedTLS + ATECC608C)
|-- Libraries/
|   |-- ioLibrary_Driver/              # WIZnet 이더넷 드라이버 서브모듈
|   |-- mbedtls/                       # mbedTLS 3.6 LTS 서브모듈
|   `-- cryptoauthlib/                 # Microchip CryptoAuthLib 서브모듈
|-- port/
|   |-- atecc608/                      # ATECC608C 드라이버, 엔트로피 소스, I2C HAL
|   |-- mbedtls/                       # 커스텀 mbedtls_config.h (라이브러리 기본값을 가림)
|   |-- wizchip_qspi.c/.h             # W6300 QSPI 포트 계층
|   |-- wizchip_dhcp.c/.h             # DHCP 래퍼
|   `-- wizchip_tls.c/.h              # W6300 소켓용 mbedTLS BIO 콜백
|-- STM32F412-W6300-SoM-C.ioc          # CubeMX 프로젝트 파일
|-- STM32F412-W6300-SoM-C.launch       # CubeIDE 디버그 실행 설정
`-- README.md
```

## 예제

각 예제는 `examples/` 아래에 있으며 자체 `app_main.c`를 제공합니다. `Core/Inc/main.h`에서 예제 매크로를 **정확히 하나만** 선택하십시오.

| 예제 | 매크로 | 설명 |
|---------|-------|-------------|
| `dhcp_dns` | `EXAMPLE_DHCP_DNS` | DHCP로 IP를 받고 DNS로 도메인 이름 확인 |
| `loopback` | `EXAMPLE_LOOPBACK` | TCP 서버/클라이언트 및 UDP 에코 루프백 |
| `iperf` | `EXAMPLE_IPERF` | iperf2 TCP 처리량 서버 (포트 5001, iperf v2 전용) |
| `udp` | `EXAMPLE_UDP` | UDP 서버/클라이언트 에코백 |
| `udp_multicast` | `EXAMPLE_UDP_MULTICAST` | IGMP를 사용하는 UDP 멀티캐스트 수신 |
| `tcp_server_multi_socket` | `EXAMPLE_TCP_SERVER_MULTI_SOCKET` | 연속된 포트의 멀티 소켓 TCP 서버 |
| `sntp` | `EXAMPLE_SNTP` | SNTP 서버에서 현재 시각 가져오기 |
| `netbios` | `EXAMPLE_NETBIOS` | NetBIOS 네임 서비스 응답기 |
| `http_server` | `EXAMPLE_HTTP_SERVER` | 정적 웹 페이지를 제공하는 HTTP 서버 |
| `mqtt` | `EXAMPLE_MQTT` | MQTT 발행 및 구독 |
| `mqtts` | `EXAMPLE_MQTTS` | AWS IoT Core로의 MQTT over TLS 1.2 (mbedTLS + ATECC608C) |
| `tftp` | `EXAMPLE_TFTP` | TFTP 클라이언트 파일 읽기 |
| `pppoe` | `EXAMPLE_PPPOE` | PPPoE 클라이언트 연결 |
| `network_install` | `EXAMPLE_NETWORK_INSTALL` | 네트워크 초기화, PHY 링크 확인, ping 테스트 |
| `upnp` | `EXAMPLE_UPNP` | UPnP IGD 검색 및 포트 포워딩 |
| `tcp_client_over_ssl` | `EXAMPLE_TCP_CLIENT_OVER_SSL` | TLS 1.2 클라이언트 (mbedTLS + ATECC608C), mTLS 지원 |
| `tcp_server_over_ssl` | `EXAMPLE_TCP_SERVER_OVER_SSL` | TLS 1.2 에코 서버 (mbedTLS + ATECC608C) |

### TLS 예제

세 가지 TLS 예제 모두 **I2C2**를 통해 온보드 **ATECC608C-TNGTLS**를 사용하며, ATECC608C 하드웨어 RNG를 엔트로피 소스로 사용합니다.

- **tcp_client_over_ssl**: OpenSSL 테스트 서버에 접속합니다. `tls_client.h`의 `ENABLE_MTLS`로 선택적 **mTLS**(상호 TLS)를 지원하며, 활성화하면 ATECC608C-TNGTLS 장치 인증서와 개인키(슬롯 0)가 클라이언트 인증에 사용됩니다.
- **tcp_server_over_ssl**: 내장된 테스트 인증서로 TLS 클라이언트 연결을 수신합니다. 받은 데이터를 암호화된 채널로 되돌려 보냅니다.
- **mqtts**: 8883 포트로 AWS IoT Core에 접속하는 MQTT over TLS 1.2. `mqtt` 예제와 동일한 Paho MQTT 클라이언트 아래에 mbedTLS 전송 계층을 둡니다. 보드는 ATECC608C-TNGTLS 장치 인증서로 인증하므로 개인키가 플래시에 저장되지 않습니다.

`port/mbedtls/mbedtls_config.h`는 셋이 공유합니다. 에코 예제를 위한 ECDHE-ECDSA(P-256)와, RSA-2048 인증서를 제시하는 AWS IoT를 위한 ECDHE-RSA를 함께 담고 있습니다.

설정 방법, OpenSSL 명령, 예상 출력은 각 예제의 README를 참고하십시오.

### 처리량

`iperf` 예제는 100 Mbit 링크에서 약 **70 Mbps** TCP를 측정합니다 (QSPI QUAD @ 50 MHz). 튜닝 항목은 [`examples/iperf/README_KO.md`](examples/iperf/README_KO.md)를 참고하십시오.

## 시작하기

### 1. 클론

```bash
git clone --recurse-submodules https://github.com/Wiznet/STM32F412-W6300-SoM-C.git
```

서브모듈 없이 클론한 경우:

```bash
git submodule update --init --recursive
```

### 2. STM32CubeIDE에서 열기

다음 경로로 프로젝트를 임포트합니다:

```text
File -> Import -> Existing Projects into Workspace -> 클론한 디렉터리 선택
```

### 3. 전처리기 정의 확인

Debug 구성은 W6300 QSPI 모드로 설정되어 있습니다. CubeIDE 구성을 다시 만들거나 빌드 설정을 변경하는 경우 다음 심볼이 있는지 확인하십시오:

| 정의 | 값 | 설명 |
|--------|-------|-------------|
| `_WIZCHIP_` | `W6300` | WIZnet 칩 선택 |
| `_WIZCHIP_QSPI_MODE_` | `QSPI_QUAD_MODE` | QSPI 버스 모드 |

지원되는 QSPI 모드 값:

| 값 | 모드 |
|-------|------|
| `QSPI_QUAD_MODE` | Quad SPI |
| `QSPI_DUAL_MODE` | Dual SPI |
| `QSPI_SINGLE_MODE` | Single SPI |

### 4. 예제 하나 선택

`Core/Inc/main.h`를 열고 `EXAMPLE_*` 매크로를 정확히 하나만 주석 해제 상태로 둡니다. 예를 들어 DHCP + DNS 예제를 빌드하려면:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_DHCP_DNS
//#define EXAMPLE_LOOPBACK
//#define EXAMPLE_IPERF
//#define EXAMPLE_UDP
//#define EXAMPLE_UDP_MULTICAST
//#define EXAMPLE_TCP_SERVER_MULTI_SOCKET
//#define EXAMPLE_SNTP
//#define EXAMPLE_NETBIOS
//#define EXAMPLE_HTTP_SERVER
//#define EXAMPLE_MQTT
//#define EXAMPLE_MQTTS
//#define EXAMPLE_TFTP
//#define EXAMPLE_PPPOE
//#define EXAMPLE_NETWORK_INSTALL
//#define EXAMPLE_UPNP
//#define EXAMPLE_TCP_CLIENT_OVER_SSL
//#define EXAMPLE_TCP_SERVER_OVER_SSL
/* USER CODE END Private defines */
```

### 5. 네트워크 설정

대부분의 예제는 자신의 `app_main.c`에 `NET_MODE`를 정의합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

고정 IP 설정은 각 예제의 `g_net_info` 구조체에 있습니다.

### 6. SysTick 타이머

`Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출합니다. 선택한 예제는 DHCP, DNS, HTTP 서버, UPnP 타임아웃 등 필요한 타이머를 위해 각자 `app_timer_tick()`을 구현합니다.

### 7. 빌드 및 플래시

STM32CubeIDE에서 Debug 구성을 빌드하고 ST-Link로 플래시합니다.

### 8. 시리얼 모니터

USART1 (PA9/PA10)에 115200 bps, 8-N-1로 시리얼 터미널을 열어 예제 출력을 확인합니다.

## 포트 계층

MCU 의존적인 W6300 코드는 `port/`에 있습니다:

```text
port/
|-- atecc608/
|   |-- atca_config.h          # CryptoAuthLib 프로젝트 설정
|   |-- atecc608.c/.h          # ATECC608C 초기화, 시리얼, 인증서, ECDSA 테스트
|   |-- entropy_atecc608.c/.h  # mbedTLS 엔트로피 소스 (ATECC608 RNG)
|   `-- hal_stm32_i2c.c       # STM32용 CryptoAuthLib I2C HAL
|-- mbedtls/
|   `-- mbedtls_config.h       # 커스텀 mbedTLS 설정 (라이브러리 기본값을 가림)
|-- wizchip_qspi.c/.h          # W6300 QSPI 읽기/쓰기, DMA + 폴링 하이브리드
|-- wizchip_dhcp.c/.h          # DHCP 클라이언트 래퍼
`-- wizchip_tls.c/.h           # W6300 소켓용 mbedTLS BIO 송수신
```

`QSPI_DMA_THRESHOLD`보다 짧은 전송은 폴링을, 그보다 긴 전송은 효율을 위해 DMA를 사용합니다.

## 라이브러리

| 라이브러리 | 위치 | 설명 |
|---------|----------|-------------|
| ioLibrary_Driver | `Libraries/ioLibrary_Driver` | WIZnet 하드와이어드 TCP/IP 드라이버 서브모듈 |
| mbedTLS | `Libraries/mbedtls` | TLS, 암호화, X.509를 위한 mbedTLS 3.6 LTS (서브모듈) |
| CryptoAuthLib | `Libraries/cryptoauthlib` | Microchip CryptoAuthentication 라이브러리 (서브모듈) |
| STM32 HAL | `Drivers/STM32F4xx_HAL_Driver` | STM32F4 HAL 드라이버 |
| CMSIS | `Drivers/CMSIS` | Arm CMSIS 헤더 |

## 라이선스

이 프로젝트는 MIT 라이선스를 따릅니다. `LICENSE`를 참고하십시오.

서드파티 구성 요소는 각자의 라이선스를 유지합니다:

- STM32 HAL: `Drivers/STM32F4xx_HAL_Driver/LICENSE.txt` 참고
- CMSIS: Apache-2.0, `Drivers/CMSIS/LICENSE.txt` 참고
- ioLibrary_Driver: MIT 형식의 WIZnet 라이선스, `Libraries/ioLibrary_Driver/license.txt` 참고
- mbedTLS: `Libraries/mbedtls/LICENSE` 참고
- CryptoAuthLib: `Libraries/cryptoauthlib/license.txt` 참고

## 참고 자료

- [W6300 데이터시트](https://docs.wiznet.io/Product/Chip/Ethernet/W6300)
- [STM32F412 레퍼런스 매뉴얼](https://www.st.com/resource/en/reference_manual/rm0402-stm32f412-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [ioLibrary_Driver GitHub](https://github.com/Wiznet/ioLibrary_Driver)
- [ATECC608C 데이터시트](https://www.microchip.com/en-us/product/ATECC608C)
- [CryptoAuthLib GitHub](https://github.com/MicrochipTech/cryptoauthlib)
- [WIZnet-PICO-C](https://github.com/WIZnet-ioNIC/WIZnet-PICO-C)
- [WIZnet-PICO-IPERF3-C](https://github.com/WIZnet-ioNIC/WIZnet-PICO-IPERF3-C) (iperf2 예제의 원본)
