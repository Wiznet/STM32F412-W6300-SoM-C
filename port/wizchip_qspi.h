/**
 * wizchip_qspi.h
 *
 * STM32F412 + W6300 QSPI port for ioLibrary_Driver
 */

#ifndef __WIZCHIP_QSPI_H__
#define __WIZCHIP_QSPI_H__

#include "main.h"
#include "wizchip_conf.h"

/* External QSPI handle defined in main.c */
extern QSPI_HandleTypeDef hqspi;

/* QSPI mode is defined in wizchip_conf.h:
 *   QSPI_SINGLE_MODE, QSPI_DUAL_MODE, QSPI_QUAD_MODE
 * Select active mode here:
 */
#ifndef _WIZCHIP_QSPI_MODE_
#define _WIZCHIP_QSPI_MODE_  QSPI_QUAD_MODE
#endif

/**
 * @brief  W6300 QSPI write
 * @param  opcode  SPI opcode
 * @param  addr    16-bit register address
 * @param  pbuf    Data buffer to write
 * @param  len     Number of bytes
 */
void W6300_QspiWriteByte(uint8_t opcode, uint16_t addr, uint8_t *pbuf, uint16_t len);

/**
 * @brief  W6300 QSPI read
 * @param  opcode  SPI opcode
 * @param  addr    16-bit register address
 * @param  pbuf    Data buffer to read into
 * @param  len     Number of bytes
 */
void W6300_QspiReadByte(uint8_t opcode, uint16_t addr, uint8_t *pbuf, uint16_t len);

/**
 * @brief  Number of QSPI transfers that failed since boot
 *
 * ioLibrary fixes the QSPI callback signature (see reg_wizchip_qspi_cbfunc),
 * so the two functions above cannot report a failure to their caller. A failed
 * read leaves its buffer zeroed and bumps this counter. Expected to stay 0;
 * anything else means the QSPI link is marginal.
 *
 * @return Failed transfer count
 */
uint32_t W6300_QspiGetErrorCount(void);

/**
 * @brief  Hardware reset W6300 via RSTn pin
 */
void wizchip_reset(void);

/**
 * @brief  Initialize WIZchip (register QSPI callbacks, set buffer sizes, wait PHY link)
 */
void wizchip_initialize(void);

/**
 * @brief  Apply network configuration to WIZchip
 * @param  net_info  Network info struct (IP, MAC, GW, etc.)
 */
void network_initialize(wiz_NetInfo net_info);

/**
 * @brief  Print current network configuration via UART
 * @param  net_info  Network info struct
 */
void print_network_information(wiz_NetInfo net_info);

/**
 * @brief  Print IPv6 address with label
 * @param  name      Label string
 * @param  ip6addr   16-byte IPv6 address
 */
void print_ipv6_addr(uint8_t *name, uint8_t *ip6addr);

#endif /* __WIZCHIP_QSPI_H__ */
