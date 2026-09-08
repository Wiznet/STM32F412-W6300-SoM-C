# TCP Server Multi Socket 예제

[English](README.md) | 한국어

## 개요

W6300의 하드웨어 소켓을 사용해 여러 클라이언트 연결을 동시에 받는 TCP 서버입니다. 모든 TCP 서버 소켓이 같은 포트를 사용하며, 받은 데이터를 클라이언트에게 되돌려 보냅니다(루프백).

여러 WIZnet 소켓이 같은 포트에서 수신 대기할 때는 가장 낮은 소켓 번호가 다음 연결을 먼저 받습니다. 이 예제는 연결을 하나 수락할 때마다 수신 대기 창을 회전시켜, 소켓 1이 새 클라이언트를 계속 독차지하지 않도록 합니다.

소켓 0은 DHCP용으로 예약되어 있습니다. 소켓 1~7이 TCP 서버에 사용됩니다.

모든 소켓은 `wizchip_initialize()`에서 0이 아닌 TX/RX 메모리를 할당받아야 합니다. 이 프로젝트는 W6300의 16KB TX와 16KB RX 메모리를 소켓 0~7에 고르게 나누어 각 소켓에 2KB TX, 2KB RX를 할당합니다.

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 테스트 PC와 같은 네트워크에 연결된 이더넷 케이블

## 소켓 할당

| 소켓 | 용도 | 포트 |
|--------|------------|------|
| 0      | DHCP       | -    |
| 1      | TCP Server | 5000 |
| 2      | TCP Server | 5000 |
| 3      | TCP Server | 5000 |
| ...    | ...        | ...  |
| 7      | TCP Server | 5000 |

## 사용법

1. `main.h`에 `EXAMPLE_TCP_SERVER_MULTI_SOCKET`을 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_TCP_SERVER_MULTI_SOCKET
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP 타임아웃 처리를 구동합니다.

4. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps).

5. Hercules로 테스트합니다:
   - TCP Client 탭을 열고 `<board IP>:5000`에 접속합니다
   - 다른 TCP Client 탭을 열고 `<board IP>:5000`에 접속합니다
   - 각 클라이언트에서 데이터를 보내면 각각 독립적으로 에코를 받습니다

## 예상 출력

```text
=== TCP Server Multi Socket Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

1:Listen, TCP server loopback, port [5000]
2:Listen, TCP server loopback, port [5000]
3:Listen, TCP server loopback, port [5000]
...
1:Connected - 192.168.11.100 : 50000
socket1 from:192.168.11.100 port: 50000  message:Hello
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `PORT_TCP_SERVER` — 모든 루프백 소켓이 공유하는 TCP 서버 포트 (기본: 5000)
- `port/wizchip_qspi.c`의 `memsize` — 소켓별 TX/RX 메모리 할당
