/**
 * @file    app_main.c
 * @brief   TCP Client over SSL (TLS 1.2) example
 *          for STM32F412 + W6300 + ATECC608C-TNGTLS SoM
 *
 * @details Connects to a TLS server, sends a hello message, then echoes
 *          received data back until the server closes the connection.
 *          Uses mbedTLS 3.6 LTS with ATECC608C for entropy (RNG).
 *
 * @note    Prerequisites:
 *          1. Enable I2C2 in STM32CubeMX (.ioc) for ATECC608C communication.
 *             - Add MX_I2C2_Init() call in main.c
 *             - Declare: I2C_HandleTypeDef hi2c2;
 *          2. SysTick must call app_timer_tick() every 1 ms.
 *          3. Include paths must have port/mbedtls BEFORE Libraries/mbedtls/include
 *             so the custom mbedtls_config.h takes priority.
 */

#include "main.h"

#ifdef EXAMPLE_TCP_CLIENT_OVER_SSL

#include <stdio.h>
#include <string.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "dhcp.h"
#include "atecc608.h"
#include "tls_client.h"

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
/* TLS_SOCKET_NUM (1) is defined in w6300_tls_transport.h */

/* TLS server address -- change to your server's IP */
static uint8_t g_tls_server_ip[] = {192, 168, 11, 2};
#define TLS_SERVER_PORT        443

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
/* TLS echo test                                                 */
/* ============================================================ */
static void tls_echo_test(void)
{
    int ret;
    unsigned char buf[256];

    printf("\r\n========================================\r\n");
    printf(" TLS 1.2 Client Echo Test\r\n");
    printf("========================================\r\n");

    ret = tls_client_init();
    if (ret != 0) {
        printf("[TLS] Init failed: %d\r\n", ret);
        return;
    }

    ret = tls_client_connect(g_tls_server_ip, TLS_SERVER_PORT, NULL);
    if (ret != 0) {
        printf("[TLS] Connect failed: %d\r\n", ret);
        tls_client_close();
        return;
    }

    const char *msg = "Hello from STM32F412-W6300-SoM TLS Client\r\n";
    ret = tls_client_write((const unsigned char *)msg, strlen(msg));
    printf("[TLS] Sent %d bytes: %s", ret, msg);

    while (1) {
        ret = tls_client_read(buf, sizeof(buf) - 1);
        if (ret > 0) {
            buf[ret] = '\0';
            printf("[TLS] Received %d bytes: %s\r\n", ret, buf);

            ret = tls_client_write(buf, ret);
            if (ret <= 0) {
                printf("[TLS] Write failed: %d\r\n", ret);
                break;
            }
            printf("[TLS] Echo sent %d bytes\r\n", ret);
        } else if (ret == 0) {
            printf("[TLS] Connection closed by server\r\n");
            break;
        } else {
            printf("[TLS] Read error: -0x%04X\r\n", (unsigned)-ret);
            break;
        }
    }

    tls_client_close();
    printf("[TLS] Test complete\r\n");
}

/* ============================================================ */
/* Timer tick -- call from SysTick_Handler every 1 ms            */
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
    int32_t retval = 0;
    uint8_t dhcp_retry = 0;
    uint8_t tls_done = 0;

    printf("\r\n========================================\r\n");
    printf(" TCP Client over SSL Example\r\n");
    printf(" STM32F412 + W6300 + ATECC608C-TNGTLS\r\n");
    printf("========================================\r\n\r\n");

    /* ---- ATECC608 init ---- */
    if (atecc608_init() != ATCA_SUCCESS)
    {
        printf("[ATECC608] Init failed! Halting.\r\n");
        while (1)
            ;
    }
    printf("[ATECC608] Init OK\r\n");

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

        /* ---- TLS test (run once after IP acquired) ---- */
        if (!tls_done)
        {
            tls_done = 1;
            tls_echo_test();
        }
    }
}

#endif /* EXAMPLE_TCP_CLIENT_OVER_SSL */
