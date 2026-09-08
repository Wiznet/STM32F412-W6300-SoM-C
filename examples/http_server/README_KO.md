# HTTP Server 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 간단한 HTTP 서버입니다. 80 포트로 정적 웹 페이지를 제공합니다. 브라우저에서 보드 IP 주소로 접속하면 페이지를 볼 수 있습니다.

이 예제는 WIZnet-PICO-C의 HTTP 서버 예제를 기반으로 합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 테스트 PC와 같은 네트워크에 연결된 이더넷 케이블

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0 | DHCP |
| 1-4 | HTTP |

## 사용법

1. `Core/Inc/main.h`에 `EXAMPLE_HTTP_SERVER`를 정의합니다:

```c
#define EXAMPLE_HTTP_SERVER
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. 빌드·플래시하고 115200 bps로 시리얼 터미널을 엽니다.

4. 웹 브라우저를 열고 다음 주소로 접속합니다:

```text
http://<board IP>/
```

`Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP와 HTTP 서버 타이머를 구동합니다.

## 예상 출력

```text
=== HTTP Server Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 HTTP server running on port 80
```

## 설정 항목

다음을 수정할 수 있습니다:

- `app_main.c`의 `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `app_main.c`의 `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `web_page.h`의 `index_page` — 제공할 페이지의 HTML 내용
- `app_main.c`의 `HTTP_SOCKET_MAX_NUM` — 동시 HTTP 소켓 개수
