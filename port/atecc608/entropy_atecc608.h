#ifndef __ENTROPY_ATECC608_H__
#define __ENTROPY_ATECC608_H__

#include <stddef.h>

int entropy_atecc608_source(void *data, unsigned char *output,
                            size_t len, size_t *olen);

#endif /* __ENTROPY_ATECC608_H__ */
