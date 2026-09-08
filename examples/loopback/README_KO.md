# Loopback 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 에코백 루프백 테스트입니다. 수신한 데이터를 그대로 송신자에게 되돌려 보냅니다. 가장 기본적인 네트워크 연결 테스트로, 루프백이 동작하면 W6300 QSPI 링크와 소켓 계층이 정상이라는 뜻입니다.

세 가지 모드가 있습니다 (메인 루프에서 한 번에 하나만 주석 해제):

- **TCP 서버** (기본) — 포트 5000에서 수신 대기
- **TCP 클라이언트** — 원격 서버에 접속해 에코
- **UDP** — UDP 패킷을 수신해 에코백

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (테스트 PC와 같은 네트워크에 연결)

## 소켓 할당

| 소켓 | 용도 |
|--------|----------|
| 0      | DHCP     |
| 1      | Loopback |

## 사용법

1. `main.h`에 `EXAMPLE_LOOPBACK`을 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_LOOPBACK
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
// #define NET_MODE    NETINFO_STATIC
```

3. 메인 루프에서 하나를 주석 해제해 루프백 모드를 선택합니다:

```c
/* TCP Server (default) */
loopback_tcps(SOCKET_LOOPBACK, g_loopback_buf, PORT_LOOPBACK);

/* TCP Client */
// loopback_tcpc(SOCKET_LOOPBACK, g_loopback_buf, g_tcp_client_destip, PORT_TCP_CLIENT_DEST);

/* UDP */
// loopback_udps(SOCKET_LOOPBACK, g_loopback_buf, PORT_LOOPBACK);
```

4. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP 타임아웃 처리를 구동합니다.

5. 빌드·플래시한 뒤 [Hercules](https://www.hw-group.com/software/hercules-setup-utility)로 테스트합니다.

## 예상 출력

```
=== Loopback Example ===

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
- `g_tcp_client_destip` — TCP 클라이언트 목적지 IP
- `PORT_LOOPBACK` / `PORT_TCP_CLIENT_DEST` — 포트 번호
