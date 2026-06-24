#ifndef __TLS_CLIENT_H__
#define __TLS_CLIENT_H__

#include <stdint.h>

int tls_client_init(void);
int tls_client_connect(uint8_t *server_ip, uint16_t port, const char *hostname);
int tls_client_write(const unsigned char *buf, size_t len);
int tls_client_read(unsigned char *buf, size_t len);
void tls_client_close(void);

#endif /* __TLS_CLIENT_H__ */
