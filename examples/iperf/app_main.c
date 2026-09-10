/**
 * @file    app_main.c
 * @brief   iperf2 throughput test for STM32F412 + W6300 SoM
 *
 * @details TCP iperf2 server. The W6300 hardware TCP (TOE) receives and discards
 *          data as fast as it can; the PC-side iperf client reports the throughput.
 *          Uses the ioLibrary socket API — ported from
 *          WIZnet-PICO-IPERF3-C / examples/iperf2 (wizchip_iperf.c).
 *
 *          Supports both DHCP and static IP — change NET_MODE below.
 *          TCP port 5001. DHCP uses socket 0, iperf data uses socket 1.
 *
 * @note    iperf VERSION 2 only (not iperf3). On the PC run, e.g.:
 *              iperf -c <board_ip> -i 1 -t 10
 *          With DHCP the leased IP is printed over the UART at boot — use that IP.
 *
 * @note    When using DHCP, SysTick must call app_timer_tick() every 1 ms.
 *          stm32f4xx_it.c SysTick_Handler already calls it:
 *
 *            extern void app_timer_tick(void);
 *            app_timer_tick();
 */

#include "main.h"

#ifdef EXAMPLE_IPERF

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
#define ETHERNET_BUF_MAX_SIZE  (1024 * 2)    /* DHCP messages are small */

/* iperf recv chunk. Every recv() cycle costs a fixed overhead - getSn_SR, the
 * getSn_RX_RSR double read, getSn_RX_RD/setSn_RX_RD, setSn_CR and the Sn_CR
 * spin, each a separate QSPI transaction - and that cost does not scale with
 * the QSPI clock. A larger chunk amortises it, which makes this the dominant
 * throughput knob. Keep <= the socket RX buffer set in wizchip_initialize(). */
#define IPERF_BUF_MAX_SIZE     (1024 * 16)
#define DHCP_RETRY_COUNT       5

/* Socket allocation */
#define SOCKET_DHCP            0
#define SOCKET_IPERF           1

/* Port */
#define PORT_IPERF             5001

/* ============================================================ */
/* Network information                                           */
/* ============================================================ */
static wiz_NetInfo g_net_info = {
    .mac  = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
    .ip   = {192, 168, 11, 7},
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
static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};   /* DHCP */
static uint8_t g_iperf_buf[IPERF_BUF_MAX_SIZE]       = {0};    /* iperf recv */

/* ============================================================ */
/* DHCP                                                          */
/* ============================================================ */
static uint8_t g_dhcp_get_ip_flag = 0;
static volatile uint16_t g_msec_cnt = 0;
static volatile uint8_t  g_dhcp_tick = 0;   /* set once per second by app_timer_tick() */

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
    printf(" iperf2 server ready. On the PC (iperf v2) run:\r\n");
    printf("   iperf -c %d.%d.%d.%d -i 1 -t 10\r\n\r\n",
           g_net_info.ip[0], g_net_info.ip[1], g_net_info.ip[2], g_net_info.ip[3]);
}

static void cb_dhcp_conflict(void)
{
    printf(" Conflict IP from DHCP\r\n");
    while (1)
        ;
}

/* ============================================================ */
/* Timer tick — called from SysTick_Handler every 1 ms           */
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
        g_dhcp_tick = 1;        /* tells the main loop to service DHCP */
    }
}

/* ============================================================ */
/* iperf2 server state machine (receive & discard)              */
/* ============================================================ */
static void iperf_run(void)
{
    int32_t  ret;
    uint16_t size;

    switch (getSn_SR(SOCKET_IPERF))
    {
        case SOCK_ESTABLISHED:
            size = getSn_RX_RSR(SOCKET_IPERF);
            if (size > 0)
            {
                if (size > (IPERF_BUF_MAX_SIZE - 1))
                    size = IPERF_BUF_MAX_SIZE - 1;

                ret = recv(SOCKET_IPERF, g_iperf_buf, size);   /* drain, don't echo */
                if (ret < 0)
                {
                    printf(" recv error %ld, closing socket\r\n", (long)ret);
                    close(SOCKET_IPERF);
                }
            }
            break;

        case SOCK_CLOSE_WAIT:
            /* One line per test run. A QSPI transfer cannot report failure to
             * its caller (ioLibrary fixes the callback signature), so this
             * counter is the only way to tell a clean run from one that lost
             * transfers - a stall with errors at 0 is a network or host
             * problem, not the QSPI link. */
            printf(" QSPI errors: %lu\r\n", W6300_QspiGetErrorCount());
            disconnect(SOCKET_IPERF);
            break;

        case SOCK_INIT:
            listen(SOCKET_IPERF);
            break;

        case SOCK_CLOSED:
            /* SF_TCP_NODELAY is required, not an optimisation. The W6300 uses
             * delayed ACK by default and, per w6300.h (Sn_MR_ND), only ACKs
             * promptly "when SOCKETn window size is less than MSS after
             * Sn_CR_RECV". This example drains fast, so with an RX buffer larger
             * than one MSS the free window never falls that low and every window
             * waits out the delayed-ACK timer instead. The resulting stall looks
             * exactly like packet loss but is not. A slower drain or a smaller
             * RX buffer hides it, so it only shows up once the rest of the
             * stack is fast. */
            socket(SOCKET_IPERF, Sn_MR_TCP, PORT_IPERF, SF_TCP_NODELAY);
            break;

        default:
            break;
    }
}

/* ============================================================ */
/* Main application entry                                        */
/* ============================================================ */
void app_main(void)
{
    int32_t retval = 0;
    uint8_t dhcp_retry = 0;

    printf("\r\n=== iperf2 Example (TCP server, port %d) ===\r\n\r\n", PORT_IPERF);

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
        printf(" iperf2 server ready. On the PC (iperf v2) run:\r\n");
        printf("   iperf -c %d.%d.%d.%d -i 1 -t 10\r\n\r\n",
               g_net_info.ip[0], g_net_info.ip[1], g_net_info.ip[2], g_net_info.ip[3]);
    }

    /* ---- Main loop ---- */
    while (1)
    {
        /* ---- DHCP process ---- */
        if (g_net_info.dhcp == NETINFO_DHCP)
        {
            /* Poll hard until the lease arrives; after that DHCP only has to
             * handle renewal, so service it once per second. Calling DHCP_run()
             * every loop iteration polls socket registers over QSPI hundreds of
             * times a second and competes with recv() for the bus. */
            if (g_dhcp_get_ip_flag == 0 || g_dhcp_tick)
            {
                g_dhcp_tick = 0;
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
            }

            /* Don't run iperf until we have a lease. */
            if (g_dhcp_get_ip_flag == 0)
                continue;
        }

        /* ---- iperf2 server ---- */
        iperf_run();
    }
}

#endif /* EXAMPLE_IPERF */
