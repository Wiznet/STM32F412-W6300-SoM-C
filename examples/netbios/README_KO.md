# NetBIOS 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 NetBIOS 네임 서비스(NBNS) 응답기입니다. 보드가 UDP 137 포트에서 호스트명 조회에 응답할 수 있게 합니다.

이 예제는 WIZnet-PICO-C의 NetBIOS 예제를 기반으로 합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 테스트 PC와 같은 로컬 네트워크에 연결된 이더넷 케이블

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0 | DHCP |
| 1 | NetBIOS |

## 사용법

1. `Core/Inc/main.h`에 `EXAMPLE_NETBIOS`를 정의합니다:

```c
#define EXAMPLE_NETBIOS
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. 빌드·플래시하고 115200 bps로 시리얼 터미널을 엽니다.

4. 같은 네트워크의 PC에서 보드 이름을 조회합니다:

```bash
nbtstat -a W6300
```

`Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP 타임아웃 처리를 구동합니다.

## 예상 출력

```text
=== NetBIOS Example ===

 DHCP client running
 NetBIOS name service running

 DHCP success
==========================================================
 W6300 network configuration

 MAC         : 00:08:DC:12:34:56
 IP          : 192.168.11.xxx
 Subnet Mask : 255.255.255.0
 Gateway     : 192.168.11.1
 DNS         : 192.168.11.1
==========================================================
```

같은 네트워크의 PC에서 보드를 조회합니다:

```bash
nbtstat -a W6300
```

## 설정 항목

다음을 수정할 수 있습니다:

- `app_main.c`의 `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `app_main.c`의 `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `netbios.c`의 `NETBIOS_BOARD_NAME` — NetBIOS 호스트명
