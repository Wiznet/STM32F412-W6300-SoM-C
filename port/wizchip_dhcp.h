/**
 * wizchip_dhcp.h
 *
 * DHCP wrapper for W6300 ioLibrary_Driver
 * Style follows WIZnet official reference (RP2040 examples)
 */

#ifndef __WIZCHIP_DHCP_H__
#define __WIZCHIP_DHCP_H__

#include "main.h"
#include "wizchip_conf.h"
#include "dhcp.h"

/* Socket number used by DHCP */
#define SOCKET_DHCP         0

/* DHCP retry limit */
#define DHCP_RETRY_COUNT    5

/**
 * @brief  Initialize DHCP client
 *         Registers DHCP callbacks (assign / update / conflict).
 *         Network info is auto-applied when an IP is leased.
 * @param  net_info  Pointer to the network info struct to be updated.
 * @param  buf       DHCP message buffer (at least 548 bytes; 1024 recommended).
 */
void wizchip_dhcp_init(wiz_NetInfo *net_info, uint8_t *buf);

/**
 * @brief  Run DHCP state machine (call periodically in main loop).
 *         Internally handles retry logic and stops DHCP after too many failures.
 * @retval DHCP_run() return value (DHCP_IP_LEASED, DHCP_FAILED, ...)
 */
uint8_t wizchip_dhcp_run(void);

/**
 * @brief  DHCP 1-second tick handler (call from SysTick at 1 Hz).
 */
void wizchip_dhcp_time_tick(void);

#endif /* __WIZCHIP_DHCP_H__ */
