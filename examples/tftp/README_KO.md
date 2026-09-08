# TFTP Client 예제

[English](README.md) | 한국어

## 개요

네트워크상의 TFTP 서버에서 파일을 읽어오는 TFTP 클라이언트입니다. WIZnet-PICO-C의 tftp 예제 기반입니다.

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (TFTP 서버와 같은 네트워크에 연결)

## 소프트웨어

- [SolarWinds TFTP Server](https://www.solarwinds.com/free-tools/free-tftp-server) 또는 임의의 TFTP 서버

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0      | DHCP  |
| 1      | TFTP  |

## 준비

CubeIDE 프로젝트에는 ioLibrary TFTP 소스가 이미 포함되어 있습니다. 프로젝트를 다시 만드는 경우 `Libraries/ioLibrary_Driver/Internet/TFTP`를 소스 및 인클루드 경로에 추가하십시오.

## 사용법

1. `main.h`에 `EXAMPLE_TFTP`를 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_TFTP
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. `app_main.c`에 TFTP 서버 IP와 파일 이름을 설정합니다:

```c
#define TFTP_SERVER_IP         "192.168.11.2"
#define TFTP_SERVER_FILE_NAME  "tftp_test_file.txt"
```

4. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP와 TFTP 타임아웃 처리를 구동합니다.

5. PC에서 TFTP 서버를 실행하고 루트 디렉터리에 테스트 파일(예: `tftp_test_file.txt`)을 둡니다.

6. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps).

## 예상 출력

```
=== TFTP Client Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 TFTP server IP : 192.168.11.2
 File name      : tftp_test_file.txt
 Sending read request...
 TFTP read success : tftp_test_file.txt
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `TFTP_SERVER_IP` — TFTP 서버 IP 주소
- `TFTP_SERVER_FILE_NAME` — 서버에서 읽어올 파일
- `TFTP_SERVER_PORT` — 서버 포트, `tftp.h`에서 설정 (기본: 69)
