/**
 * @file    app_main.c
 * @brief   TCP Server over SSL (TLS 1.2) example
 *          for STM32F412 + W6300 + ATECC608C SoM
 *
 * @details Listens for TLS client connections, performs the TLS handshake,
 *          then echoes received data back over the encrypted channel.
 *          Uses mbedTLS 3.6 LTS with ATECC608C for entropy (RNG).
 *
 * @note    Prerequisites:
 *          1. Enable I2C2 in STM32CubeMX (.ioc) for ATECC608C communication.
 *          2. SysTick must call app_timer_tick() every 1 ms.
 *          3. Include paths must have port/mbedtls BEFORE Libraries/mbedtls/include.
 */

#include "main.h"

#ifdef EXAMPLE_TCP_SERVER_OVER_SSL

#include <stdio.h>
#include <string.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "socket.h"
#include "dhcp.h"
#include "atecc608.h"
#include "tls_server.h"
#include "wizchip_tls.h"

/* ============================================================ */
/* Network mode                                                  */
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
/* TLS_SOCKET_NUM (1) is defined in wizchip_tls.h */

/* TLS server port */
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
/* TLS echo server                                               */
/* ============================================================ */
static void tls_echo_server(void)
{
    int ret;
    int8_t sock_ret;
    uint8_t sock_status;
    uint16_t recv_len;
    uint8_t handshake_done = 0;

    printf("\r\n========================================\r\n");
    printf(" TLS 1.2 Echo Server\r\n");
    printf("========================================\r\n");

    ret = tls_server_init();
    if (ret != 0) {
        printf("[TLS] Server init failed: %d\r\n", ret);
        return;
    }

    /* Open listening socket */
    sock_ret = socket(TLS_SOCKET_NUM, Sn_MR_TCP4, TLS_SERVER_PORT, SF_TCP_NODELAY);
    if (sock_ret != TLS_SOCKET_NUM) {
        printf("[TLS] socket() failed: %d\r\n", sock_ret);
        tls_server_free();
        return;
    }

    sock_ret = listen(TLS_SOCKET_NUM);
    if (sock_ret != SOCK_OK) {
        printf("[TLS] listen() failed: %d\r\n", sock_ret);
        close(TLS_SOCKET_NUM);
        tls_server_free();
        return;
    }

    printf("[TLS] Listening on port %d\r\n", TLS_SERVER_PORT);

    /* Server loop */
    while (1)
    {
        sock_status = getSn_SR(TLS_SOCKET_NUM);

        switch (sock_status)
        {
        case SOCK_ESTABLISHED:
            if (getSn_IR(TLS_SOCKET_NUM) & Sn_IR_CON) {
                setSn_IR(TLS_SOCKET_NUM, Sn_IR_CON);
                tls_server_reset();
                handshake_done = 0;
                printf("[TLS] Client connected\r\n");
            }

            if (!handshake_done) {
                ret = tls_server_handshake();
                if (ret != 0) {
                    tls_server_close_notify();
                    disconnect(TLS_SOCKET_NUM);
                    close(TLS_SOCKET_NUM);
                    handshake_done = 0;
                    break;
                }
                handshake_done = 1;

                const char *msg = "Hello from STM32F412-W6300-SoM TLS Server\r\n";
                tls_server_write((const unsigned char *)msg, strlen(msg));
            }

            getsockopt(TLS_SOCKET_NUM, SO_RECVBUF, &recv_len);
            if (recv_len > 0) {
                if (recv_len >= ETHERNET_BUF_MAX_SIZE)
                    recv_len = ETHERNET_BUF_MAX_SIZE - 1;

                memset(g_ethernet_buf, 0, ETHERNET_BUF_MAX_SIZE);
                ret = tls_server_read(g_ethernet_buf, recv_len);

                if (ret > 0) {
                    g_ethernet_buf[ret] = '\0';
                    printf("[TLS] Received %d bytes: %s", ret, g_ethernet_buf);

                    ret = tls_server_write(g_ethernet_buf, ret);
                    if (ret > 0)
                        printf("[TLS] Echo sent %d bytes\r\n", ret);
                } else {
                    printf("[TLS] Read error: %d\r\n", ret);
                    tls_server_close_notify();
                    disconnect(TLS_SOCKET_NUM);
                    close(TLS_SOCKET_NUM);
                    handshake_done = 0;
                }
            }
            break;

        case SOCK_CLOSE_WAIT:
            tls_server_close_notify();
            disconnect(TLS_SOCKET_NUM);
            close(TLS_SOCKET_NUM);
            handshake_done = 0;
            printf("[TLS] Client disconnected\r\n");
            break;

        case SOCK_CLOSED:
            tls_server_reset();
            handshake_done = 0;

            if (socket(TLS_SOCKET_NUM, Sn_MR_TCP4, TLS_SERVER_PORT, SF_TCP_NODELAY) == TLS_SOCKET_NUM) {
                if (listen(TLS_SOCKET_NUM) == SOCK_OK) {
                    printf("[TLS] Re-listening on port %d\r\n", TLS_SERVER_PORT);
                }
            }
            break;

        default:
            break;
        }
    }
}

/* ============================================================ */
/* Timer tick                                                    */
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

    printf("\r\n========================================\r\n");
    printf(" TCP Server over SSL Example\r\n");
    printf(" STM32F412 + W6300 + ATECC608C\r\n");
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

        /* ---- TLS server (runs forever after IP acquired) ---- */
        tls_echo_server();
    }
}

#endif /* EXAMPLE_TCP_SERVER_OVER_SSL */
