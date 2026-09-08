# UDP 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 UDP 서버/클라이언트 에코백 테스트입니다. 두 가지 모드가 있습니다 (메인 루프에서 한 번에 하나만 주석 해제):

- **UDP 서버** (기본) — 포트 5000에서 수신 대기하며 받은 데이터를 송신자에게 되돌려 보냄
- **UDP 클라이언트** — 원격 UDP 서버로 전송하고 받은 데이터를 에코

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (테스트 PC와 같은 네트워크에 연결)

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0      | DHCP  |
| 1      | UDP   |

## 사용법

1. `main.h`에 `EXAMPLE_UDP`를 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_UDP
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. 메인 루프에서 하나를 주석 해제해 UDP 모드를 선택합니다:

```c
/* UDP Server (default) */
loopback_udps(SOCKET_UDP, g_udp_buf, PORT_UDP);

/* UDP Client */
//loopback_udpc(SOCKET_UDP, g_udp_buf, g_udp_destip, PORT_UDP_DEST);
```

4. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP 타임아웃 처리를 구동합니다.

5. 빌드·플래시한 뒤 [Hercules](https://www.hw-group.com/software/hercules-setup-utility)의 UDP 패널로 테스트합니다.

## 예상 출력

```
=== UDP Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `g_udp_destip` / `PORT_UDP_DEST` — UDP 클라이언트 목적지
- `PORT_UDP` — 수신 대기 포트 번호 (기본: 5000)
