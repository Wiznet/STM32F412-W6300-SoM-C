/**
 * @file    app_main.c
 * @brief   SNTP example for STM32F412 + W6300 SoM
 *
 * @details Gets current time from an SNTP server (time.google.com).
 *          Based on WIZnet-PICO-C sntp example.
 *
 * @note    When using DHCP, SysTick must call app_timer_tick() every 1 ms.
 *          Add to SysTick_Handler() in stm32f4xx_it.c:
 *
 *            extern void app_timer_tick(void);
 *            app_timer_tick();
 */

#include "main.h"

#ifdef EXAMPLE_SNTP

#include <stdio.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "sntp.h"
#include "dhcp.h"

/* ============================================================ */
/* Network mode: NETINFO_DHCP or NETINFO_STATIC                  */
/* ============================================================ */
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC

/* ============================================================ */
/* Configuration                                                 */
/* ============================================================ */
#define ETHERNET_BUF_MAX_SIZE  (1024 * 2)
#define DHCP_RETRY_COUNT       5

/* Socket allocation */
#define SOCKET_DHCP            0
#define SOCKET_SNTP            1

/* SNTP */
#define RECV_TIMEOUT           (1000 * 10)  /* 10 seconds */
#define TIMEZONE               40           /* Korea (UTC+9) */

static uint8_t g_sntp_server_ip[4] = {216, 239, 35, 0}; /* time.google.com */

/* ============================================================ */
/* Network information                                           */
/* ============================================================ */
static wiz_NetInfo g_net_info = {
    .mac  = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
    .ip   = {192, 168, 11, 2},
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
    .dhcp   = NET_MODE,
    .ipmode = (NET_MODE == NETINFO_DHCP) ? NETINFO_DHCP_V4 : NETINFO_STATIC_ALL,
#else
    .dhcp = NET_MODE,
#endif
};

/* ============================================================ */
/* Buffers                                                       */
/* ============================================================ */
static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};
static uint8_t g_sntp_buf[ETHERNET_BUF_MAX_SIZE] = {0};

/* ============================================================ */
/* DHCP                                                          */
/* ============================================================ */
static uint8_t g_dhcp_get_ip_flag = 0;
static volatile uint16_t g_msec_cnt = 0;

static void cb_dhcp_assign(void)
{
    getIPfromDHCP(g_net_info.ip);
    getGWfromDHCP(g_net_info.gw);
    getSNfromDHCP(g_net_info.sn);
    getDNSfromDHCP(g_net_info.dns);

    g_net_info.dhcp = NETINFO_DHCP;

    network_initialize(g_net_info);
    print_network_information(g_net_info);

    printf(" DHCP leased time : %ld seconds\r\n", getDHCPLeasetime());
}

static void cb_dhcp_conflict(void)
{
    printf(" Conflict IP from DHCP\r\n");
    while (1)
        ;
}

/* ============================================================ */
/* Timer tick — call from SysTick_Handler every 1 ms             */
/* ============================================================ */
void app_timer_tick(void)
{
    if (g_net_info.dhcp != NETINFO_DHCP)
        return;

    g_msec_cnt++;
    if (g_msec_cnt >= 1000)
    {
        g_msec_cnt = 0;
        DHCP_time_handler();
    }
}

/* ============================================================ */
/* Main application entry                                        */
/* ============================================================ */
void app_main(void)
{
    uint8_t retval = 0;
    uint8_t dhcp_retry = 0;
    uint32_t start_ms = 0;
    datetime time;

    printf("\r\n=== SNTP Example ===\r\n\r\n");

    /* ---- W6300 HW init ---- */
    wizchip_reset();
    wizchip_initialize();

    /* ---- Network init ---- */
    if (g_net_info.dhcp == NETINFO_DHCP)
    {
        printf(" DHCP client running\r\n");
        DHCP_init(SOCKET_DHCP, g_ethernet_buf);
        reg_dhcp_cbfunc(cb_dhcp_assign, cb_dhcp_assign, cb_dhcp_conflict);

        /* Wait for DHCP lease */
        while (1)
        {
            retval = DHCP_run();

            if (retval == DHCP_IP_LEASED)
            {
                printf(" DHCP success\r\n");
                break;
            }
            else if (retval == DHCP_FAILED)
            {
                dhcp_retry++;

                if (dhcp_retry <= DHCP_RETRY_COUNT)
                    printf(" DHCP timeout, retry %d\r\n", dhcp_retry);
            }

            if (dhcp_retry > DHCP_RETRY_COUNT)
            {
                printf(" DHCP failed\r\n");
                while (1)
                    ;
            }
        }
    }
    else
    {
        network_initialize(g_net_info);
        print_network_information(g_net_info);
    }

    /* ---- SNTP ---- */
    SNTP_init(SOCKET_SNTP, g_sntp_server_ip, TIMEZONE, g_sntp_buf);

    printf(" SNTP server : %d.%d.%d.%d\r\n",
           g_sntp_server_ip[0], g_sntp_server_ip[1],
           g_sntp_server_ip[2], g_sntp_server_ip[3]);
    printf(" Requesting time...\r\n");

    start_ms = HAL_GetTick();

    /* Get time */
    do
    {
        retval = SNTP_run(&time);

        if (retval == 1)
            break;

    } while ((HAL_GetTick() - start_ms) < RECV_TIMEOUT);

    if (retval != 1)
    {
        printf(" SNTP failed : %d\r\n", retval);
        while (1)
            ;
    }

    printf(" %d-%02d-%02d, %02d:%02d:%02d\r\n",
           time.yy, time.mo, time.dd,
           time.hh, time.mm, time.ss);

    /* ---- Done ---- */
    while (1)
    {
        ;
    }
}

#endif /* EXAMPLE_SNTP */
