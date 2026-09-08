# TCP Client over SSL 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 TLS 1.2 클라이언트입니다. TLS 서버에 접속해 hello 메시지를 보내고, 서버가 연결을 닫을 때까지 받은 데이터를 되돌려 보냅니다.

mbedTLS 3.6 LTS와 ATECC608C-TNGTLS를 사용해 하드웨어 기반 ECDSA 서명, 장치 인증서 인증, 엔트로피(RNG)를 제공합니다. 하부 TCP 연결은 W6300 하드와이어드 TCP/IP 칩이 처리합니다.

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (TLS 서버와 같은 네트워크에 연결)

## 소프트웨어

- PC에서 테스트용 TLS 서버를 실행하기 위한 [OpenSSL](https://www.openssl.org/)

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0      | DHCP  |
| 1      | TLS   |

## 사용법

1. `main.h`에 `EXAMPLE_TCP_CLIENT_OVER_SSL`을 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_TCP_CLIENT_OVER_SSL
/* USER CODE END Private defines */
```

2. 온보드 ATECC608C-TNGTLS 보안 소자를 위해 I2C2가 `.ioc`에 이미 설정되어 있습니다. CubeMX 프로젝트를 재생성한다면 I2C2를 100 kHz로 유지하고 `MX_I2C2_Init()`이 `app_main()`보다 먼저 실행되도록 하십시오.

3. `tls_client.h`에서 mTLS가 기본으로 활성화되어 있으며, ATECC608C 장치 인증서를 클라이언트 인증에 사용하도록 설정합니다. 클라이언트 인증 없이 테스트하려면 `ENABLE_MTLS`를 주석 처리하십시오:

```c
#define ENABLE_MTLS
```

4. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
// #define NET_MODE    NETINFO_STATIC
```

5. `app_main.c`에 TLS 서버 IP를 설정합니다:

```c
static uint8_t g_tls_server_ip[] = {192, 168, 11, 100};
#define TLS_SERVER_PORT        443
```

> `NETINFO_STATIC`을 사용할 때는 서버 주소가 `g_net_info.ip`(기본 `192.168.11.2`)와 달라야 합니다. 둘이 같으면 보드와 서버가 한 주소를 사용하게 되어 핸드셰이크가 시작되지 않습니다.

6. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP 타임아웃 처리를 구동합니다.

7. PC에서 테스트용 서버 인증서를 생성하고 TLS 서버를 실행합니다:

```bash
openssl ecparam -genkey -name prime256v1 -out server_key.pem
openssl req -new -x509 -key server_key.pem -out server.pem -days 365 -subj "/CN=test"
openssl s_server -accept 443 -cert server.pem -key server_key.pem -tls1_2
```

8. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps).

## 예상 출력

### 시리얼 터미널 (mTLS 활성화)

```
========================================
 TCP Client over SSL Example
 STM32F412 + W6300 + ATECC608C-TNGTLS
========================================

[ATECC608] Init OK

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

========================================
 TLS 1.2 Client Echo Test
========================================
[TLS] RNG seeded OK
[TLS] Device cert loaded
[TLS] Signer cert loaded
[TLS] PK -> ATECC608 slot 0
[TLS] mTLS: enabled
[TLS] Init complete
[TLS] Connecting to 192.168.11.100:443 ...
[TLS] TCP connected
[TLS] Starting TLS handshake...
[TLS] Handshake OK!
[TLS] Ciphersuite: TLS-ECDHE-ECDSA-WITH-AES-128-GCM-SHA256
[TLS] TLS version: TLSv1.2
[TLS] Sent 43 bytes: Hello from STM32F412-W6300-SoM TLS Client
```

### 시리얼 터미널 (mTLS 비활성화)

```
[TLS] RNG seeded OK
[TLS] mTLS: disabled
[TLS] Init complete
[TLS] Connecting to 192.168.11.100:443 ...
[TLS] TCP connected
[TLS] Starting TLS handshake...
[TLS] Handshake OK!
[TLS] Ciphersuite: TLS-ECDHE-ECDSA-WITH-AES-128-GCM-SHA256
[TLS] TLS version: TLSv1.2
[TLS] Sent 43 bytes: Hello from STM32F412-W6300-SoM TLS Client
```

### OpenSSL 서버

```bash
# ENABLE_MTLS를 주석 처리해 mTLS 없이 사용하는 경우
openssl s_server -accept 443 -cert server.pem -key server_key.pem -tls1_2

# mTLS 사용 (기본)
openssl s_server -accept 443 -cert server.pem -key server_key.pem -tls1_2 -verify 1
```

```
ACCEPT
-----BEGIN SSL SESSION PARAMETERS-----
...
-----END SSL SESSION PARAMETERS-----
Hello from STM32F412-W6300-SoM TLS Client
```

OpenSSL 서버 터미널에 메시지를 입력하면 보드로 에코됩니다.

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `ENABLE_MTLS` — 기본적으로 `tls_client.h`에 정의되어 있으며, 주석 처리하면 mTLS가 비활성화됩니다
- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `g_tls_server_ip` — TLS 서버 IP 주소
- `TLS_SERVER_PORT` — TLS 서버 포트 (기본: 443)

## TLS 설정

- **서버 인증서 검증**: 비활성화(`VERIFY_NONE`). 활성화하려면 `s_cacert`에 CA 인증서를 로드하고 `tls_client.c`의 `authmode`를 `MBEDTLS_SSL_VERIFY_REQUIRED`로 변경하십시오.
- **mTLS (클라이언트 인증서)**: `tls_client.h`의 `ENABLE_MTLS`로 제어합니다. 활성화하면 ATECC608C-TNGTLS 장치 인증서와 개인키(슬롯 0)가 클라이언트 인증에 사용됩니다. OpenSSL 서버는 클라이언트 인증서를 요청하기 위해 `-verify 1`을 사용해야 합니다.
- **엔트로피 소스**: ATECC608C 하드웨어 RNG (`atcab_random()`), `entropy_atecc608.c`를 통해 mbedTLS에 시드됩니다.
