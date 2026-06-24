#ifndef __WIZCHIP_TLS_H__
#define __WIZCHIP_TLS_H__

#include <stddef.h>

#define TLS_SOCKET_NUM  1

int wizchip_tls_send(void *ctx, const unsigned char *buf, size_t len);
int wizchip_tls_recv(void *ctx, unsigned char *buf, size_t len);
void *wizchip_tls_get_ctx(void);

#endif /* __WIZCHIP_TLS_H__ */
