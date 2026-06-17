/**
 * @file    app_main.c
 * @brief   TFTP Client example for STM32F412 + W6300 SoM
 *
 * @details Reads a file from a TFTP server on the network.
 *          Based on WIZnet-PICO-C tftp example.
 *
 *          Requires a TFTP server (e.g. SolarWinds TFTP Server)
 *          running on the network with the target file.
 *
 * @note    SysTick must call app_timer_tick() every 1 ms.
 *          Add to SysTick_Handler() in stm32f4xx_it.c:
 *
 *            extern void app_timer_tick(void);
 *            app_timer_tick();
 */

#include "main.h"

#ifdef EXAMPLE_TFTP

#include <stdio.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "tftp.h"
#include "netutil.h"
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
#define TFTP_SOCKET_ID         1

/* TFTP */
#define TFTP_CLIENT_SOCKET_BUFFER_SIZE  2048
#define TFTP_SERVER_IP         "192.168.11.2"
#define TFTP_SERVER_FILE_NAME  "tftp_test_file.txt"

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
static uint8_t tftp_client_socket_buffer[TFTP_CLIENT_SOCKET_BUFFER_SIZE] = {0};

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
/* tftp_timeout_handler() is required by the TFTP library.       */
/* ============================================================ */
void app_timer_tick(void)
{
    tftp_timeout_handler();

    g_msec_cnt++;
    if (g_msec_cnt >= 1000)
    {
        g_msec_cnt = 0;

        if (g_net_info.dhcp == NETINFO_DHCP)
            DHCP_time_handler();
    }
}

/* ============================================================ */
/* Main application entry                                        */
/* ============================================================ */
void app_main(void)
{
    int32_t retval = 0;
    uint8_t dhcp_retry = 0;
    int tftp_state;
    uint8_t tftp_read_flag = 0;
    uint32_t tftp_server_ip = inet_addr((uint8_t *)TFTP_SERVER_IP);

    printf("\r\n=== TFTP Client Example ===\r\n\r\n");

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

    /* ---- TFTP init ---- */
    TFTP_init(TFTP_SOCKET_ID, tftp_client_socket_buffer);

    /* ---- Main loop ---- */
    while (1)
    {
        if (tftp_read_flag == 0)
        {
            printf(" TFTP server IP : %s\r\n", TFTP_SERVER_IP);
            printf(" File name      : %s\r\n", TFTP_SERVER_FILE_NAME);
            printf(" Sending read request...\r\n");

            TFTP_read_request(tftp_server_ip, (uint8_t *)TFTP_SERVER_FILE_NAME);
            tftp_read_flag = 1;
        }
        else
        {
            tftp_state = TFTP_run();

            if (tftp_state == TFTP_SUCCESS)
            {
                printf(" TFTP read success : %s\r\n", TFTP_SERVER_FILE_NAME);
                while (1)
                    ;
            }
            else if (tftp_state == TFTP_FAIL)
            {
                printf(" TFTP read failed : %s\r\n", TFTP_SERVER_FILE_NAME);
                while (1)
                    ;
            }
        }
    }
}

#endif /* EXAMPLE_TFTP */
