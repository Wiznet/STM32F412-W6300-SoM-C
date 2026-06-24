#ifndef __ATECC608_H__
#define __ATECC608_H__

#include "atca_basic.h"

#define ATECC608_I2C_ADDR_7BIT    0x35

ATCA_STATUS atecc608_init(void);
ATCA_STATUS atecc608_print_serial_number(void);
ATCA_STATUS atecc608_read_certs(void);
ATCA_STATUS atecc608_test_sign_verify(void);

#endif /* __ATECC608_H__ */
