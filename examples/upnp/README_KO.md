# UPnP 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 UPnP(Universal Plug and Play) IGD 제어점입니다. SSDP로 네트워크상의 인터넷 게이트웨이 장치를 검색한 뒤, 포트 포워딩(AddPortMapping / DeletePortMapping)과 LED 제어를 위한 대화형 시리얼 메뉴를 제공합니다. WIZnet-PICO-C의 upnp 예제 기반입니다.

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (UPnP/IGD를 지원하는 공유기에 연결)

## 소켓 할당

| 소켓 | 용도 |
|--------|------------------|
| 0      | DHCP             |
| 1      | UPnP (SSDP/HTTP) |
| 2      | TCP 루프백       |
| 3      | 이벤트 리스너    |

## 준비

UPnP 소스 파일은 이 예제 디렉터리와 CubeIDE 프로젝트 소스 경로에 이미 포함되어 있습니다. 프로젝트를 다시 만드는 경우 `examples/upnp`를 소스 및 인클루드 경로에 추가하십시오.

## 사용법

1. `main.h`에 `EXAMPLE_UPNP`를 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_UPNP
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP와 UPnP 타임아웃 처리를 구동합니다.

4. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps).

5. 보드가 네트워크의 IGD를 검색하고 대화형 메뉴를 표시합니다.

## 예상 출력

```
=== UPnP Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 Send SSDP..
 GetDescription Success!!
 SetEventing Success!!

====================== WIZnet Chip Control Point ===================
This Application is basic example of UART interface with
Windows Hyper Terminal.
 1 - Set LED On
 2 - Set LED Off
 3 - AddPort
 4 - DeletePort
 5 - Run Loopback
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `PORT_TCP` / `PORT_UDP` — 루프백 포트 (기본: 8000 / 5000)
- `SOCKET_UPNP` — UPnP 기준 소켓 번호

## 참고

- `UPnP.c`에 선언된 `my_time`은 타임아웃 처리를 위해 `app_timer_tick()`이 1초마다 증가시킵니다.
- LED 제어는 스텁 함수(시리얼 출력)를 사용합니다. 사용하는 보드에 맞게 `setUserLEDStatus()`를 실제 GPIO 제어로 교체하십시오.
- UPnP/IGD가 활성화된 공유기가 필요합니다.
