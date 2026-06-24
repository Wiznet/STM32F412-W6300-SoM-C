/**
 * atecc608.c
 *
 * ATECC608C-TNGTLS device driver
 */

#include <stdio.h>
#include <string.h>
#include "atecc608.h"
#include "tng_atca.h"
#include "atcacert/atcacert_client.h"
#include "atcacert/atcacert_host_hw.h"
#include "tng_root_cert.h"

static ATCAIfaceCfg g_cfg = {
    .iface_type         = ATCA_I2C_IFACE,
    .devtype            = ATECC608B,
    .atcai2c.address    = ATECC608_I2C_ADDR_7BIT,
    .atcai2c.bus        = 0,
    .atcai2c.baud       = 100000,
    .wake_delay         = 1500,
    .rx_retries         = 20,
};

ATCA_STATUS atecc608_init(void)
{
    ATCA_STATUS status;

    status = atcab_init(&g_cfg);
    if (status != ATCA_SUCCESS)
    {
        printf("[ATECC608] Init failed: 0x%02X\r\n", status);
        return status;
    }

    printf("[ATECC608] Init success\r\n");
    return ATCA_SUCCESS;
}

ATCA_STATUS atecc608_print_serial_number(void)
{
    ATCA_STATUS status;
    uint8_t serial_number[9];

    status = atcab_read_serial_number(serial_number);
    if (status != ATCA_SUCCESS)
    {
        printf("[ATECC608] Read serial number failed: 0x%02X\r\n", status);
        return status;
    }

    printf("[ATECC608] Serial Number: ");
    for (int i = 0; i < 9; i++)
    {
        printf("%02X ", serial_number[i]);
    }
    printf("\r\n");

    return ATCA_SUCCESS;
}

static uint8_t s_signer_cert[700];
static uint8_t s_device_cert[700];

ATCA_STATUS atecc608_read_certs(void)
{
    ATCA_STATUS status;
    int ret;
    const atcacert_def_t *dev_cert_def = NULL;
    const atcacert_def_t *signer_def = NULL;
    uint8_t pubkey[ATCA_ECCP256_PUBKEY_SIZE];
    uint8_t signer_pubkey[ATCA_ECCP256_PUBKEY_SIZE];
    uint8_t cert_device_pubkey[ATCA_ECCP256_PUBKEY_SIZE];
    size_t signer_cert_size = sizeof(s_signer_cert);
    size_t device_cert_size = sizeof(s_device_cert);
    cal_buffer root_ca_pubkey = cal_buf_init_const_ptr(
        ATCA_ECCP256_PUBKEY_SIZE,
        &g_cryptoauth_root_ca_002_cert[CRYPTOAUTH_ROOT_CA_002_PUBLIC_KEY_OFFSET]);
    cal_buffer signer_pubkey_buf = CAL_BUF_INIT(sizeof(signer_pubkey), signer_pubkey);
    cal_buffer cert_device_pubkey_buf = CAL_BUF_INIT(sizeof(cert_device_pubkey), cert_device_pubkey);

    printf("[ATECC608] TNGTLS certificate chain test\r\n");

    uint8_t otp_data[32];
    printf("[ATECC608] Step 1: Reading OTP zone...\r\n");
    status = atcab_read_zone(ATCA_ZONE_OTP, 0, 0, 0, otp_data, 32);
    if (status != ATCA_SUCCESS)
    {
        printf("[ATECC608] OTP read failed: 0x%02X\r\n", status);
        return status;
    }
    printf("[ATECC608] OTP code: %.8s\r\n", (char *)otp_data);

    printf("[ATECC608] Step 2: Getting TNGTLS cert definition...\r\n");
    status = tng_get_device_cert_def(&dev_cert_def);
    if (status != ATCA_SUCCESS)
    {
        printf("[ATECC608] cert def failed: 0x%02X\r\n", status);
        return status;
    }
    signer_def = dev_cert_def->ca_cert_def;
    if (signer_def == NULL)
    {
        printf("[ATECC608] signer cert def missing\r\n");
        return ATCA_BAD_PARAM;
    }

    printf("[ATECC608] Step 3: GenKey slot 0 (device pubkey)...\r\n");
    status = atcab_get_pubkey(0, pubkey);
    if (status != ATCA_SUCCESS)
    {
        printf("[ATECC608] GenKey failed: 0x%02X\r\n", status);
        return status;
    }
    printf("[ATECC608] PubKey X: %02X %02X %02X %02X ...\r\n",
           pubkey[0], pubkey[1], pubkey[2], pubkey[3]);

    printf("[ATECC608] Step 4: Reading signer certificate...\r\n");
    ret = atcacert_read_cert(signer_def, &root_ca_pubkey, s_signer_cert, &signer_cert_size);
    if (ret != ATCACERT_E_SUCCESS)
    {
        printf("[ATECC608] Signer cert failed: %d\r\n", ret);
        return (ATCA_STATUS)ret;
    }
    printf("[ATECC608] Signer cert: %u bytes\r\n", (unsigned)signer_cert_size);

    printf("[ATECC608] Step 5: Verify signer cert with root CA...\r\n");
    ret = atcacert_verify_cert_hw(atcab_get_device(), signer_def,
                                  s_signer_cert, signer_cert_size,
                                  &root_ca_pubkey);
    if (ret != ATCACERT_E_SUCCESS)
    {
        printf("[ATECC608] Signer cert verify failed: %d\r\n", ret);
        return (ATCA_STATUS)ret;
    }
    printf("[ATECC608] Signer cert verify: PASS\r\n");

    printf("[ATECC608] Step 6: Extract signer public key...\r\n");
    ret = atcacert_get_subj_public_key(signer_def, s_signer_cert,
                                       signer_cert_size, &signer_pubkey_buf);
    if (ret != ATCACERT_E_SUCCESS)
    {
        printf("[ATECC608] Signer pubkey extract failed: %d\r\n", ret);
        return (ATCA_STATUS)ret;
    }
    printf("[ATECC608] Signer PubKey X: %02X %02X %02X %02X ...\r\n",
           signer_pubkey[0], signer_pubkey[1], signer_pubkey[2], signer_pubkey[3]);

    printf("[ATECC608] Step 7: Reading device certificate...\r\n");
    ret = atcacert_read_cert(dev_cert_def, &signer_pubkey_buf,
                             s_device_cert, &device_cert_size);
    if (ret != ATCACERT_E_SUCCESS)
    {
        printf("[ATECC608] Device cert failed: %d\r\n", ret);
        return (ATCA_STATUS)ret;
    }
    printf("[ATECC608] Device cert: %u bytes\r\n", (unsigned)device_cert_size);

    printf("[ATECC608] Step 8: Verify device cert with signer public key...\r\n");
    ret = atcacert_verify_cert_hw(atcab_get_device(), dev_cert_def,
                                  s_device_cert, device_cert_size,
                                  &signer_pubkey_buf);
    if (ret != ATCACERT_E_SUCCESS)
    {
        printf("[ATECC608] Device cert verify failed: %d\r\n", ret);
        return (ATCA_STATUS)ret;
    }
    printf("[ATECC608] Device cert verify: PASS\r\n");

    printf("[ATECC608] Step 9: Compare cert public key with GenKey slot 0...\r\n");
    ret = atcacert_get_subj_public_key(dev_cert_def, s_device_cert,
                                       device_cert_size, &cert_device_pubkey_buf);
    if (ret != ATCACERT_E_SUCCESS)
    {
        printf("[ATECC608] Device pubkey extract failed: %d\r\n", ret);
        return (ATCA_STATUS)ret;
    }
    if (memcmp(cert_device_pubkey, pubkey, sizeof(pubkey)) != 0)
    {
        printf("[ATECC608] Device public key match: FAIL\r\n");
        printf("[ATECC608] Cert PubKey X: %02X %02X %02X %02X ...\r\n",
               cert_device_pubkey[0], cert_device_pubkey[1],
               cert_device_pubkey[2], cert_device_pubkey[3]);
        return ATCA_GEN_FAIL;
    }
    printf("[ATECC608] Device public key match: PASS\r\n");
    printf("[ATECC608] TNGTLS certificate chain: PASS\r\n");

    return ATCA_SUCCESS;
}

ATCA_STATUS atecc608_test_sign_verify(void)
{
    ATCA_STATUS status;
    bool verified = false;

    const char *msg = "ATECC608 TNGTLS sign test";
    uint8_t digest[32];
    uint8_t signature[64];
    uint8_t public_key[64];

    status = atcab_sha((uint16_t)strlen(msg), (const uint8_t *)msg, digest);
    if (status != ATCA_SUCCESS)
    {
        printf("[ATECC608] SHA failed: 0x%02X\r\n", status);
        return status;
    }
    printf("[ATECC608] Digest: ");
    for (int i = 0; i < 8; i++) printf("%02X ", digest[i]);
    printf("...\r\n");

    status = atcab_sign(0, digest, signature);
    if (status != ATCA_SUCCESS)
    {
        printf("[ATECC608] Sign failed: 0x%02X\r\n", status);
        return status;
    }
    printf("[ATECC608] Signature (r): ");
    for (int i = 0; i < 8; i++) printf("%02X ", signature[i]);
    printf("...\r\n");

    status = atcab_get_pubkey(0, public_key);
    if (status != ATCA_SUCCESS)
    {
        printf("[ATECC608] Get pubkey failed: 0x%02X\r\n", status);
        return status;
    }
    printf("[ATECC608] Public key X: ");
    for (int i = 0; i < 8; i++) printf("%02X ", public_key[i]);
    printf("...\r\n");

    status = atcab_verify_extern(digest, signature, public_key, &verified);
    if (status != ATCA_SUCCESS)
    {
        printf("[ATECC608] Verify failed: 0x%02X\r\n", status);
        return status;
    }

    if (verified)
        printf("[ATECC608] ECDSA verify: PASS\r\n");
    else
        printf("[ATECC608] ECDSA verify: FAIL (signature mismatch)\r\n");

    return verified ? ATCA_SUCCESS : ATCA_CHECKMAC_VERIFY_FAILED;
}
