/**
 * atca_config.h
 *
 * CryptoAuthLib configuration for STM32F412 + ATECC608C-TNGTLS over I2C
 */

#ifndef ATCA_CONFIG_H
#define ATCA_CONFIG_H

#include <stdlib.h>

/* === Device support === */
#define ATCA_ATECC608_SUPPORT

/* === TNG (Trust&GO) support === */
#define ATCA_TNGTLS_SUPPORT

/* === HAL selection === */
#define ATCA_HAL_I2C

/* === Memory === */
#define ATCA_HEAP
#define ATCA_PLATFORM_MALLOC malloc
#define ATCA_PLATFORM_FREE   free

/* === Host crypto library === */
#define ATCA_MBEDTLS

/* === Required delay constants === */
#ifndef ATCA_POST_DELAY_MSEC
#define ATCA_POST_DELAY_MSEC    25
#endif

#ifndef ATCA_POLLING_INIT_TIME_MSEC
#define ATCA_POLLING_INIT_TIME_MSEC    1
#endif

#ifndef ATCA_POLLING_FREQUENCY_TIME_MSEC
#define ATCA_POLLING_FREQUENCY_TIME_MSEC    2
#endif

#ifndef ATCA_POLLING_MAX_TIME_MSEC
#define ATCA_POLLING_MAX_TIME_MSEC    2500
#endif

/* === Printf for debug === */
#define ATCA_PRINTF

#endif /* ATCA_CONFIG_H */
