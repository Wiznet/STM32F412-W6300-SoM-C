/**
 * @file    mqtts_transport.h
 * @brief   TLS transport for the Paho MQTT embedded client on the W6300.
 *
 * @details The ioLibrary MQTT client talks to the network through three
 *          callbacks in its Network struct. The plain mqtt example points
 *          them at recv()/send(); this one points them at mbedTLS, so the
 *          same MQTT client speaks MQTTS.
 *
 *          The board authenticates with the ATECC608C-TNGTLS device
 *          certificate. Its private key lives in slot 0 and never leaves the
 *          secure element -- no key material is stored in flash.
 */

#ifndef __MQTTS_TRANSPORT_H__
#define __MQTTS_TRANSPORT_H__

#include <stdint.h>
#include <stddef.h>
#include "MQTTClient.h"

/* Comment out to authenticate the broker only, without presenting a client
 * certificate. AWS IoT Core always requires a client certificate, so leave
 * this enabled unless you are pointing the example at a different broker. */
#define ENABLE_MTLS

/**
 * @brief  Set up mbedTLS: RNG from the ATECC608C, the trusted root, and
 *         (with ENABLE_MTLS) the device certificate and its key handle.
 * @return 0 on success, negative mbedTLS error code otherwise.
 */
int mqtts_transport_init(void);

/**
 * @brief  Tell the transport which buffer Paho reads into, so it can refuse a
 *         read that would run past the end of it.
 *
 * @details readPacket() in the MQTT client reads the remaining length off the
 *          wire and passes it straight to this transport without checking it
 *          against the buffer it was given, so a broker packet larger than the
 *          buffer would overrun it. Call this with the same buffer and size
 *          passed to MQTTClientInit(); without it the check is inactive.
 *
 * @param  buf  Receive buffer handed to MQTTClientInit().
 * @param  size Its size in bytes.
 */
void mqtts_transport_set_recv_buf(unsigned char *buf, size_t size);

/**
 * @brief  Open the TCP socket, run the TLS handshake, and wire @p n up to
 *         the resulting session.
 * @param  n         Paho Network to populate.
 * @param  server_ip Broker address, already resolved.
 * @param  port      Broker port (8883 for AWS IoT Core).
 * @param  hostname  Sent as SNI and verified against the certificate.
 *                   AWS IoT Core requires it; never pass NULL there.
 * @return 0 on success, negative on failure.
 */
int mqtts_transport_connect(Network *n, uint8_t *server_ip, uint16_t port,
                            const char *hostname);

/**
 * @brief  Tear the session down and release the socket. Safe to call on a
 *         session that never came up.
 */
void mqtts_transport_close(Network *n);

#endif /* __MQTTS_TRANSPORT_H__ */
