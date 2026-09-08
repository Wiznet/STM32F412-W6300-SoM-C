# MQTTS (MQTT over TLS) Example

## Overview

MQTT client for the STM32F412 + W6300 SoM. Connects to AWS IoT Core over TLS 1.2 on port 8883, subscribes to a topic, and periodically publishes.

Uses the same Paho Embedded C client as the [`mqtt`](../mqtt) example, with mbedTLS 3.6 LTS underneath instead of raw sockets. The ATECC608C-TNGTLS provides the device certificate, hardware-backed ECDSA signing, and entropy (RNG); the private key stays in slot 0 and is never held in flash.

Based on the [WIZnet-PICO-AWS-C](https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C) aws_iot_mqtt example, reworked for the Paho client and the on-board secure element.

Supports both **DHCP** and **static IP**.

## Hardware

- STM32F412 + W6300 SoM
- Ethernet cable (connected to a network with internet access)

## Software

- An AWS account with AWS IoT Core
- [OpenSSL](https://www.openssl.org/) to check the certificate file (optional; also available in AWS CloudShell)

## Socket Allocation

| Socket | Usage |
|--------|-------|
| 0      | DHCP  |
| 1      | MQTT over TLS |
| 2      | DNS   |

Socket 1 is fixed by `TLS_SOCKET_NUM` in [`port/wizchip_tls.h`](../../port/wizchip_tls.h), which is the socket the mbedTLS BIO callbacks read and write.

## How to Use

1. Define `EXAMPLE_MQTTS` in `main.h`:

```c
/* USER CODE BEGIN Private defines */
#define EXAMPLE_MQTTS
/* USER CODE END Private defines */
```

2. I2C2 is already configured in the `.ioc` for the on-board ATECC608C-TNGTLS secure element. If you regenerate the CubeMX project, keep I2C2 enabled at 100 kHz and make sure `MX_I2C2_Init()` runs before `app_main()`.

3. Build, flash, and open a serial terminal (115200 bps). `ENABLE_CERT_DUMP` in `mqtts_transport.c` is on by default, so the board prints its own certificate at boot:

```
[CERT] device subject : O=Microchip Technology Inc, CN=sn01236103D443C9DC01
-----BEGIN CERTIFICATE-----
MIICIDCCAcWgAwIBAgIQW1PcrZK/i0yxnq0obf9rKTAKBggqhkjOPQQDAjBPMSEw
...
-----END CERTIFICATE-----
```

Save the block, `-----BEGIN CERTIFICATE-----` through `-----END CERTIFICATE-----`, as `device.pem`.

> The board also sends the Microchip signer that issued this certificate, so the broker can walk the chain up to the Microchip root. The signer is not what you register, so it is not printed; set `ENABLE_SIGNER_DUMP` to `1` if you need it, for example to register it as a CA.

> Each board has its own certificate and a locked private key, so a certificate generated in the AWS console cannot be used. Register this one instead.

> Serial terminals usually prefix each line with a timestamp or an `RX` marker. Remove them; the file must contain nothing but the PEM block.

Check the saved file:

```bash
openssl x509 -in device.pem -noout -subject -dates
```

```
subject=O=Microchip Technology Inc, CN=sn01236103D443C9DC01
notBefore=Jul 26 14:00:00 2026 GMT
notAfter=Jul 26 14:00:00 2054 GMT
```

A bad save reports `Could not find certificate` instead.

4. Register `device.pem` with AWS IoT, either in the console or in CloudShell.

**Console**: **Security -> Certificates -> Add certificate -> Register certificates**, choose the option for a certificate whose CA is not registered with AWS IoT, upload `device.pem`, then **Activate** it.

**CloudShell** (bottom-left of the AWS console; already signed in, AWS CLI installed). Upload `device.pem` with **Actions -> Upload file**, then:

```bash
aws iot register-certificate-without-ca \
  --certificate-pem file://device.pem \
  --status ACTIVE \
  --region ap-southeast-2
```

The command prints the `certificateArn` used in the next step. Use your own region here and below.

5. Create a policy and attach it to the certificate. A permissive policy is enough for a first run:

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

In the console: select the certificate, then **Attach policy**.

> Policies attach to certificates, not to things.

6. Create a thing and attach the certificate to it, under **Manage -> Things** or with `aws iot attach-thing-principal`. Not needed to connect with the policy above, but Device Shadow topics require a thing.

7. Set your account endpoint in `app_main.c`:

```c
#define MQTT_BROKER_DOMAIN     "account-specific-prefix-ats.iot.ap-northeast-2.amazonaws.com"
```

Found in the AWS IoT console under **Settings -> Device data endpoint**, or:

```bash
aws iot describe-endpoint --endpoint-type iot:Data-ATS
```

8. Set the client ID and topics in `app_main.c`:

```c
#define MQTT_CLIENT_ID         "w6300-som"
#define MQTT_PUBLISH_TOPIC     "$aws/things/w6300-som/shadow/update"
#define MQTT_SUBSCRIBE_TOPIC   "$aws/things/w6300-som/shadow/update/accepted"
```

> The client ID only has to be unique among live connections. It is the thing name inside the shadow topics that selects the shadow, so that is the part to match against a thing you have.

9. Select network mode in `app_main.c`:

```c
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC
```

10. `Core/Src/stm32f4xx_it.c` already calls `app_timer_tick()` from `SysTick_Handler()`, which drives DHCP, DNS, and MQTT timing.

11. Rebuild, flash, and subscribe to the publish topic from the console's **MQTT test client** to watch the messages arrive.

## Expected Output

### Serial Terminal

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

The certificate dump between `[TLS] Signer cert loaded` and `[TLS] Private key` is omitted above; see step 3.

## Configuration

The following can be modified in `app_main.c`:

- `NET_MODE` - `NETINFO_DHCP` or `NETINFO_STATIC`
- `g_net_info` - MAC, static IP, gateway, subnet
- `MQTT_BROKER_DOMAIN` - AWS IoT data endpoint (ATS)
- `PORT_MQTTS` - broker port (default: 8883)
- `MQTT_CLIENT_ID` - MQTT connection identifier
- `MQTT_PUBLISH_TOPIC` / `MQTT_SUBSCRIBE_TOPIC` / `MQTT_PUBLISH_PAYLOAD`
- `MQTT_PUBLISH_PERIOD` - publish interval in ms (default: 10000)
- `MQTT_KEEP_ALIVE` - keep-alive in seconds (default: 60)
- `MQTT_YIELD_TIMEOUT` - `MQTTYield()` timeout in ms (default: 100)
- `MQTT_RECONNECT_DELAY` - delay between reconnect attempts in ms (default: 5000)

In `mqtts_transport.h`:

- `ENABLE_MTLS` - present the ATECC608C device certificate. AWS IoT Core always requires a client certificate; comment it out only for a different broker.

In `mqtts_transport.c`:

- `ENABLE_CERT_DUMP` - print the device certificate at boot. Turn it off once the board is registered.
- `ENABLE_SIGNER_DUMP` - also print the Microchip signer certificate, `0` by default. Only the device certificate is registered with a broker, so this stays off to keep the dump unambiguous.
- `TLS_DEBUG_LEVEL` - mbedTLS logging, `0` by default. Level 1 prints the non-blocking `WANT_READ` path hundreds of times a second.

In `mqtt_certificate.h`:

- `mqtt_root_ca` - Amazon Root CA 1. ATS endpoints chain to this root; legacy endpoints need a different one.

## TLS Settings

- **Server certificate verification**: enabled (`VERIFY_REQUIRED`) against `mqtt_root_ca` in `mqtt_certificate.h`. The endpoint hostname is sent as SNI, which AWS IoT requires.
- **mTLS (client certificate)**: controlled by `ENABLE_MTLS` in `mqtts_transport.h`. The ATECC608C-TNGTLS device certificate and private key (slot 0) are used for client authentication. AWS IoT Core always requires one.
- **Entropy source**: ATECC608C hardware RNG (`atcab_random()`), seeded into mbedTLS via `entropy_atecc608.c`.

## Troubleshooting

**Handshake succeeds, then the connection drops before CONNACK.** The identity is not authorised, not a TLS fault. AWS closes the session without a reason code. Check, in order:

1. The board's certificate is registered. Its `certificateId` is the SHA-256 of the DER, so you can look it up: `openssl x509 -in device.pem -outform DER | openssl dgst -sha256`
2. Its status is `ACTIVE`, not `PENDING_ACTIVATION` or `INACTIVE`.
3. A policy is attached to the certificate.
4. The policy allows this client ID. With `client/${iot:Connection.Thing.ThingName}`, `MQTT_CLIENT_ID` must equal the thing name and the thing must be attached to the certificate.

Enable AWS IoT logging under **Settings -> Logs** to see the reason in the CloudWatch log group `AWSIotLogsV2`.

**Shadow topics rejected.** `$aws/things/<name>/shadow/...` needs a registered thing named `<name>`. For a first test use a plain topic such as `w6300/test`.

**Handshake fails with `-0x2700`** (`X509_CERT_VERIFY_FAILED`). The broker certificate did not verify against `mqtt_root_ca`.

**`calib_read_zone - execution failed` at boot.** An occasional I2C read failure while the ATECC608C is still waking after reset. Harmless if the certificate loads afterwards; if it repeats every boot, increase the delay before `atecc608_init()`.
