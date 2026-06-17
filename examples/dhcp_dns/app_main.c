/**
 * @file    app_dhcp_dns.c
 * @brief   DHCP + DNS example for STM32F412 + W6300 SoM
 *
 * @details Obtains an IP address via DHCP, then resolves a target domain
 *          name using DNS.  Based on WIZnet-PICO-C dhcp_dns example,
 *          adapted for STM32 HAL + W6300 QSPI.
 *
 * @note    SysTick (1 ms tick) must call app_dhcp_dns_time_tick() so that
 *          DHCP and DNS timeouts work correctly.
 *          Add the following to SysTick_Handler() in stm32f4xx_it.c:
 *
 *            extern void app_dhcp_dns_time_tick(void);
 *            app_dhcp_dns_time_tick();
 */
#include "main.h"

#ifdef EXAMPLE_DHCP_DNS

#include <stdio.h>
#include <string.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "socket.h"
#include "dhcp.h"
#include "dns.h"

/* ============================================================ */
/* Configuration                                                 */
/* ============================================================ */
#define SOCKET_DHCP         0
#define SOCKET_DNS          1

#define DHCP_RETRY_COUNT    5
#define DNS_RETRY_COUNT     5

#define ETHERNET_BUF_MAX_SIZE  (1024 * 2)

/* Target domain to resolve */
static uint8_t g_dns_target_domain[] = "example.com";

/* ============================================================ */
/* Network information                                           */
/* ============================================================ */
static wiz_NetInfo g_net_info = {
    .mac  = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
    .ip   = {192, 168, 11, 50},
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
    .dhcp   = NETINFO_DHCP,
    .ipmode = NETINFO_DHCP_V4,
#else
    .dhcp = NETINFO_DHCP,
#endif
};

/* ============================================================ */
/* Buffers & flags                                               */
/* ============================================================ */
static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};

/* DHCP */
static uint8_t g_dhcp_get_ip_flag = 0;

/* DNS */
static uint8_t g_dns_target_ip[4] = {0};
static uint8_t g_dns_get_ip_flag  = 0;

/* Timer (1 ms tick counter → 1 s period for DHCP/DNS handlers) */
static volatile uint16_t g_msec_cnt = 0;

/* ============================================================ */
/* DHCP callbacks                                                */
/* ============================================================ */
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
void app_dhcp_dns_time_tick(void)
{
    g_msec_cnt++;

    if (g_msec_cnt >= 1000)
    {
        g_msec_cnt = 0;
        DHCP_time_handler();
        DNS_time_handler();
    }
}

/* ============================================================ */
/* Main application entry                                        */
/* ============================================================ */
void app_main(void)
{
    uint8_t retval    = 0;
    uint8_t dhcp_retry = 0;
    uint8_t dns_retry  = 0;

    printf("\r\n=== DHCP + DNS Example ===\r\n\r\n");

    /* ---- W6300 HW init ---- */
    wizchip_reset();
    wizchip_initialize();

    /* ---- Network init ---- */
    if (g_net_info.dhcp == NETINFO_DHCP)
    {
        printf(" DHCP client running\r\n");
        DHCP_init(SOCKET_DHCP, g_ethernet_buf);
        reg_dhcp_cbfunc(cb_dhcp_assign, cb_dhcp_assign, cb_dhcp_conflict);
    }
    else
    {
        network_initialize(g_net_info);
        print_network_information(g_net_info);
    }

    /* ---- DNS init ---- */
    DNS_init(SOCKET_DNS, g_ethernet_buf);

    /* ---- Main loop ---- */
    while (1)
    {
        /* ---- DHCP process ---- */
        if (g_net_info.dhcp == NETINFO_DHCP)
        {
            retval = DHCP_run();

            if (retval == DHCP_IP_LEASED)
            {
                if (g_dhcp_get_ip_flag == 0)
                {
                    printf(" DHCP success\r\n");
                    g_dhcp_get_ip_flag = 1;
                }
            }
            else if (retval == DHCP_FAILED)
            {
                g_dhcp_get_ip_flag = 0;
                dhcp_retry++;

                if (dhcp_retry <= DHCP_RETRY_COUNT)
                {
                    printf(" DHCP timeout occurred and retry %d\r\n", dhcp_retry);
                }
            }

            if (dhcp_retry > DHCP_RETRY_COUNT)
            {
                printf(" DHCP failed\r\n");
                DHCP_stop();
                while (1)
                    ;
            }

            HAL_Delay(1000);
        }

        /* ---- DNS process (runs once after DHCP lease) ---- */
        if ((g_dns_get_ip_flag == 0) && (retval == DHCP_IP_LEASED))
        {
            while (1)
            {
                if (DNS_run(g_net_info.dns, g_dns_target_domain, g_dns_target_ip) > 0)
                {
                    printf(" DNS success\r\n");
                    printf(" Target domain : %s\r\n", g_dns_target_domain);
                    printf(" IP of target  : %d.%d.%d.%d\r\n",
                           g_dns_target_ip[0], g_dns_target_ip[1],
                           g_dns_target_ip[2], g_dns_target_ip[3]);

                    g_dns_get_ip_flag = 1;
                    break;
                }
                else
                {
                    dns_retry++;

                    if (dns_retry <= DNS_RETRY_COUNT)
                    {
                        printf(" DNS timeout occurred and retry %d\r\n", dns_retry);
                    }
                }

                if (dns_retry > DNS_RETRY_COUNT)
                {
                    printf(" DNS failed\r\n");
                    while (1)
                        ;
                }

                HAL_Delay(1000);
            }
        }
    }
}

#endif /* EXAMPLE_DHCP_DNS */
