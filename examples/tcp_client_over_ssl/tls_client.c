/**
 * tls_client.c
 *
 * TLS 1.2 client using mbedTLS 3.6 LTS + ATECC608C-TNGTLS + W6300
 */

#include <stdio.h>
#include <string.h>
#include "tls_client.h"
#include "wizchip_tls.h"
#include "entropy_atecc608.h"
#include "socket.h"

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/debug.h"
#include "mbedtls/error.h"

#include "cryptoauthlib.h"
#include "tng_atca.h"
#include "atca_mbedtls_wrap.h"

static mbedtls_ssl_context      s_ssl;
static mbedtls_ssl_config       s_conf;
static mbedtls_x509_crt         s_cacert;
static mbedtls_x509_crt         s_clicert;
static mbedtls_pk_context       s_pkey;
static mbedtls_entropy_context  s_entropy;
static mbedtls_ctr_drbg_context s_ctr_drbg;

static void tls_debug_cb(void *ctx, int level,
                          const char *file, int line, const char *str)
{
    (void)ctx;
    const char *p = file;
    const char *slash = strrchr(file, '/');
    if (slash) p = slash + 1;
    const char *bslash = strrchr(p, '\\');
    if (bslash) p = bslash + 1;

    printf("[mbedTLS:%d] %s:%d: %s", level, p, line, str);
}

int tls_client_init(void)
{
    int ret;

    /* 1. Entropy + DRBG */
    mbedtls_entropy_init(&s_entropy);
    mbedtls_ctr_drbg_init(&s_ctr_drbg);

    ret = mbedtls_entropy_add_source(&s_entropy, entropy_atecc608_source,
                                     NULL, 32, MBEDTLS_ENTROPY_SOURCE_STRONG);
    if (ret != 0) {
        printf("[TLS] entropy_add_source failed: -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }

    ret = mbedtls_ctr_drbg_seed(&s_ctr_drbg, mbedtls_entropy_func,
                                &s_entropy,
                                (const unsigned char *)"atecc608_tls", 12);
    if (ret != 0) {
        printf("[TLS] ctr_drbg_seed failed: -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }
    printf("[TLS] RNG seeded OK\r\n");

    /* 2. CA certificate -- skip for now (VERIFY_NONE) */
    mbedtls_x509_crt_init(&s_cacert);

    /* 3. Client certificate from ATECC608 (mTLS) */
    mbedtls_x509_crt_init(&s_clicert);
    mbedtls_pk_init(&s_pkey);

#if 0 /* Enable for mTLS using TNGTLS device certificate */
    {
        const atcacert_def_t *device_cert_def = NULL;
        ret = tng_get_device_cert_def(&device_cert_def);
        if (ret == ATCA_SUCCESS) {
            if (device_cert_def->ca_cert_def != NULL) {
                ret = atca_mbedtls_cert_add(&s_clicert, device_cert_def->ca_cert_def);
                if (ret != 0)
                    printf("[TLS] signer cert: %d (skipped)\r\n", ret);
                else
                    printf("[TLS] Signer cert loaded\r\n");
            }
            ret = atca_mbedtls_cert_add(&s_clicert, device_cert_def);
            if (ret != 0)
                printf("[TLS] device cert: %d (skipped)\r\n", ret);
            else
                printf("[TLS] Device cert loaded\r\n");

            ret = atca_mbedtls_pk_init(&s_pkey, 0);
            if (ret != 0)
                printf("[TLS] pk_init: %d (skipped)\r\n", ret);
            else
                printf("[TLS] PK -> ATECC608 slot 0\r\n");
        }
    }
#endif
    printf("[TLS] mTLS: disabled\r\n");

    /* 4. SSL config */
    mbedtls_ssl_init(&s_ssl);
    mbedtls_ssl_config_init(&s_conf);

    ret = mbedtls_ssl_config_defaults(&s_conf,
                                       MBEDTLS_SSL_IS_CLIENT,
                                       MBEDTLS_SSL_TRANSPORT_STREAM,
                                       MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        printf("[TLS] ssl_config_defaults failed: -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }

    mbedtls_ssl_conf_authmode(&s_conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_ca_chain(&s_conf, &s_cacert, NULL);
    mbedtls_ssl_conf_rng(&s_conf, mbedtls_ctr_drbg_random, &s_ctr_drbg);

    mbedtls_ssl_conf_dbg(&s_conf, tls_debug_cb, NULL);
    mbedtls_debug_set_threshold(1);

    ret = mbedtls_ssl_setup(&s_ssl, &s_conf);
    if (ret != 0) {
        printf("[TLS] ssl_setup failed: -0x%04X\r\n", (unsigned)-ret);
        return ret;
    }

    mbedtls_ssl_set_bio(&s_ssl, wizchip_tls_get_ctx(),
                        wizchip_tls_send, wizchip_tls_recv, NULL);

    printf("[TLS] Init complete\r\n");
    return 0;
}

int tls_client_connect(uint8_t *server_ip, uint16_t port, const char *hostname)
{
    int ret;
    int8_t sock_ret;
    char errbuf[128];

    printf("[TLS] Connecting to %d.%d.%d.%d:%d ...\r\n",
           server_ip[0], server_ip[1], server_ip[2], server_ip[3], port);

    sock_ret = socket(TLS_SOCKET_NUM, Sn_MR_TCP4, 0, 0);
    if (sock_ret != TLS_SOCKET_NUM) {
        printf("[TLS] socket() failed: %d\r\n", sock_ret);
        return -1;
    }

    sock_ret = connect(TLS_SOCKET_NUM, server_ip, port);
    if (sock_ret != SOCK_OK) {
        printf("[TLS] TCP connect failed: %d\r\n", sock_ret);
        close(TLS_SOCKET_NUM);
        return -1;
    }
    printf("[TLS] TCP connected\r\n");

    if (hostname != NULL) {
        ret = mbedtls_ssl_set_hostname(&s_ssl, hostname);
        if (ret != 0) {
            printf("[TLS] set_hostname failed: -0x%04X\r\n", (unsigned)-ret);
        }
    }

    printf("[TLS] Starting TLS handshake...\r\n");
    while ((ret = mbedtls_ssl_handshake(&s_ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            mbedtls_strerror(ret, errbuf, sizeof(errbuf));
            printf("[TLS] Handshake failed: -0x%04X (%s)\r\n",
                   (unsigned)-ret, errbuf);
            close(TLS_SOCKET_NUM);
            return ret;
        }
    }

    printf("[TLS] Handshake OK!\r\n");
    printf("[TLS] Ciphersuite: %s\r\n", mbedtls_ssl_get_ciphersuite(&s_ssl));
    printf("[TLS] TLS version: %s\r\n", mbedtls_ssl_get_version(&s_ssl));

    return 0;
}

int tls_client_write(const unsigned char *buf, size_t len)
{
    int ret;
    while ((ret = mbedtls_ssl_write(&s_ssl, buf, len)) <= 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            return ret;
    }
    return ret;
}

int tls_client_read(unsigned char *buf, size_t len)
{
    int ret;
    while ((ret = mbedtls_ssl_read(&s_ssl, buf, len)) <= 0) {
        if (ret == 0)
            return 0;
        if (ret != MBEDTLS_ERR_SSL_WANT_READ)
            return ret;
    }
    return ret;
}

void tls_client_close(void)
{
    mbedtls_ssl_close_notify(&s_ssl);
    disconnect(TLS_SOCKET_NUM);
    close(TLS_SOCKET_NUM);

    mbedtls_ssl_free(&s_ssl);
    mbedtls_ssl_config_free(&s_conf);
    mbedtls_x509_crt_free(&s_clicert);
    mbedtls_x509_crt_free(&s_cacert);
    mbedtls_pk_free(&s_pkey);
    mbedtls_ctr_drbg_free(&s_ctr_drbg);
    mbedtls_entropy_free(&s_entropy);

    printf("[TLS] Connection closed\r\n");
}
