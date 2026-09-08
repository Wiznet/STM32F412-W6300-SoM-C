/**
 * @file    mqtts_transport.c
 * @brief   mbedTLS transport behind the Paho MQTT embedded client.
 *
 * @details Two things here are not just "tls_client.c with a different name":
 *
 *          1. The socket is switched to non-blocking before the handshake.
 *             ioLibrary's recv() spins until data arrives in blocking mode,
 *             which would park MQTTYield() forever and stall the keep-alive
 *             and publish timers. Non-blocking recv() returns SOCK_BUSY,
 *             which wizchip_tls_recv() maps to MBEDTLS_ERR_SSL_WANT_READ.
 *
 *          2. Reads and writes honour the timeout Paho passes in. TLS records
 *             do not line up with the MQTT byte counts Paho asks for, so both
 *             directions loop until the requested length is transferred or the
 *             deadline passes.
 */

#include "main.h"

#ifdef EXAMPLE_MQTTS

#include <stdio.h>
#include <string.h>

#include "mqtts_transport.h"
#include "mqtt_certificate.h"
#include "wizchip_tls.h"
#include "entropy_atecc608.h"
#include "socket.h"

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/debug.h"
#include "mbedtls/error.h"
#include "mbedtls/base64.h"

#ifdef ENABLE_MTLS
#include "cryptoauthlib.h"
#include "tng_atca.h"
#include "atca_mbedtls_wrap.h"
#endif

/* ============================================================ */
/* Tunables                                                      */
/* ============================================================ */
#define TLS_HANDSHAKE_TIMEOUT   (1000 * 30)  /* ms */

/* 0 = silent. Raise to 1-4 for mbedTLS internals; note that with a
 * non-blocking socket level 1 already floods the console with the
 * WANT_READ path, which drowns the example's own messages. */
#define TLS_DEBUG_LEVEL         0

/* Print the certificate chain the board presents, as PEM, at init.
 * Paste it into a file to register the device with a cloud broker, and to
 * confirm the registered certificate really is this board's. */
#define ENABLE_CERT_DUMP

/* Paho hands sendPacket() whatever is left of its command timer, which can
 * be 0. A write that gives up immediately would fail the CONNECT, so give
 * outbound transfers a floor. */
#define TLS_WRITE_MIN_TIMEOUT   (1000 * 5)   /* ms */

/* ============================================================ */
/* mbedTLS state                                                 */
/* ============================================================ */
static mbedtls_ssl_context      s_ssl;
static mbedtls_ssl_config       s_conf;
static mbedtls_x509_crt         s_cacert;
static mbedtls_x509_crt         s_clicert;
static mbedtls_pk_context       s_pkey;
static mbedtls_entropy_context  s_entropy;
static mbedtls_ctr_drbg_context s_ctr_drbg;

static uint8_t s_initialised = 0;
static uint8_t s_socket_open = 0;

#ifdef ENABLE_CERT_DUMP
/* Dump one certificate as PEM, plus its subject. mbedTLS already holds the
 * DER that came out of the ATECC608C, so this needs no extra chip access. */
static void dump_cert(const char *label, const mbedtls_x509_crt *crt)
{
    static unsigned char b64[1024];
    char dn[160];
    size_t olen = 0;
    size_t i;
    int ret;

    if (crt == NULL || crt->raw.p == NULL || crt->raw.len == 0)
        return;

    if (mbedtls_x509_dn_gets(dn, sizeof(dn), &crt->subject) > 0)
        printf("[CERT] %s subject : %s\r\n", label, dn);

    ret = mbedtls_base64_encode(b64, sizeof(b64), &olen, crt->raw.p, crt->raw.len);
    if (ret != 0)
    {
        printf("[CERT] %s encode failed : -0x%04X (DER %u bytes)\r\n",
               label, (unsigned)-ret, (unsigned)crt->raw.len);
        return;
    }

    printf("-----BEGIN CERTIFICATE-----\r\n");
    for (i = 0; i < olen; i += 64)
    {
        size_t n = ((olen - i) > 64) ? 64 : (olen - i);
        printf("%.*s\r\n", (int)n, (const char *)&b64[i]);
    }
    printf("-----END CERTIFICATE-----\r\n\r\n");
}
#endif

static void tls_debug_cb(void *ctx, int level,
                         const char *file, int line, const char *str)
{
    (void)ctx;
    (void)level;

    const char *p = file;
    const char *slash = strrchr(file, '/');
    if (slash) p = slash + 1;
    const char *bslash = strrchr(p, '\\');
    if (bslash) p = bslash + 1;

    printf("[mbedTLS] %s:%d: %s", p, line, str);
}

/* ============================================================ */
/* Paho Network callbacks                                        */
/* ============================================================ */
static int mqtts_read(Network *n, unsigned char *buf, int len, long timeout_ms)
{
    (void)n;

    uint32_t start = HAL_GetTick();
    int got = 0;

    if (len <= 0)
        return 0;

    if (timeout_ms < 0)
        timeout_ms = 0;

    for (;;)
    {
        int ret = mbedtls_ssl_read(&s_ssl, buf + got, (size_t)(len - got));

        if (ret > 0)
        {
            got += ret;

            if (got >= len)
                return got;
        }
        else if (ret == MBEDTLS_ERR_SSL_WANT_READ ||
                 ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            /* Nothing decrypted yet -- fall through to the deadline check. */
        }
        else if (ret == 0 || ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
        {
            /* Peer closed. Report it as an error so the caller reconnects
             * rather than treating it as "no packet this time". */
            return (got > 0) ? got : -1;
        }
        else
        {
            return ret;
        }

        if ((HAL_GetTick() - start) >= (uint32_t)timeout_ms)
            return got;   /* short read; Paho treats this as "no packet" */
    }
}

static int mqtts_write(Network *n, unsigned char *buf, int len, long timeout_ms)
{
    (void)n;

    uint32_t start = HAL_GetTick();
    int sent = 0;

    if (len <= 0)
        return 0;

    if (timeout_ms < TLS_WRITE_MIN_TIMEOUT)
        timeout_ms = TLS_WRITE_MIN_TIMEOUT;

    while (sent < len)
    {
        int ret = mbedtls_ssl_write(&s_ssl, buf + sent, (size_t)(len - sent));

        if (ret > 0)
        {
            /* A single record is capped at MBEDTLS_SSL_OUT_CONTENT_LEN, so a
             * short write is normal rather than an error. */
            sent += ret;
            continue;
        }

        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            return ret;

        if ((HAL_GetTick() - start) >= (uint32_t)timeout_ms)
            break;
    }

    return sent;
}

static void mqtts_net_disconnect(Network *n)
{
    mqtts_transport_close(n);
}

/* ============================================================ */
/* Init                                                          */
/* ============================================================ */
int mqtts_transport_init(void)
{
    int ret;

    if (s_initialised)
        return 0;

    /* ---- 1. Entropy and DRBG, seeded from the ATECC608C ---- */
    mbedtls_entropy_init(&s_entropy);
    mbedtls_ctr_drbg_init(&s_ctr_drbg);

    ret = mbedtls_entropy_add_source(&s_entropy, entropy_atecc608_source,
                                     NULL, 32, MBEDTLS_ENTROPY_SOURCE_STRONG);
    if (ret != 0)
    {
        printf("[TLS] entropy_add_source failed : -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }

    ret = mbedtls_ctr_drbg_seed(&s_ctr_drbg, mbedtls_entropy_func, &s_entropy,
                                (const unsigned char *)"mqtts", 5);
    if (ret != 0)
    {
        printf("[TLS] ctr_drbg_seed failed : -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }
    printf("[TLS] RNG seeded from ATECC608C\r\n");

    /* ---- 2. Trusted root for the broker ---- */
    mbedtls_x509_crt_init(&s_cacert);

    /* The length must include the NUL terminator for PEM input. */
    ret = mbedtls_x509_crt_parse(&s_cacert,
                                 (const unsigned char *)mqtt_root_ca,
                                 sizeof(mqtt_root_ca));
    if (ret != 0)
    {
        printf("[TLS] root CA parse failed : -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }
    printf("[TLS] Root CA loaded\r\n");

    /* ---- 3. Device identity from the ATECC608C ---- */
    mbedtls_x509_crt_init(&s_clicert);
    mbedtls_pk_init(&s_pkey);

#ifdef ENABLE_MTLS
    {
        const atcacert_def_t *device_cert_def = NULL;

        ret = tng_get_device_cert_def(&device_cert_def);
        if (ret != ATCA_SUCCESS)
        {
            printf("[TLS] tng_get_device_cert_def failed : 0x%02X\r\n", ret);
            return -1;
        }

        ret = atca_mbedtls_cert_add(&s_clicert, device_cert_def);
        if (ret != 0)
        {
            printf("[TLS] device cert failed : %d\r\n", ret);
            return -1;
        }
        printf("[TLS] Device cert loaded\r\n");

        if (device_cert_def->ca_cert_def != NULL)
        {
            ret = atca_mbedtls_cert_add(&s_clicert, device_cert_def->ca_cert_def);
            if (ret != 0)
                printf("[TLS] signer cert failed : %d\r\n", ret);
            else
                printf("[TLS] Signer cert loaded\r\n");
        }

#ifdef ENABLE_CERT_DUMP
        printf("\r\n[CERT] Chain this board presents to the broker\r\n");
        printf("[CERT] Register the 'device' certificate below, and check it\r\n");
        printf("[CERT] matches the one already registered in your account.\r\n\r\n");
        dump_cert("device", &s_clicert);
        dump_cert("signer", s_clicert.next);
#endif

        /* Binds the key handle, not the key: signing happens inside the chip. */
        ret = atca_mbedtls_pk_init(&s_pkey, 0);
        if (ret != 0)
        {
            printf("[TLS] pk_init failed : %d\r\n", ret);
            return -1;
        }
        printf("[TLS] Private key -> ATECC608C slot 0\r\n");
    }
#else
    printf("[TLS] mTLS disabled -- broker authentication only\r\n");
#endif

    /* ---- 4. SSL configuration ---- */
    mbedtls_ssl_init(&s_ssl);
    mbedtls_ssl_config_init(&s_conf);

    ret = mbedtls_ssl_config_defaults(&s_conf,
                                      MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0)
    {
        printf("[TLS] ssl_config_defaults failed : -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }

    /* The broker must prove itself. Unlike the echo examples this one talks
     * to a public endpoint, so VERIFY_NONE would leave it open to anyone who
     * can answer on that address. */
    mbedtls_ssl_conf_authmode(&s_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&s_conf, &s_cacert, NULL);

#ifdef ENABLE_MTLS
    ret = mbedtls_ssl_conf_own_cert(&s_conf, &s_clicert, &s_pkey);
    if (ret != 0)
    {
        printf("[TLS] conf_own_cert failed : -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }
#endif

    mbedtls_ssl_conf_rng(&s_conf, mbedtls_ctr_drbg_random, &s_ctr_drbg);
    mbedtls_ssl_conf_dbg(&s_conf, tls_debug_cb, NULL);
    mbedtls_debug_set_threshold(TLS_DEBUG_LEVEL);

    ret = mbedtls_ssl_setup(&s_ssl, &s_conf);
    if (ret != 0)
    {
        printf("[TLS] ssl_setup failed : -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }

    mbedtls_ssl_set_bio(&s_ssl, wizchip_tls_get_ctx(),
                        wizchip_tls_send, wizchip_tls_recv, NULL);

    s_initialised = 1;
    printf("[TLS] Init complete\r\n");

    return 0;
}

/* ============================================================ */
/* Connect                                                       */
/* ============================================================ */
int mqtts_transport_connect(Network *n, uint8_t *server_ip, uint16_t port,
                            const char *hostname)
{
    int ret;
    int8_t sock_ret;
    uint8_t io_mode = SOCK_IO_NONBLOCK;
    uint32_t start_ms;
    uint32_t flags;
    char errbuf[128];

    if (!s_initialised)
        return -1;

    /* Reset the session but keep the configuration, certificates and the
     * ATECC608C key handle -- reconnecting must not re-read the chip. */
    ret = mbedtls_ssl_session_reset(&s_ssl);
    if (ret != 0)
    {
        printf("[TLS] session_reset failed : -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }

    /* AWS IoT Core routes on SNI and the certificate is issued for the
     * endpoint name, so the hostname is required, not decorative. */
    if (hostname == NULL)
    {
        printf("[TLS] no hostname -- AWS IoT requires SNI\r\n");
        return -1;
    }

    ret = mbedtls_ssl_set_hostname(&s_ssl, hostname);
    if (ret != 0)
    {
        printf("[TLS] set_hostname failed : -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }

    printf("[TLS] Connecting to %d.%d.%d.%d:%d\r\n",
           server_ip[0], server_ip[1], server_ip[2], server_ip[3], port);

    /* Open and connect while still blocking, so connect() completes here
     * instead of having to be polled. */
    sock_ret = socket(TLS_SOCKET_NUM, Sn_MR_TCP4, 0, 0);
    if (sock_ret != TLS_SOCKET_NUM)
    {
        printf("[TLS] socket() failed : %d\r\n", sock_ret);
        return -1;
    }
    s_socket_open = 1;

    sock_ret = connect(TLS_SOCKET_NUM, server_ip, port);
    if (sock_ret != SOCK_OK)
    {
        printf("[TLS] TCP connect failed : %d\r\n", sock_ret);
        mqtts_transport_close(n);
        return -1;
    }
    printf("[TLS] TCP connected\r\n");

    /* From here on recv() must return instead of spinning on an empty
     * buffer, or MQTTYield() would never come back. */
    if (ctlsocket(TLS_SOCKET_NUM, CS_SET_IOMODE, &io_mode) != SOCK_OK)
    {
        printf("[TLS] could not switch socket to non-blocking\r\n");
        mqtts_transport_close(n);
        return -1;
    }

    printf("[TLS] Handshake...\r\n");
    start_ms = HAL_GetTick();

    while ((ret = mbedtls_ssl_handshake(&s_ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            mbedtls_strerror(ret, errbuf, sizeof(errbuf));
            printf("[TLS] Handshake failed : -0x%04X (%s)\r\n",
                   (unsigned)-ret, errbuf);
            mqtts_transport_close(n);
            return ret;
        }

        if ((HAL_GetTick() - start_ms) >= TLS_HANDSHAKE_TIMEOUT)
        {
            printf("[TLS] Handshake timeout\r\n");
            mqtts_transport_close(n);
            return -1;
        }
    }

    flags = mbedtls_ssl_get_verify_result(&s_ssl);
    if (flags != 0)
    {
        /* VERIFY_REQUIRED means the handshake would already have failed, so
         * this is belt and braces -- but it names the reason. */
        mbedtls_x509_crt_verify_info(errbuf, sizeof(errbuf), "  ", flags);
        printf("[TLS] Certificate rejected :\r\n%s", errbuf);
        mqtts_transport_close(n);
        return -1;
    }

    printf("[TLS] Handshake OK (%s, %s)\r\n",
           mbedtls_ssl_get_version(&s_ssl),
           mbedtls_ssl_get_ciphersuite(&s_ssl));

    n->my_socket = TLS_SOCKET_NUM;
    n->mqttread = mqtts_read;
    n->mqttwrite = mqtts_write;
    n->disconnect = mqtts_net_disconnect;

    return 0;
}

/* ============================================================ */
/* Close                                                         */
/* ============================================================ */
void mqtts_transport_close(Network *n)
{
    (void)n;

    if (!s_socket_open)
        return;

    if (s_initialised)
        mbedtls_ssl_close_notify(&s_ssl);

    disconnect(TLS_SOCKET_NUM);
    close(TLS_SOCKET_NUM);

    s_socket_open = 0;
}

#endif /* EXAMPLE_MQTTS */
