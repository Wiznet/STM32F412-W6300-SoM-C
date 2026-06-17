/**
 * @file    app_main.c
 * @brief   UPnP example for STM32F412 + W6300 SoM
 *
 * @details UPnP (Universal Plug and Play) IGD control point.
 *          Discovers an Internet Gateway Device via SSDP,
 *          then provides an interactive menu for port forwarding
 *          (AddPortMapping / DeletePortMapping).
 *          Based on WIZnet-PICO-C upnp example.
 *
 * @note    Required source files from PICO-C examples/upnp/:
 *            - src/UPnP.c, src/MakeXML.c, src/hyperterminal.c
 *            - inc/UPnP.h, inc/MakeXML.h, inc/hyperterminal.h
 *          Modify UPnP.c:
 *            - Remove #include "pico/time.h"
 *            - Replace sleep_ms() with HAL_Delay()
 *
 * @note    When using DHCP, SysTick must call app_timer_tick() every 1 ms.
 *          Add to SysTick_Handler() in stm32f4xx_it.c:
 *
 *            extern void app_timer_tick(void);
 *            app_timer_tick();
 */

#include "main.h"

#ifdef EXAMPLE_UPNP

#include <stdio.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "dhcp.h"

#include "hyperterminal.h"
#include "MakeXML.h"
#include "UPnP.h"

/* ============================================================ */
/* Network mode: NETINFO_DHCP or NETINFO_STATIC                  */
/* ============================================================ */
#define NET_MODE    NETINFO_DHCP
//#define NET_MODE    NETINFO_STATIC

/* ============================================================ */
/* Configuration                                                 */
/* ============================================================ */
#define ETHERNET_BUF_MAX_SIZE  (1024 * 4)
#define DHCP_RETRY_COUNT       5

/* Socket allocation */
#define SOCKET_DHCP            0
#define SOCKET_UPNP            1

/* Port */
#define PORT_TCP               8000
#define PORT_UDP               5000

/* Socket buffer sizes */
static uint8_t tx_size[_WIZCHIP_SOCK_NUM_] = {4, 4, 2, 1, 1, 1, 1, 2};
static uint8_t rx_size[_WIZCHIP_SOCK_NUM_] = {4, 4, 2, 1, 1, 1, 1, 2};

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
static uint8_t g_upnp_buf[ETHERNET_BUF_MAX_SIZE] = {0};

/* ============================================================ */
/* UPnP timer (my_time is extern'd by UPnP.c)                   */
/* ============================================================ */
extern unsigned long my_time;

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
/* LED control (stub — replace with actual GPIO if needed)       */
/* ============================================================ */
static void setUserLEDStatus(uint8_t val)
{
    /* Replace with your board's LED GPIO if available */
    /* Example: HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET); */
    printf(" LED %s\r\n", val ? "ON" : "OFF");
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
        my_time++;

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

    printf("\r\n=== UPnP Example ===\r\n\r\n");

    /* ---- W6300 HW init ---- */
    wizchip_reset();
    wizchip_initialize();
    wizchip_init(tx_size, rx_size);

    /* ---- LED init ---- */
    UserLED_Control_Init(setUserLEDStatus);

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

    /* ---- UPnP SSDP Discovery ---- */
    do
    {
        printf(" Send SSDP..\r\n");
    } while (SSDPProcess(SOCKET_UPNP) != 0);

    /* ---- Get IGD Description ---- */
    if (GetDescriptionProcess(SOCKET_UPNP) == 0)
        printf(" GetDescription Success!!\r\n");
    else
        printf(" GetDescription Fail!!\r\n");

    /* ---- Subscribe to Events ---- */
    if (SetEventing(SOCKET_UPNP) == 0)
        printf(" SetEventing Success!!\r\n");
    else
        printf(" SetEventing Fail!!\r\n");

    /* ---- Interactive Menu ---- */
    Main_Menu(SOCKET_UPNP, SOCKET_UPNP + 1, SOCKET_UPNP + 2,
              g_upnp_buf, PORT_TCP, PORT_UDP);

    /* ---- Done ---- */
    while (1)
    {
        ;
    }
}

#endif /* EXAMPLE_UPNP */
