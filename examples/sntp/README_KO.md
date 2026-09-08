# SNTP 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM에서 SNTP 서버로부터 현재 날짜와 시각을 가져옵니다. 기본으로 `time.google.com`과 한국 시간대(UTC+9)를 사용합니다. WIZnet-PICO-C의 sntp 예제 기반입니다.

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (인터넷이 되는 네트워크에 연결)

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0      | DHCP  |
| 1      | SNTP  |

## 사용법

1. `main.h`에 `EXAMPLE_SNTP`를 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_SNTP
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. CubeIDE 프로젝트에는 ioLibrary SNTP 소스가 이미 포함되어 있습니다. 프로젝트를 다시 만드는 경우 `Libraries/ioLibrary_Driver/Internet/SNTP`를 소스 및 인클루드 경로에 추가하십시오.

4. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP와 SNTP 타임아웃 처리를 구동합니다.

5. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps).

## 예상 출력

```
=== SNTP Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 SNTP server : 216.239.35.0
 Requesting time...
 2026-06-16, 14:30:25
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `g_sntp_server_ip` — SNTP 서버 IP (기본: `216.239.35.0` / time.google.com)
- `TIMEZONE` — 시간대 오프셋 (기본: 40 = 한국 UTC+9)
- `RECV_TIMEOUT` — SNTP 응답 타임아웃, ms (기본: 10000)
