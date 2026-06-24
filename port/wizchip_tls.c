#include "wizchip_tls.h"
#include "socket.h"
#include "mbedtls/ssl.h"

#define WIZCHIP_ERR_NET_SEND_FAILED  (-0x004E)
#define WIZCHIP_ERR_NET_RECV_FAILED  (-0x004C)

typedef struct {
    uint8_t sn;
} wizchip_tls_ctx_t;

static wizchip_tls_ctx_t g_tls_ctx = { .sn = TLS_SOCKET_NUM };

int wizchip_tls_send(void *ctx, const unsigned char *buf, size_t len)
{
    wizchip_tls_ctx_t *c = (wizchip_tls_ctx_t *)ctx;

    if (len > 0xFFFF)
        len = 0xFFFF;

    int32_t ret = send(c->sn, (uint8_t *)buf, (uint16_t)len);

    if (ret == SOCK_BUSY)
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    if (ret < 0)
        return WIZCHIP_ERR_NET_SEND_FAILED;
    return (int)ret;
}

int wizchip_tls_recv(void *ctx, unsigned char *buf, size_t len)
{
    wizchip_tls_ctx_t *c = (wizchip_tls_ctx_t *)ctx;

    if (len > 0xFFFF)
        len = 0xFFFF;

    int32_t ret = recv(c->sn, buf, (uint16_t)len);

    if (ret == SOCK_BUSY)
        return MBEDTLS_ERR_SSL_WANT_READ;
    if (ret < 0)
        return WIZCHIP_ERR_NET_RECV_FAILED;
    return (int)ret;
}

void *wizchip_tls_get_ctx(void)
{
    return &g_tls_ctx;
}
