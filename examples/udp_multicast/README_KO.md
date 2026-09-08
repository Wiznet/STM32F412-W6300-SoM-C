# UDP Multicast 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 UDP 멀티캐스트 수신기입니다. 보드가 멀티캐스트 그룹에 가입하고, ioLibrary_Driver의 `multicast.h` API를 사용해 그 그룹으로 전송된 데이터를 수신합니다.

두 가지 모드가 있습니다 (메인 루프에서 하나만 주석 해제):

- **multicast_recv** (기본) — 멀티캐스트 데이터를 수신해 출력
- **multicast_loopback** — 멀티캐스트 데이터를 수신해 에코백

**DHCP**와 **고정 IP** 모두 지원합니다.

> **W6300 주의:** W5500과 달리 W6300은 소켓을 열기 전에 멀티캐스트 목적지 하드웨어 주소(`setSn_DHAR`)를 설정해야 합니다. 이 예제에서는 `multicast_set_dhar()`가 자동으로 처리합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (멀티캐스트를 지원하는 네트워크에 연결)

## 소켓 할당

| 소켓 | 용도 |
|--------|-----------|
| 0      | DHCP      |
| 1      | Multicast |

## 사용법

1. `main.h`에 `EXAMPLE_UDP_MULTICAST`를 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_UDP_MULTICAST
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. `app_main.c`에서 멀티캐스트 그룹을 설정합니다:

```c
static uint8_t g_multicast_ip[] = {239, 0, 0, 1};
static uint16_t g_multicast_port = 5000;
```

4. CubeIDE 프로젝트에는 ioLibrary 멀티캐스트 소스가 이미 포함되어 있습니다. 프로젝트를 다시 만드는 경우 `Libraries/ioLibrary_Driver/Application/multicast`를 소스 및 인클루드 경로에 추가하십시오.

5. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP 타임아웃 처리를 구동합니다.

6. 빌드·플래시하고 테스트합니다:
   - Hercules UDP 패널을 엽니다
   - 목적지를 `239.0.0.1:5000`으로 설정합니다
   - 데이터를 전송하면 보드가 받은 메시지를 시리얼 터미널에 출력합니다

## 예상 출력

```
=== UDP Multicast Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 Multicast group : 239.0.0.1:5000

1:Multicast Recv start
1:Opened, UDP Multicast Socket
1:Multicast Group IP - 239.0.0.1
1:Multicast Group Port - 5000

recv size : 11
Hello World
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `g_multicast_ip` — 멀티캐스트 그룹 주소 (기본: `239.0.0.1`)
- `g_multicast_port` — 멀티캐스트 그룹 포트 (기본: 5000)
