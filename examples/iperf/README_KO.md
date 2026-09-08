# iperf2 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 TCP 처리량 테스트입니다. 보드가 TCP **5001** 포트에서 **iperf 버전 2** 서버로 동작합니다. PC 측 `iperf` 클라이언트가 데이터를 밀어넣고 측정된 처리량을 보고하며, 보드는 그것을 받아서 버리기만 합니다.

기본 설정에서 100 Mbit 링크 기준 약 **70 Mbps**를 측정합니다.

**DHCP**와 **고정 IP** 모두 지원합니다.

[WIZnet-PICO-IPERF3-C](https://github.com/WIZnet-ioNIC/WIZnet-PICO-IPERF3-C)의 `examples/iperf2`에서 이식했습니다.

> **iperf 버전 2 전용** — `iperf3`와는 호환되지 않습니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (테스트 PC와 같은 네트워크에 연결)
- PC에 설치된 iperf **v2**

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0      | DHCP  |
| 1      | iperf |

이 예제를 활성화하면 `port/wizchip_qspi.c`에서 처리량에 맞춰 조정된 소켓 버퍼 배치(소켓 1 RX = 16 KB)도 함께 선택됩니다. 다른 예제들은 소켓당 기본값 2 KB를 유지합니다.

## 사용법

1. `main.h`에 `EXAMPLE_IPERF`를 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_IPERF
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
// #define NET_MODE    NETINFO_STATIC
```

3. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP 타임아웃 처리를 구동합니다.

4. 빌드·플래시한 뒤 시리얼 출력에서 보드 IP를 확인합니다.

5. PC에서 그 IP를 대상으로 클라이언트를 실행합니다:

```
iperf -c <board_ip> -p 5001 -i 2 -t 10
```

## 예상 출력

보드 (UART, 115200 8N1):

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

## 성능

STM32F412RET6 @ 100 MHz, 100 Mbit 링크에서 측정:

| QSPI 모드 / 클럭 | 소켓 1 RX | recv 청크 | 처리량 |
|---|---|---|---|
| SINGLE, 2 MHz | 2 KB | 2 KB | 5.1 Mbps |
| QUAD, 50 MHz | 2 KB | 2 KB | 17.9 Mbps |
| QUAD, 25 MHz | 16 KB | 2 KB | 34.6 Mbps |
| QUAD, 50 MHz | 16 KB | 2 KB | 38.0 Mbps |
| QUAD, 50 MHz | 16 KB | 16 KB | **~70 Mbps** (기본값) |

QSPI 클럭보다 recv 청크 크기가 더 중요합니다. 클럭을 25에서 50 MHz로 두 배 올렸을 때는 약 3 Mbps가 늘었지만, 청크를 2 KB에서 16 KB로 올렸을 때는 30 Mbps 넘게 늘었습니다. 위 수치는 모두 `SF_TCP_NODELAY`가 필요하며, 없으면 같은 설정에서 1 Mbps 미만으로 측정됩니다.

## 설정 항목

`app_main.c`에서:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `PORT_IPERF` — 서버 포트 (기본 5001)
- `IPERF_BUF_MAX_SIZE` — `recv()` 한 번에 가져오는 바이트 수. 처리량에 가장 큰 영향을 주는 값입니다. `recv()`마다 고정된 레지스터 오버헤드가 있어 청크가 클수록 그 비용이 분산됩니다. 소켓 1의 RX 버퍼를 초과해서는 안 됩니다.

`port/wizchip_qspi.c`에서:

- `memsize` — 소켓 TX/RX 버퍼 크기 (`EXAMPLE_IPERF` 분기)

`Core/Src/main.c`에서:

- `hqspi.Init.ClockPrescaler` — QSPI 클럭, `HCLK / (prescaler + 1)`. CubeMX 재생성 시에도 유지되도록 변경 사항을 `.ioc`에도 반영하십시오.

> **`socket()` 호출에서 `SF_TCP_NODELAY`를 제거하지 마십시오.** 없으면 RX 버퍼가 1 MSS를 넘는 순간 W6300이 약 200 ms 지연 ACK로 되돌아가 처리량이 1 Mbps 아래로 떨어집니다. 해당 호출부의 주석에 그 메커니즘이 설명되어 있습니다.
