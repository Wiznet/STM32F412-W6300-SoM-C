/**
 * wizchip_dhcp.c
 *
 * DHCP wrapper for W6300 ioLibrary_Driver
 * Style follows WIZnet official reference (RP2040 examples)
 */

#include <stdio.h>
#include "wizchip_dhcp.h"
#include "wizchip_qspi.h"   /* for network_initialize, print_network_information */

/* ============================================================ */
/* Internal state                                                */
/* ============================================================ */
static wiz_NetInfo *gp_net_info       = NULL;
static uint8_t      g_dhcp_retry      = 0;
static uint8_t      g_dhcp_get_ip_flag = 0;

/* ============================================================ */
/* DHCP callbacks (called by ioLibrary's DHCP_run)               */
/* ============================================================ */
static void on_dhcp_assign(void)
{
    /* Copy assigned network info from DHCP into the application struct */
    getIPfromDHCP(gp_net_info->ip);
    getGWfromDHCP(gp_net_info->gw);
    getSNfromDHCP(gp_net_info->sn);
    getDNSfromDHCP(gp_net_info->dns);

    gp_net_info->dhcp = NETINFO_DHCP;

    /* Apply network info to W6300 and print */
    network_initialize(*gp_net_info);
    print_network_information(*gp_net_info);

    printf(" DHCP leased time : %ld seconds\r\n", getDHCPLeasetime());
}

static void on_dhcp_conflict(void)
{
    printf(" Conflict IP from DHCP\r\n");

    /* Halt here (could also reset or take other action) */
    while (1)
        ;
}

/* ============================================================ */
/* Public API                                                    */
/* ============================================================ */
void wizchip_dhcp_init(wiz_NetInfo *net_info, uint8_t *buf)
{
    printf(" DHCP client running\r\n");

    gp_net_info        = net_info;
    g_dhcp_retry       = 0;
    g_dhcp_get_ip_flag = 0;

    /* Initialize DHCP on the given socket with provided buffer */
    DHCP_init(SOCKET_DHCP, buf);

    /* Use the same callback for assign and update */
    reg_dhcp_cbfunc(on_dhcp_assign, on_dhcp_assign, on_dhcp_conflict);
}

uint8_t wizchip_dhcp_run(void)
{
    uint8_t result = DHCP_run();

    if (result == DHCP_IP_LEASED)
    {
        /* Print "DHCP success" only once per lease */
        if (g_dhcp_get_ip_flag == 0)
        {
            printf(" DHCP success\r\n");
            g_dhcp_get_ip_flag = 1;
        }
    }
    else if (result == DHCP_FAILED)
    {
        g_dhcp_get_ip_flag = 0;
        g_dhcp_retry++;

        if (g_dhcp_retry <= DHCP_RETRY_COUNT)
        {
            printf(" DHCP timeout occurred and retry %d\r\n", g_dhcp_retry);
        }
    }

    /* Stop DHCP after too many failures */
    if (g_dhcp_retry > DHCP_RETRY_COUNT)
    {
        printf(" DHCP failed\r\n");
        DHCP_stop();

        /* Halt here (application can decide otherwise) */
        while (1)
            ;
    }

    return result;
}

void wizchip_dhcp_time_tick(void)
{
    DHCP_time_handler();
}
