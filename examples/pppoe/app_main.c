/**
 * @file    app_main.c
 * @brief   PPPoE example for STM32F412 + W6300 SoM
 *
 * @details Establishes a PPPoE connection and obtains an IP address
 *          from the PPPoE server (NAS). After connection, prints
 *          the assigned network configuration.
 *          Based on WIZnet-PICO-C pppoe example.
 *
 * @note    PPPoE.c, PPPoE.h, md5.c, md5.h must be copied from the
 *          PICO-C example into this directory.
 *          Remove #include "pico/time.h" from PPPoE.c.
 *
 * @note    No SysTick timer tick needed for this example.
 */

#include "main.h"

#ifdef EXAMPLE_PPPOE

#include <stdio.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "PPPoE.h"

/* ============================================================ */
/* Configuration                                                 */
/* ============================================================ */
#define DATA_BUF_SIZE          2048

/* ============================================================ */
/* Network information (used for initial chip config)            */
/* ============================================================ */
static wiz_NetInfo g_net_info = {
    .mac  = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
    .ip   = {192, 168, 11, 10},
    .sn   = {255, 255, 255, 0},
    .gw   = {192, 168, 11, 1},
    .dns  = {8, 8, 8, 8},
#if _WIZCHIP_ > W5500
    .lla  = {0xfe,0x80, 0x00,0x00, 0x00,0x00, 0x00,0x00,
             0x02,0x08, 0xdc,0xff, 0xfe,0x57, 0x57,0x25},
    .gua  = {0},
    .sn6  = {0xff,0xff, 0xff,0xff, 0xff,0xff, 0xff,0xff,
             0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00},
    .gw6  = {0},
    .dns6 = {0x20,0x01, 0x48,0x60, 0x48,0x60, 0x00,0x00,
             0x00,0x00, 0x00,0x00, 0x00,0x00, 0x88,0x88},
    .ipmode = NETINFO_STATIC_ALL,
#else
    .dhcp = NETINFO_STATIC,
#endif
};

/* ============================================================ */
/* PPPoE credentials (extern'd by PPPoE.c)                       */
/* ============================================================ */
uint8_t gDATABUF[DATA_BUF_SIZE];

uint8_t pppoe_id[6] = "W6300";
uint8_t pppoe_id_len = 5;
uint8_t pppoe_pw[7] = "WIZnet";
uint8_t pppoe_pw_len = 6;
uint8_t pppoe_ip[4] = {0};

uint16_t pppoe_retry_count = 0;

/* ============================================================ */
/* Timer tick — not used by PPPoE, but defined for consistency   */
/* ============================================================ */
void app_timer_tick(void)
{
    /* PPPoE does not require a periodic tick */
}

/* ============================================================ */
/* Main application entry                                        */
/* ============================================================ */
void app_main(void)
{
    int32_t ret = 0;
    uint8_t str[16];

    printf("\r\n=== PPPoE Example ===\r\n\r\n");

    /* ---- W6300 HW init ---- */
    wizchip_reset();
    wizchip_initialize();

    /* ---- Network init ---- */
    network_initialize(g_net_info);
    print_network_information(g_net_info);

    printf(" PPPoE connecting...\r\n\r\n");

    /* ---- PPPoE connection ---- */
    while (1)
    {
        ret = ppp_start(gDATABUF);

        if (ret == PPP_SUCCESS || pppoe_retry_count > PPP_MAX_RETRY_COUNT)
            break;
    }

    if (ret == PPP_SUCCESS)
    {
        printf("\r\n<<<< PPPoE Success >>>>\r\n");
        printf(" Assigned IP address : %d.%d.%d.%d\r\n",
               pppoe_ip[0], pppoe_ip[1], pppoe_ip[2], pppoe_ip[3]);

        printf("\r\n==================================================\r\n");
        printf("    AFTER PPPoE, Net Configuration Information        \r\n");
        printf("==================================================\r\n");

        getSHAR(str);
        printf(" MAC address    : %x:%x:%x:%x:%x:%x\r\n",
               str[0], str[1], str[2], str[3], str[4], str[5]);
        getSUBR(str);
        printf(" SUBNET MASK    : %d.%d.%d.%d\r\n",
               str[0], str[1], str[2], str[3]);
        getGAR(str);
        printf(" G/W IP ADDRESS : %d.%d.%d.%d\r\n",
               str[0], str[1], str[2], str[3]);
        getSIPR(str);
        printf(" SOURCE IP ADDR : %d.%d.%d.%d\r\n\r\n",
               str[0], str[1], str[2], str[3]);
    }
    else
    {
        printf("\r\n<<<< PPPoE Failed >>>>\r\n");
        print_network_information(g_net_info);
    }

    /* ---- Done ---- */
    while (1)
    {
        ;
    }
}

#endif /* EXAMPLE_PPPOE */
