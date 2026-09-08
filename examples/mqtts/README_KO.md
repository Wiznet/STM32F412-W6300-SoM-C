# MQTTS (MQTT over TLS) 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 MQTT 클라이언트입니다. AWS IoT Core에 TLS 1.2, 8883 포트로 접속해 토픽을 구독하고 주기적으로 발행합니다.

[`mqtt`](../mqtt) 예제와 동일한 Paho Embedded C 클라이언트를 사용하되, raw 소켓 대신 mbedTLS 3.6 LTS를 아래에 둡니다. ATECC608C-TNGTLS가 장치 인증서, 하드웨어 기반 ECDSA 서명, 엔트로피(RNG)를 제공하며, 개인키는 슬롯 0에 머물고 플래시에 저장되지 않습니다.

[WIZnet-PICO-AWS-C](https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C)의 aws_iot_mqtt 예제 기반이며, Paho 클라이언트와 온보드 보안 소자에 맞게 재작업했습니다.

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (인터넷이 되는 네트워크에 연결)

## 소프트웨어

- AWS IoT Core를 사용할 수 있는 AWS 계정
- 인증서 파일 확인용 [OpenSSL](https://www.openssl.org/) (선택 사항, AWS CloudShell에도 포함)

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0      | DHCP  |
| 1      | MQTT over TLS |
| 2      | DNS   |

소켓 1은 [`port/wizchip_tls.h`](../../port/wizchip_tls.h)의 `TLS_SOCKET_NUM`으로 고정되며, mbedTLS BIO 콜백이 읽고 쓰는 소켓입니다.

## 사용법

1. `main.h`에 `EXAMPLE_MQTTS`를 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_MQTTS
/* USER CODE END Private defines */
```

2. 온보드 ATECC608C-TNGTLS 보안 소자를 위해 I2C2가 `.ioc`에 이미 설정되어 있습니다. CubeMX 프로젝트를 재생성한다면 I2C2를 100 kHz로 유지하고 `MX_I2C2_Init()`이 `app_main()`보다 먼저 실행되도록 하십시오.

3. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps). `mqtts_transport.c`의 `ENABLE_CERT_DUMP`가 기본으로 켜져 있어, 보드가 부팅 시 자기 인증서를 출력합니다:

```
[CERT] device subject : O=Microchip Technology Inc, CN=sn01236103D443C9DC01
-----BEGIN CERTIFICATE-----
MIICIDCCAcWgAwIBAgIQW1PcrZK/i0yxnq0obf9rKTAKBggqhkjOPQQDAjBPMSEw
...
-----END CERTIFICATE-----
```

출력된 블록을 `-----BEGIN CERTIFICATE-----`부터 `-----END CERTIFICATE-----`까지 `device.pem`으로 저장합니다.

> 보드는 이 인증서를 발급한 Microchip signer도 함께 전송하여, 브로커가 Microchip 루트까지 체인을 따라갈 수 있게 합니다. signer는 등록 대상이 아니므로 출력하지 않습니다. CA로 등록하는 경우처럼 signer가 필요하면 `ENABLE_SIGNER_DUMP`를 `1`로 설정하십시오.

> 보드마다 인증서가 다르고 개인키가 잠겨 있으므로, AWS 콘솔에서 생성한 인증서는 사용할 수 없습니다. 이 인증서를 등록하십시오.

> 시리얼 터미널은 보통 각 줄 앞에 타임스탬프나 `RX` 표시를 붙입니다. 제거해야 하며, 파일에는 PEM 블록 외에 아무것도 없어야 합니다.

저장한 파일을 확인합니다:

```bash
openssl x509 -in device.pem -noout -subject -dates
```

```
subject=O=Microchip Technology Inc, CN=sn01236103D443C9DC01
notBefore=Jul 26 14:00:00 2026 GMT
notAfter=Jul 26 14:00:00 2054 GMT
```

잘못 저장된 경우에는 대신 `Could not find certificate`가 나옵니다.

4. `device.pem`을 AWS IoT에 등록합니다. 콘솔이나 CloudShell 중 하나를 사용합니다.

**콘솔**: **Security → Certificates → Add certificate → Register certificates**에서, CA가 AWS IoT에 등록되지 않은 인증서에 해당하는 옵션을 선택하고 `device.pem`을 업로드한 뒤 **Activate** 합니다.

**CloudShell** (AWS 콘솔 왼쪽 아래. 이미 로그인되어 있고 AWS CLI가 설치되어 있습니다). **Actions → Upload file**로 `device.pem`을 올린 다음:

```bash
aws iot register-certificate-without-ca \
  --certificate-pem file://device.pem \
  --status ACTIVE \
  --region ap-southeast-2
```

이 명령이 다음 단계에서 사용할 `certificateArn`을 출력합니다. 여기와 아래의 리전은 본인 것으로 바꾸십시오.

5. 정책을 만들어 인증서에 붙입니다. 첫 실행에는 관대한 정책으로 충분합니다:

```json
{
  "Version": "2012-10-17",
  "Statement": [{
    "Effect": "Allow",
    "Action": ["iot:Connect", "iot:Publish", "iot:Subscribe", "iot:Receive"],
    "Resource": "*"
  }]
}
```

```bash
aws iot attach-policy --policy-name <your-policy> --target <certificateArn> --region ap-southeast-2
```

콘솔에서는 인증서를 선택한 뒤 **Attach policy**를 사용합니다.

> 정책은 thing이 아니라 인증서에 붙습니다.

6. **Manage → Things**에서 또는 `aws iot attach-thing-principal`로 thing을 만들고 인증서를 붙입니다. 위 정책으로 접속하는 데는 필요 없지만, Device Shadow 토픽에는 thing이 필요합니다.

7. `app_main.c`에 본인 계정의 엔드포인트를 설정합니다:

```c
#define MQTT_BROKER_DOMAIN     "account-specific-prefix-ats.iot.ap-northeast-2.amazonaws.com"
```

AWS IoT 콘솔의 **Settings → Device data endpoint**에서 찾거나:

```bash
aws iot describe-endpoint --endpoint-type iot:Data-ATS
```

8. `app_main.c`에서 client ID와 토픽을 설정합니다:

```c
#define MQTT_CLIENT_ID         "w6300-som"
#define MQTT_PUBLISH_TOPIC     "$aws/things/w6300-som/shadow/update"
#define MQTT_SUBSCRIBE_TOPIC   "$aws/things/w6300-som/shadow/update/accepted"
```

> client ID는 살아 있는 연결들 사이에서만 유일하면 됩니다. 어느 shadow를 선택할지 결정하는 것은 shadow 토픽 안의 thing 이름이므로, 보유한 thing과 맞춰야 하는 것은 그쪽입니다.

9. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

10. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP, DNS, MQTT 타이밍을 구동합니다.

11. 다시 빌드·플래시한 뒤, 콘솔의 **MQTT test client**에서 발행 토픽을 구독해 메시지가 도착하는지 확인합니다.

## 예상 출력

### 시리얼 터미널

```
========================================
 MQTTS Example (AWS IoT Core)
 STM32F412 + W6300 + ATECC608C-TNGTLS
========================================

[ATECC608] Init OK
[ATECC608] Serial Number: 01 23 61 03 D4 43 C9 DC 01
 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 Resolving account-specific-prefix-ats.iot.ap-northeast-2.amazonaws.com
 Broker IP : 13.124.xxx.xxx

[TLS] RNG seeded from ATECC608C
[TLS] Root CA loaded
[TLS] Device cert loaded
[TLS] Signer cert loaded
[TLS] Private key -> ATECC608C slot 0
[TLS] Init complete
[TLS] Connecting to 13.124.xxx.xxx:8883
[TLS] TCP connected
[TLS] Handshake...
[TLS] Handshake OK (TLSv1.2, TLS-ECDHE-RSA-WITH-AES-128-GCM-SHA256)
 MQTT connected over TLS
 Subscribed to '$aws/things/w6300-som/shadow/update/accepted'
 Publishing to '$aws/things/w6300-som/shadow/update' every 10 seconds

 Published
 [SUB] {"state":{"reported":{"hello":"w6300"}},"metadata":{...},"version":1}
```

`[TLS] Signer cert loaded`와 `[TLS] Private key` 사이의 인증서 덤프는 위에서 생략했습니다. 3단계를 참고하십시오.

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `MQTT_BROKER_DOMAIN` — AWS IoT 데이터 엔드포인트 (ATS)
- `PORT_MQTTS` — 브로커 포트 (기본: 8883)
- `MQTT_CLIENT_ID` — MQTT 연결 식별자
- `MQTT_PUBLISH_TOPIC` / `MQTT_SUBSCRIBE_TOPIC` / `MQTT_PUBLISH_PAYLOAD`
- `MQTT_PUBLISH_PERIOD` — 발행 주기, ms (기본: 10000)
- `MQTT_KEEP_ALIVE` — keep-alive, 초 (기본: 60)
- `MQTT_YIELD_TIMEOUT` — `MQTTYield()` 타임아웃, ms (기본: 100)
- `MQTT_RECONNECT_DELAY` — 재접속 시도 간격, ms (기본: 5000)

`mqtts_transport.h`에서:

- `ENABLE_MTLS` — ATECC608C 장치 인증서를 제시합니다. AWS IoT Core는 항상 클라이언트 인증서를 요구하므로, 다른 브로커를 쓸 때만 주석 처리하십시오.

`mqtts_transport.c`에서:

- `ENABLE_CERT_DUMP` — 부팅 시 장치 인증서를 출력합니다. 보드 등록이 끝나면 끄십시오.
- `ENABLE_SIGNER_DUMP` — Microchip signer 인증서도 함께 출력합니다. 기본 `0`. 브로커에 등록하는 것은 장치 인증서뿐이므로, 출력을 헷갈리지 않게 꺼 둡니다.
- `TLS_DEBUG_LEVEL` — mbedTLS 로깅, 기본 `0`. 레벨 1은 논블로킹 `WANT_READ` 경로를 초당 수백 번 출력합니다.

`mqtt_certificate.h`에서:

- `mqtt_root_ca` — Amazon Root CA 1. ATS 엔드포인트는 이 루트로 이어지며, 구형 엔드포인트는 다른 것이 필요합니다.

## TLS 설정

- **서버 인증서 검증**: 활성화(`VERIFY_REQUIRED`), `mqtt_certificate.h`의 `mqtt_root_ca`를 사용합니다. 엔드포인트 호스트명이 SNI로 전송되며 AWS IoT가 이를 요구합니다.
- **mTLS (클라이언트 인증서)**: `mqtts_transport.h`의 `ENABLE_MTLS`로 제어합니다. ATECC608C-TNGTLS 장치 인증서와 개인키(슬롯 0)가 클라이언트 인증에 사용됩니다. AWS IoT Core는 항상 이를 요구합니다.
- **엔트로피 소스**: ATECC608C 하드웨어 RNG (`atcab_random()`), `entropy_atecc608.c`를 통해 mbedTLS에 시드됩니다.

## 문제 해결

**핸드셰이크는 성공하는데 CONNACK 전에 연결이 끊깁니다.** TLS 결함이 아니라 신원이 인가되지 않은 것입니다. AWS는 사유 코드 없이 세션을 닫습니다. 순서대로 확인하십시오:

1. 보드의 인증서가 등록되어 있는지. `certificateId`는 DER의 SHA-256이므로 직접 찾아볼 수 있습니다: `openssl x509 -in device.pem -outform DER | openssl dgst -sha256`
2. 상태가 `PENDING_ACTIVATION`이나 `INACTIVE`가 아니라 `ACTIVE`인지.
3. 정책이 인증서에 붙어 있는지.
4. 정책이 이 client ID를 허용하는지. `client/${iot:Connection.Thing.ThingName}`을 사용한다면 `MQTT_CLIENT_ID`가 thing 이름과 같아야 하고 그 thing이 인증서에 붙어 있어야 합니다.

**Settings → Logs**에서 AWS IoT 로깅을 활성화하면 CloudWatch 로그 그룹 `AWSIotLogsV2`에서 사유를 확인할 수 있습니다.

**shadow 토픽이 거부됩니다.** `$aws/things/<name>/shadow/...`는 `<name>`이라는 이름의 등록된 thing이 필요합니다. 첫 테스트에는 `w6300/test` 같은 평범한 토픽을 사용하십시오.

**`-0x2700`으로 핸드셰이크가 실패합니다** (`X509_CERT_VERIFY_FAILED`). 브로커 인증서가 `mqtt_root_ca`로 검증되지 않았습니다.

**부팅 시 `calib_read_zone - execution failed`.** 리셋 후 ATECC608C가 깨어나는 중에 이따금 발생하는 I2C 읽기 실패입니다. 이후 인증서가 로드되면 무해합니다. 매 부팅마다 반복되면 `atecc608_init()` 앞의 지연을 늘리십시오.
