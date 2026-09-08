# DHCP + DNS 예제

[English](README.md) | 한국어

## 개요

이 예제는 DHCP로 IPv4 주소를 받은 뒤, DNS를 사용해 도메인 이름을 IPv4 주소로 확인합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- DHCP 서버가 있는 네트워크에 연결된 이더넷 케이블

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0 | DHCP |
| 1 | DNS |

## 사용법

1. `Core/Inc/main.h`에 `EXAMPLE_DHCP_DNS`를 정의합니다:

```c
#define EXAMPLE_DHCP_DNS
```

2. 빌드·플래시하고 115200 bps로 시리얼 터미널을 엽니다.

`Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP와 DNS 타임아웃 처리를 구동합니다.

## 예상 출력

```text
=== DHCP + DNS Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration

 MAC         : 00:08:DC:12:34:56
 IP          : 192.168.11.xxx
 Subnet Mask : 255.255.255.0
 Gateway     : 192.168.11.1
 DNS         : 192.168.11.1
==========================================================

 DNS success
 Target domain : example.com
 IP of target  : 93.184.216.34
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `g_dns_target_domain` — 확인할 도메인 이름
- `g_net_info` — MAC 주소 및 대체 네트워크 설정
- `DHCP_RETRY_COUNT` / `DNS_RETRY_COUNT` — 재시도 횟수 제한
