# MQTT Publish & Subscribe 예제

[English](README.md) | 한국어

## 개요

STM32F412 + W6300 SoM용 MQTT 클라이언트입니다. MQTT 브로커에 접속해 토픽을 구독하고 주기적으로 메시지를 발행합니다. WIZnet-PICO-C의 mqtt publish_subscribe 예제 기반입니다.

ioLibrary_Driver에 포함된 Paho MQTT Embedded C 라이브러리를 사용합니다.

**DHCP**와 **고정 IP** 모두 지원합니다.

## 하드웨어

- STM32F412 + W6300 SoM
- 이더넷 케이블 (MQTT 브로커와 같은 네트워크에 연결)

## 소프트웨어

- PC에서 실행되는 [Mosquitto](https://mosquitto.org/) MQTT 브로커

## 소켓 할당

| 소켓 | 용도 |
|--------|-------|
| 0      | DHCP  |
| 1      | MQTT  |

## 사용법

1. `main.h`에 `EXAMPLE_MQTT`를 정의합니다:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_MQTT
/* USER CODE END Private defines */
```

2. `app_main.c`에서 네트워크 모드를 선택합니다:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

3. `app_main.c`에 MQTT 브로커 IP를 설정합니다:

```c
static uint8_t g_mqtt_broker_ip[4] = {192, 168, 11, 100};
```

> `NETINFO_STATIC`을 사용할 때는 브로커 주소가 `g_net_info.ip`(기본 `192.168.11.2`)와 달라야 합니다. 둘이 같으면 보드와 브로커가 한 주소를 사용하게 되어 연결이 완료되지 않습니다.

4. CubeIDE 프로젝트에는 ioLibrary MQTT 소스가 이미 포함되어 있습니다. 프로젝트를 다시 만드는 경우 `Libraries/ioLibrary_Driver/Internet/MQTT`를 소스 및 인클루드 경로에 추가하십시오.

5. `Core/Src/stm32f4xx_it.c`가 이미 `SysTick_Handler()`에서 `app_timer_tick()`을 호출하며, 이것이 DHCP와 MQTT 타이밍을 구동합니다.

6. PC에서 Mosquitto 브로커를 실행합니다:

```bash
mosquitto -c mosquitto.conf -v
```

7. 빌드·플래시하고 시리얼 터미널을 엽니다 (115200 bps).

8. 다른 터미널에서 발행 토픽을 구독해 메시지를 확인합니다:

```bash
mosquitto_sub -h localhost -t publish_topic
```

9. 구독 토픽으로 발행해 보드에 메시지를 보냅니다:

```bash
mosquitto_pub -h localhost -t subscribe_topic -m "Hello W6300"
```

## 예상 출력

### 시리얼 터미널

```
=== MQTT Publish & Subscribe Example ===

 DHCP client running
 DHCP success
==========================================================
 W6300 network configuration
 ...
==========================================================

 MQTT connected
 Subscribed to 'subscribe_topic'
 Publishing to 'publish_topic' every 10 seconds

 Published
 Published
 [SUB] Hello W6300
```

### Mosquitto 구독자

```
Hello, World!
Hello, World!
```

## 설정 항목

`app_main.c`에서 수정할 수 있는 것들:

- `NET_MODE` — `NETINFO_DHCP` 또는 `NETINFO_STATIC`
- `g_net_info` — MAC, 고정 IP, 게이트웨이, 서브넷
- `g_mqtt_broker_ip` — MQTT 브로커 IP 주소
- `PORT_MQTT` — 브로커 포트 (기본: 1883)
- `MQTT_CLIENT_ID` — 클라이언트 식별자
- `MQTT_USERNAME` / `MQTT_PASSWORD` — 브로커 인증 정보
- `MQTT_PUBLISH_TOPIC` / `MQTT_SUBSCRIBE_TOPIC` — 토픽 이름
- `MQTT_PUBLISH_PAYLOAD` — 메시지 내용
- `MQTT_PUBLISH_PERIOD` — 발행 주기, ms (기본: 10000)
- `MQTT_KEEP_ALIVE` — keep-alive 주기, 초 (기본: 60)
- `MQTT_YIELD_TIMEOUT` — `MQTTYield()` 한 번이 수신 패킷을 기다리는 시간, ms (기본: 100)
- `MQTT_RECONNECT_DELAY` — 재접속 시도 간격, ms (기본: 5000)

## 재접속

이 예제는 세션을 스스로 유지합니다. 매 루프 반복마다 소켓 상태를 확인하며, 연결 끊김·yield 오류·발행 실패가 발생하면 소켓을 닫고 `MQTT_RECONNECT_DELAY` ms마다 `MQTTConnect` + `MQTTSubscribe`를 재시도합니다:

```
 Connection lost
 Network connect failed
 Retry in 5 seconds
 MQTT connected
 Subscribed to 'subscribe_topic'
```

소켓 상태를 명시적으로 확인해야 하는 이유는, Paho 임베디드 클라이언트가 닫힌 소켓을 오류가 아니라 "패킷이 도착하지 않음"으로 보고하기 때문입니다.

DHCP도 계속 동작합니다. 최초 임대 이후 메인 루프가 초당 한 번 `DHCP_run()`을 처리해 임대를 갱신하며, 서버가 다른 주소를 할당하면 MQTT 세션을 닫고 새 주소로 다시 엽니다:

```
 DHCP IP changed
 MQTT connected
```

*최초* 임대가 `DHCP_RETRY_COUNT`번 시도 후에도 실패하면, 다른 예제들과 마찬가지로 예제가 정지합니다.

## 참고

Mosquitto 2.0 이상은 인증 설정을 명시해야 합니다. 다음 내용으로 `mosquitto.conf`를 만드십시오:

```
listener 1883
allow_anonymous true
```
