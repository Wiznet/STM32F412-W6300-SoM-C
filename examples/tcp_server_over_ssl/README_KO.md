# TCP Server over SSL 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 TLS 1.2 에코 서버입니다. 들어오는 TLS 클라이언트 연결을 수신 대기하고, TLS 핸드셰이크를 수행한 뒤 환영 메시지를 보내고, 받은 데이터를 암호화된 채널로 되돌려 보냅니다.

mbedTLS 3.6 LTS와 ATECC608C-TNGTLS를 사용해 하드웨어 기반 ECDSA 서명, 장치 인증서 인증, 엔트로피(RNG)를 제공합니다. 서버 인증서와 개인키는 펌웨어에 내장되어 있습니다 (mbedTLS의 ECDSA P-256 테스트 인증서).

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (테스트 클라이언트와 같은 네트워크에 연결)

## 소프트웨어

- PC에서 TLS 클라이언트로 테스트하기 위한 [OpenSSL](https://www.openssl.org/)

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0      | DHCP  |
| 1      | TLS   |

## 사용법

1. `main.h`에 `EXAMPLE_TCP_SERVER_OVER_SSL`을 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_TCP_SERVER_OVER_SSL
/* USER CODE END Private defines */
```

2. 온보드 ATECC608C-TNGTLS 보안 소자를 위해 I2C2가 `.ioc`에 이미 설정되어 있습니다. CubeMX 프로젝트를 재생성한다면 I2C2를 100 kHz로 유지하고 `MX_I2C2_Init()`이 `app_main()`보다 먼저 실행되도록 하십시오.

3. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
// #define NET_MODE    NETINFO_STATIC
```

4. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP 타임아웃 처리를 구동합니다.

5. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps).

6. PC에서 OpenSSL로 서버에 접속합니다:

```bash
openssl s_client -connect 192.168.11.2:443 -tls1_2
```

`192.168.11.2`를 보드의 실제 IP 주소(DHCP 이후 시리얼 터미널에 표시됨)로 바꾸십시오.

메시지를 입력하고 Enter를 누르면 에코되는 것을 볼 수 있습니다.

## 예상 출력

### 시리얼 터미널

```
========================================
 TCP Server over SSL Example
 STM32F412 + W6300 + ATECC608C
========================================

[ATECC608] Init OK
QSPI Mode: QUAD
QSPI DMA threshold: 16 bytes
 W6300 PHY Link UP
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

========================================
 TLS 1.2 Echo Server
========================================
[TLS] RNG seeded OK
[TLS] Server cert loaded
[TLS] Server key loaded
[TLS] Server init complete
[TLS] Listening on port 443

[TLS] Client connected
[TLS] Starting handshake...
[TLS] Handshake OK!
[TLS] Ciphersuite: TLS-ECDHE-ECDSA-WITH-AES-128-GCM-SHA256
[TLS] TLS version: TLSv1.2
[TLS] Received 12 bytes: Hello World
[TLS] Echo sent 12 bytes
```

### OpenSSL 클라이언트

```
CONNECTED(00000003)
---
Hello from W6300 TLS Server
Hello World
Hello World
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `TLS_SERVER_PORT` — 수신 대기 포트 (기본: 443)

## TLS 설정

- **서버 인증서**: 내장된 ECDSA P-256 테스트 인증서 (mbedTLS PolarSSL Test EC). 실제 사용 시에는 `tls_server.c`의 `srv_cert_pem`과 `srv_key_pem`을 직접 만든 인증서로 교체하십시오.
- **클라이언트 인증서 검증**: 비활성화(`VERIFY_NONE`). 서버가 클라이언트 인증서를 요청하지 않습니다.
- **엔트로피 소스**: ATECC608C 하드웨어 RNG (`atcab_random()`), `entropy_atecc608.c`를 통해 mbedTLS에 시드됩니다.

### 서버 인증서 교체

직접 ECDSA P-256 인증서와 키를 생성합니다:

```bash
openssl ecparam -genkey -name prime256v1 -out server_key.pem
openssl req -new -x509 -key server_key.pem -out server.pem -days 365 -subj "/CN=W6300-TLS-Server"
```

그다음 각 PEM 파일을 C 문자열 형식으로 만들어 `tls_server.c`의 `srv_cert_pem` / `srv_key_pem`을 교체합니다. 각 줄은 `"line\r\n"` 형태로 감싸야 합니다.
