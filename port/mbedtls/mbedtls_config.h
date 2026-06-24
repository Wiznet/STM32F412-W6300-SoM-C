/**
 * mbedtls_config.h
 *
 * mbedTLS 3.6 LTS configuration for STM32F412 + ATECC608C-TNGTLS + W6300
 * Based on config-suite-b.h: ECDHE-ECDSA TLS, P-256.
 *
 * This file shadows Libraries/mbedtls/include/mbedtls/mbedtls_config.h
 * via include path priority (port/mbedtls before Libraries/mbedtls/include).
 */

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

/* === System support === */
#define MBEDTLS_HAVE_ASM

/* === Platform === */
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_NO_PLATFORM_ENTROPY

/* === Feature selection === */
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_SSL_PROTO_TLS1_2

/* === Crypto modules === */
#define MBEDTLS_AES_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECDSA_DETERMINISTIC
#define MBEDTLS_ECP_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_GCM_C
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_MD_C
#define MBEDTLS_OID_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_RSA_C
#define MBEDTLS_SHA256_C

/* === TLS modules === */
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_TLS_C

/* === X.509 === */
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_USE_C

/* === PEM cert parsing === */
#define MBEDTLS_BASE64_C
#define MBEDTLS_PEM_PARSE_C

/* === Debug & Error (disable for production) === */
#define MBEDTLS_DEBUG_C
#define MBEDTLS_ERROR_C
#define MBEDTLS_ERROR_STRERROR_DUMMY

/* === TLS features === */
#define MBEDTLS_SSL_ALL_ALERT_MESSAGES
#define MBEDTLS_SSL_ENCRYPT_THEN_MAC
#define MBEDTLS_SSL_EXTENDED_MASTER_SECRET
#define MBEDTLS_SSL_SERVER_NAME_INDICATION
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE

/* Single ciphersuite to save ROM. */
#define MBEDTLS_SSL_CIPHERSUITES \
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256

/* Buffer sizes: defaults are 16KB each, which is too large for embedded. */
#define MBEDTLS_SSL_IN_CONTENT_LEN     4096
#define MBEDTLS_SSL_OUT_CONTENT_LEN    2048

/* === Memory optimizations === */
#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_MPI_MAX_SIZE    32
#define MBEDTLS_ECP_WINDOW_SIZE        2
#define MBEDTLS_ECP_FIXED_POINT_OPTIM  0
#define MBEDTLS_ECP_NIST_OPTIM

/* Entropy: only ATECC608 hardware source */
#define MBEDTLS_ENTROPY_MAX_SOURCES 2

#endif /* MBEDTLS_CONFIG_H */
