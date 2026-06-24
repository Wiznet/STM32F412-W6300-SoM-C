/**
 * hal_stm32_i2c.c
 *
 * CryptoAuthLib HAL implementation for STM32 + I2C
 *
 * Requires I2C2 to be initialized by CubeMX (MX_I2C2_Init).
 * Enable I2C2 in the .ioc file before building this example.
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "atca_hal.h"
#include "atca_iface.h"

#define ATECC608_I2C_ADDR_7BIT 0x35
#define ATECC608_I2C_TIMEOUT  100
#define ATECC608_I2C_MAX_TX_SIZE 160U

extern I2C_HandleTypeDef hi2c2;

static ATCAHAL_t g_stm32_i2c_hal = {
    hal_i2c_init,
    hal_i2c_post_init,
    hal_i2c_send,
    hal_i2c_receive,
    hal_i2c_control,
    hal_i2c_release
};

static uint16_t get_stm32_hal_i2c_addr(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    uint8_t addr = ATECC608_I2C_ADDR_7BIT;

    if (cfg != NULL)
    {
        addr = ATCA_IFACECFG_I2C_ADDRESS(cfg);
    }

    return (uint16_t)addr << 1;
}

void hal_delay_ms(uint32_t delay)
{
    HAL_Delay(delay);
}

void hal_delay_us(uint32_t delay)
{
    uint32_t cycles = delay * (SystemCoreClock / 1000000U);
    while (cycles--)
    {
        __NOP();
    }
}

ATCA_STATUS hal_i2c_init(ATCAIface iface, ATCAIfaceCfg *cfg)
{
    (void)iface;
    (void)cfg;
    printf("[ATECC608] I2C HAL initialized\r\n");
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    (void)iface;
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t word_address, uint8_t *txdata, int txlength)
{
    HAL_StatusTypeDef status;
    uint8_t buf[ATECC608_I2C_MAX_TX_SIZE + 1U];
    uint8_t *payload = txdata;
    uint16_t payload_len = (uint16_t)txlength;

    if ((txlength < 0) || ((uint32_t)txlength > ATECC608_I2C_MAX_TX_SIZE))
    {
        return ATCA_BAD_PARAM;
    }

    if ((txlength > 0) && (txdata == NULL))
    {
        return ATCA_BAD_PARAM;
    }

    if (word_address != 0xFFU)
    {
        buf[0] = word_address;

        if (txlength > 0)
        {
            memcpy(&buf[1], txdata, (size_t)txlength);
        }

        payload = buf;
        payload_len = (uint16_t)txlength + 1U;
    }

    status = HAL_I2C_Master_Transmit(&hi2c2,
                                     get_stm32_hal_i2c_addr(iface),
                                     payload,
                                     payload_len,
                                     ATECC608_I2C_TIMEOUT);

    if (status != HAL_OK)
    {
        if (hi2c2.ErrorCode != HAL_I2C_ERROR_NONE)
        {
            __HAL_I2C_CLEAR_FLAG(&hi2c2, I2C_FLAG_AF);
            hi2c2.ErrorCode = HAL_I2C_ERROR_NONE;
            hi2c2.State = HAL_I2C_STATE_READY;
        }
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t word_address, uint8_t *rxdata, uint16_t *rxlength)
{
    (void)word_address;

    HAL_StatusTypeDef status;
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int retries = (cfg != NULL) ? cfg->rx_retries : 1;

    if ((rxdata == NULL) || (rxlength == NULL) || (*rxlength == 0U))
    {
        return ATCA_BAD_PARAM;
    }

    do
    {
        status = HAL_I2C_Master_Receive(&hi2c2,
                                        get_stm32_hal_i2c_addr(iface),
                                        rxdata,
                                        *rxlength,
                                        ATECC608_I2C_TIMEOUT);

        if (status == HAL_OK)
        {
            return ATCA_SUCCESS;
        }

        if (hi2c2.ErrorCode != HAL_I2C_ERROR_NONE)
        {
            __HAL_I2C_CLEAR_FLAG(&hi2c2, I2C_FLAG_AF);
            hi2c2.ErrorCode = HAL_I2C_ERROR_NONE;
            hi2c2.State = HAL_I2C_STATE_READY;
        }

        HAL_Delay(1);
    } while (retries-- > 0);

    printf("[ATECC608] I2C receive error: %d\r\n", status);

    return ATCA_COMM_FAIL;
}

ATCA_STATUS hal_i2c_control(ATCAIface iface, uint8_t option, void *param, size_t paramlen)
{
    (void)param;
    (void)paramlen;

    HAL_StatusTypeDef status;
    uint8_t word_address;
    uint8_t wake_response[4];
    uint16_t wake_response_len = sizeof(wake_response);
    uint8_t wake_token = 0x00;

    switch (option)
    {
    case ATCA_HAL_CONTROL_WAKE:
        (void)HAL_I2C_Master_Transmit(&hi2c2, 0x00, &wake_token, 1, 1);
        if (hi2c2.ErrorCode != HAL_I2C_ERROR_NONE)
        {
            __HAL_I2C_CLEAR_FLAG(&hi2c2, I2C_FLAG_AF);
            hi2c2.ErrorCode = HAL_I2C_ERROR_NONE;
            hi2c2.State = HAL_I2C_STATE_READY;
        }
        HAL_Delay(2);
        status = HAL_I2C_Master_Receive(&hi2c2,
                                        get_stm32_hal_i2c_addr(iface),
                                        wake_response,
                                        wake_response_len,
                                        ATECC608_I2C_TIMEOUT);
        if (status != HAL_OK)
        {
            return ATCA_WAKE_FAILED;
        }
        return hal_check_wake(wake_response, wake_response_len);

    case ATCA_HAL_CONTROL_IDLE:
        word_address = 0x02;
        break;

    case ATCA_HAL_CONTROL_SLEEP:
        word_address = 0x01;
        break;

    default:
        return ATCA_UNIMPLEMENTED;
    }

    status = HAL_I2C_Master_Transmit(&hi2c2,
                                     get_stm32_hal_i2c_addr(iface),
                                     &word_address,
                                     1,
                                     ATECC608_I2C_TIMEOUT);

    return (status == HAL_OK) ? ATCA_SUCCESS : ATCA_COMM_FAIL;
}

ATCA_STATUS hal_i2c_release(void *hal_data)
{
    (void)hal_data;
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_iface_init(ATCAIfaceCfg *cfg, ATCAHAL_t **hal, ATCAHAL_t** phy)
{
    if ((cfg == NULL) || (hal == NULL))
    {
        return ATCA_BAD_PARAM;
    }

    if (cfg->iface_type != ATCA_I2C_IFACE)
    {
        return ATCA_BAD_PARAM;
    }

    *hal = &g_stm32_i2c_hal;

    if (phy != NULL)
    {
        *phy = NULL;
    }

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_iface_release(ATCAIfaceType iface_type, void *hal_data)
{
    (void)iface_type;
    (void)hal_data;
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_check_wake(const uint8_t *response, int response_size)
{
    const uint8_t expected[4] = {0x04, 0x11, 0x33, 0x43};

    if (response_size < 4)
        return ATCA_COMM_FAIL;

    if (memcmp(response, expected, 4) == 0)
        return ATCA_SUCCESS;

    return ATCA_WAKE_FAILED;
}

void atca_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void atca_delay_us(uint32_t us)
{
    uint32_t cycles = us * (SystemCoreClock / 1000000U);
    while (cycles--)
    {
        __NOP();
    }
}
