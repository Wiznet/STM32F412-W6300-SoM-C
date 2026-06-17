/**
 * @file    app_main.c
 * @brief   Network Install example for STM32F412 + W6300 SoM
 *
 * @details Initializes network, checks PHY link status and configuration,
 *          then waits for a ping to verify connectivity.
 *          Based on WIZnet-PICO-C network_install example.
 *
 * @note    When using DHCP, SysTick must call app_timer_tick() every 1 ms.
 *          Add to SysTick_Handler() in stm32f4xx_it.c:
 *
 *            extern void app_timer_tick(void);
 *            app_timer_tick();
 */

#include "main.h"

#ifdef EXAMPLE_NETWORK_INSTALL

#include <stdio.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
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
    uint8_t link_status;
    wiz_PhyConf phyconf;
    uint16_t count = 0;
    uint8_t dhcp_retry = 0;
    int32_t retval = 0;

    printf("\r\n=== Network Install Example ===\r\n\r\n");

    /* ---- W6300 HW init ---- */
    wizchip_reset();
    wizchip_initialize();

    /* ---- Network init ---- */
    if (g_net_info.dhcp == NETINFO_DHCP)
    {
        printf(" DHCP client running\r\n");
        DHCP_init(SOCKET_DHCP, g_ethernet_buf);
        reg_dhcp_cbfunc(cb_dhcp_assign, cb_dhcp_assign, cb_dhcp_conflict);

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

    /* ---- Check PHY link ---- */
    do
    {
        link_status = wizphy_getphylink();
        printf("%u", link_status);

        if (link_status == PHY_LINK_OFF)
        {
            count++;
            if (count > 10)
            {
                printf("\r\n Link failed of Internal PHY.\r\n");
                break;
            }
        }

        HAL_Delay(500);

    } while (link_status == PHY_LINK_OFF);

    if (link_status == PHY_LINK_ON)
    {
        wizphy_getphyconf(&phyconf);
        printf("\r\n Link OK of Internal PHY.\r\n");
        printf(" The %d Mbps speed of Internal PHY.\r\n",
               phyconf.speed == PHY_SPEED_10 ? 10 : 100);
        printf(" The %s Duplex Mode of the Internal PHY.\r\n",
               phyconf.duplex == PHY_DUPLEX_FULL ? "Full-Duplex" : "Half-Duplex");

        printf("\r\n Try ping the IP : %d.%d.%d.%d\r\n",
               g_net_info.ip[0], g_net_info.ip[1],
               g_net_info.ip[2], g_net_info.ip[3]);
    }
    else
    {
        printf("\r\n Please check whether the network cable is loose or disconnected.\r\n");
    }

    /* ---- Done ---- */
    while (1)
    {
        ;
    }
}

#endif /* EXAMPLE_NETWORK_INSTALL */
