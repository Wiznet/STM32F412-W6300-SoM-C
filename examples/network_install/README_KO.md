# Network Install 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 네트워크 초기화 및 PHY 링크 검증 예제입니다. 이더넷 PHY 링크 상태를 확인하고, 링크 속도와 듀플렉스 모드를 출력한 뒤, ping으로 연결을 확인할 수 있도록 대기합니다. WIZnet-PICO-C의 network_install 예제 기반입니다.

가장 단순한 연결 테스트로, W6300 하드웨어와 네트워크 케이블 연결을 확인하는 데 사용하십시오.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (테스트 PC와 같은 네트워크에 연결)

## 사용법

1. `main.h`에 `EXAMPLE_NETWORK_INSTALL`을 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_NETWORK_INSTALL
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

고정 IP를 사용하는 경우 `g_net_info`를 사용 중인 네트워크에 맞게 조정하십시오.

3. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps).

4. "Try ping the IP"가 보이면 PC에서 보드로 ping을 보냅니다:

```bash
ping <board IP>
```

## 예상 출력

```
=== Network Install Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 Link OK of Internal PHY.
 The 100 Mbps speed of Internal PHY.
 The Full-Duplex Duplex Mode of the Internal PHY.

 Try ping the IP : <board IP>
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷

## 참고

- DHCP가 기본으로 활성화되어 있습니다. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출해 DHCP 타임아웃을 처리합니다.
- PHY 링크가 실패하면 이더넷 케이블 연결을 확인하십시오.
