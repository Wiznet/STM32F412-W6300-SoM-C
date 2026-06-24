#include <string.h>
#include "entropy_atecc608.h"
#include "mbedtls/entropy.h"
#include "cryptoauthlib.h"

int entropy_atecc608_source(void *data, unsigned char *output,
                            size_t len, size_t *olen)
{
    (void)data;
    uint8_t rand_buf[32];

    *olen = 0;
    while (*olen < len) {
        ATCA_STATUS status = atcab_random(rand_buf);
        if (status != ATCA_SUCCESS) {
            return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
        }
        size_t copy = (len - *olen < 32) ? (len - *olen) : 32;
        memcpy(output + *olen, rand_buf, copy);
        *olen += copy;
    }
    return 0;
}
