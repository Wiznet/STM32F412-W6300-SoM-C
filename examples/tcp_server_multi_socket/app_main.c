/**
 * @file    app_main.c
 * @brief   TCP Server Multi Socket example for STM32F412 + W6300 SoM
 *
 * @details Opens TCP server on multiple sockets simultaneously.
 *          Each socket listens on the same server port.
 *          Accepted sockets are rotated so the lowest socket does not
 *          keep taking every new connection.
 *          All sockets echo received data back (loopback).
 *          Based on WIZnet-PICO-C tcp_server_multi_socket example.
 *
 * @note    When using DHCP, SysTick must call app_timer_tick() every 1 ms.
 *          Add to SysTick_Handler() in stm32f4xx_it.c:
 *
 *            extern void app_timer_tick(void);
 *            app_timer_tick();
 */

#include "main.h"

#ifdef EXAMPLE_TCP_SERVER_MULTI_SOCKET

#include <stdio.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "socket.h"
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

#define _LOOPBACK_DEBUG_

/* Socket allocation */
#define SOCKET_DHCP            0
#define SOCKET_START           1
#define SOCKET_END             (_WIZCHIP_SOCK_NUM_ - 1)

/* TCP server port shared by all loopback sockets */
#define PORT_TCP_SERVER        5000

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
/* TCP Server Multi Socket loopback                              */
/* Matches WIZnet-PICO-C reference implementation.               */
/* Socket cycles: SOCKET_START ~ (_WIZCHIP_SOCK_NUM_ - 1)       */
/* Port per socket: all sockets use the same port                 */
/* ============================================================ */
static uint8_t g_listen_floor = SOCKET_START;

static uint8_t get_next_socket(uint8_t sock)
{
    if (sock >= SOCKET_END)
        return SOCKET_START;

    return sock + 1;
}

static int32_t loopback_tcps_multi_socket(uint8_t *buf, uint16_t port)
{
    int32_t ret;
    uint16_t size = 0, sentsize = 0;
    uint8_t  destip[4];
    uint16_t destport;
    uint8_t  sn;

    for (sn = SOCKET_START; sn <= SOCKET_END; sn++)
    {
        switch (getSn_SR(sn))
        {
        case SOCK_ESTABLISHED:
            if (getSn_IR(sn) & Sn_IR_CON)
            {
                getSn_DIPR(sn, destip);
                destport = getSn_DPORT(sn);
                printf("%d:Connected - %d.%d.%d.%d : %d\r\n",
                       sn, destip[0], destip[1], destip[2], destip[3], destport);
                setSn_IR(sn, Sn_IR_CON);

                g_listen_floor = get_next_socket(sn);
            }

            if ((size = getSn_RX_RSR(sn)) > 0)
            {
                if (size >= ETHERNET_BUF_MAX_SIZE)
                    size = ETHERNET_BUF_MAX_SIZE - 1;

                ret = recv(sn, buf, size);
                if (ret <= 0)
                    return ret;

                size = (uint16_t)ret;
                buf[size] = 0x00;

                sentsize = 0;
                while (size != sentsize)
                {
                    ret = send(sn, buf + sentsize, size - sentsize);
                    if (ret < 0)
                    {
                        close(sn);
                        return ret;
                    }
                    sentsize += ret;
                }

                getSn_DIPR(sn, destip);
                destport = getSn_DPORT(sn);
                printf("socket%d from:%d.%d.%d.%d port: %d  message:%s\r\n",
                       sn, destip[0], destip[1], destip[2], destip[3], destport, buf);
            }
            break;

        case SOCK_CLOSE_WAIT:
#ifdef _LOOPBACK_DEBUG_
            printf("%d:CloseWait\r\n", sn);
#endif
            if ((ret = disconnect(sn)) != SOCK_OK)
                return ret;
#ifdef _LOOPBACK_DEBUG_
            printf("%d:Socket Closed\r\n", sn);
#endif
            break;

        case SOCK_INIT:
            if (sn < g_listen_floor)
            {
                close(sn);
                break;
            }

#ifdef _LOOPBACK_DEBUG_
            printf("%d:Listen, TCP server loopback, port [%d]\r\n", sn, port);
#endif
            if ((ret = listen(sn)) != SOCK_OK)
                return ret;
            break;

        case SOCK_LISTEN:
            if (sn < g_listen_floor)
                close(sn);
            break;

        case SOCK_CLOSED:
            if (sn < g_listen_floor)
                break;

            if ((ret = socket(sn, Sn_MR_TCP, port, Sn_MR_ND)) != sn)
                return ret;
            break;

        default:
            break;
        }
    }

    return 1;
}

/* ============================================================ */
/* Main application entry                                        */
/* ============================================================ */
void app_main(void)
{
    int32_t retval = 0;
    uint8_t dhcp_retry = 0;

    printf("\r\n=== TCP Server Multi Socket Example ===\r\n\r\n");

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
                dhcp_retry = 0;
            }
            else if (retval == DHCP_IP_CHANGED)
            {
                g_dhcp_get_ip_flag = 1;
                dhcp_retry = 0;
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

            if (g_dhcp_get_ip_flag == 0)
                continue;
        }

        /* ---- Multi socket TCP server ---- */
        loopback_tcps_multi_socket(g_ethernet_buf, PORT_TCP_SERVER);
    }
}

#endif /* EXAMPLE_TCP_SERVER_MULTI_SOCKET */
