# PPPoE 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 PPPoE(Point-to-Point Protocol over Ethernet) 클라이언트입니다. PPPoE 서버에 접속해 PAP/CHAP 인증으로 IP 주소를 받습니다. WIZnet-PICO-C의 pppoe 예제 기반입니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (PPPoE를 지원하는 네트워크 또는 서버에 연결)

## 준비

PPPoE 소스 파일은 이 예제 디렉터리와 CubeIDE 프로젝트 소스 경로에 이미 포함되어 있습니다. 프로젝트를 다시 만드는 경우 `examples/pppoe`를 소스 및 인클루드 경로에 추가하십시오.

## 사용법

1. `main.h`에 `EXAMPLE_PPPOE`를 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_PPPOE
/* USER CODE END Private defines */
```

2. `app_main.c`에 PPPoE 인증 정보를 설정합니다:

```c
uint8_t pppoe_id[6] = "W6300";
uint8_t pppoe_id_len = 5;
uint8_t pppoe_pw[7] = "WIZnet";
uint8_t pppoe_pw_len = 6;
```

3. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps).

## 예상 출력

```
=== PPPoE Example ===

QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
==========================================================
 W6300 network configuration
 ...
==========================================================

 PPPoE connecting...

<<<< PPPoE Success >>>>
 Assigned IP address : X.X.X.X

==================================================
    AFTER PPPoE, Net Configuration Information
==================================================
 MAC address    : 0:8:dc:12:34:56
 SUBNET MASK    : X.X.X.X
 G/W IP ADDRESS : X.X.X.X
 SOURCE IP ADDR : X.X.X.X
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `g_net_info` — 초기 네트워크 설정
- `pppoe_id` / `pppoe_id_len` — PPPoE 사용자명
- `pppoe_pw` / `pppoe_pw_len` — PPPoE 비밀번호

## 참고

- PPPoE는 내부적으로 MACRAW 소켓을 사용하므로 별도의 소켓 할당이 필요 없습니다.
- 이 예제에는 DHCP나 타이머 틱이 필요하지 않습니다.
- `PPP_MAX_RETRY_COUNT`는 `PPPoE.h`에 정의되어 있습니다 (기본: 5).
