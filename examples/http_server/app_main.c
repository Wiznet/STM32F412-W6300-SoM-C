/**
 * @file    app_main.c
 * @brief   HTTP Server example for STM32F412 + W6300 SoM
 *
 * @details Simple HTTP server that serves a static web page.
 *          Open a browser and navigate to the board's IP address.
 *          Based on WIZnet-PICO-C http server example.
 *
 * @note    When using DHCP, SysTick must call app_timer_tick() every 1 ms.
 *          Add to SysTick_Handler() in stm32f4xx_it.c:
 *
 *            extern void app_timer_tick(void);
 *            app_timer_tick();
 */

#include "main.h"

#ifdef EXAMPLE_HTTP_SERVER

#include <stdio.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "httpServer.h"
#include "web_page.h"
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
#define HTTP_SOCKET_MAX_NUM    4
static uint8_t g_http_socket_num_list[HTTP_SOCKET_MAX_NUM] = {1, 2, 3, 4};

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
static uint8_t g_http_send_buf[ETHERNET_BUF_MAX_SIZE] = {0};
static uint8_t g_http_recv_buf[ETHERNET_BUF_MAX_SIZE] = {0};

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
    g_msec_cnt++;

    if (g_msec_cnt >= 1000)
    {
        g_msec_cnt = 0;

        if (g_net_info.dhcp == NETINFO_DHCP)
            DHCP_time_handler();

        httpServer_time_handler();
    }
}

/* ============================================================ */
/* Main application entry                                        */
/* ============================================================ */
void app_main(void)
{
    int32_t retval = 0;
    uint8_t dhcp_retry = 0;
    uint8_t i = 0;

    printf("\r\n=== HTTP Server Example ===\r\n\r\n");

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

    /* ---- HTTP Server init ---- */
    httpServer_init(g_http_send_buf, g_http_recv_buf, HTTP_SOCKET_MAX_NUM, g_http_socket_num_list);
    reg_httpServer_webContent((uint8_t *)"index.html", (uint8_t *)index_page);

    printf(" HTTP server running on port 80\r\n\r\n");

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
                    printf(" DHCP timeout, retry %d\r\n", dhcp_retry);
            }

            if (dhcp_retry > DHCP_RETRY_COUNT)
            {
                printf(" DHCP failed\r\n");
                DHCP_stop();
                while (1)
                    ;
            }

            if (retval != DHCP_IP_LEASED)
                continue;
        }

        /* ---- HTTP Server ---- */
        for (i = 0; i < HTTP_SOCKET_MAX_NUM; i++)
        {
            httpServer_run(i);
        }
    }
}

#endif /* EXAMPLE_HTTP */
