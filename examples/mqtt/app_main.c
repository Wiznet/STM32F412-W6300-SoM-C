/**
 * @file    app_main.c
 * @brief   MQTT Publish & Subscribe example for STM32F412 + W6300 SoM
 *
 * @details Connects to an MQTT broker, subscribes to a topic, and
 *          periodically publishes a message.
 *          Based on WIZnet-PICO-C mqtt publish_subscribe example.
 *
 *          Requires Mosquitto or another MQTT broker running on the network.
 *
 * @note    SysTick must call app_timer_tick() every 1 ms.
 *          Add to SysTick_Handler() in stm32f4xx_it.c:
 *
 *            extern void app_timer_tick(void);
 *            app_timer_tick();
 */

#include "main.h"

#ifdef EXAMPLE_MQTT

#include <stdio.h>
#include <string.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "socket.h"
#include "dhcp.h"

#include "MQTTClient.h"

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
#define SOCKET_MQTT            1

/* MQTT broker */
#define PORT_MQTT              1883
#define DEFAULT_TIMEOUT        (1000 * 1)  /* 1 second */

/* Broker address. With NET_MODE set to NETINFO_STATIC this must differ from
 * g_net_info.ip below — otherwise the board and the broker claim the same
 * address and the connection can never complete. */
static uint8_t g_mqtt_broker_ip[4] = {192, 168, 11, 100};

/* MQTT settings */
#define MQTT_CLIENT_ID         "w6300-som"
#define MQTT_USERNAME          "wiznet"
#define MQTT_PASSWORD          "0123456789"
#define MQTT_PUBLISH_TOPIC     "publish_topic"
#define MQTT_PUBLISH_PAYLOAD   "Hello, World!"
#define MQTT_PUBLISH_PERIOD    (1000 * 10)  /* 10 seconds */
#define MQTT_SUBSCRIBE_TOPIC   "subscribe_topic"
#define MQTT_KEEP_ALIVE        60           /* seconds */

/* MQTTYield() takes a timeout in milliseconds, not the keep-alive interval.
 * It is how long one yield waits for an inbound packet before returning. */
#define MQTT_YIELD_TIMEOUT     100          /* ms */
#define MQTT_RECONNECT_DELAY   (1000 * 5)   /* 5 seconds */

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
static uint8_t g_mqtt_send_buf[ETHERNET_BUF_MAX_SIZE] = {0};
static uint8_t g_mqtt_recv_buf[ETHERNET_BUF_MAX_SIZE] = {0};

/* ============================================================ */
/* MQTT objects                                                  */
/* ============================================================ */
static Network g_mqtt_network;
static MQTTClient g_mqtt_client;
static MQTTPacket_connectData g_mqtt_packet_connect_data = MQTTPacket_connectData_initializer;
static MQTTMessage g_mqtt_message;

/* ============================================================ */
/* DHCP                                                          */
/* ============================================================ */
static volatile uint16_t g_msec_cnt = 0;
static volatile uint8_t g_dhcp_tick = 0;

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
/* MQTT subscribe callback                                       */
/* ============================================================ */
static void message_arrived(MessageData *msg_data)
{
    MQTTMessage *message = msg_data->message;

    printf(" [SUB] %.*s\r\n", (int)message->payloadlen, (uint8_t *)message->payload);
}

/* ============================================================ */
/* Timer tick — call from SysTick_Handler every 1 ms             */
/* MilliTimer_Handler() is required by the MQTT library.         */
/* ============================================================ */
void app_timer_tick(void)
{
    MilliTimer_Handler();

    g_msec_cnt++;
    if (g_msec_cnt >= 1000)
    {
        g_msec_cnt = 0;

        if (g_net_info.dhcp == NETINFO_DHCP)
        {
            DHCP_time_handler();
            g_dhcp_tick = 1;
        }
    }
}

/* ============================================================ */
/* MQTT session open / close                                     */
/* ============================================================ */
static void mqtt_session_close(void)
{
    disconnect(SOCKET_MQTT);
    close(SOCKET_MQTT);
}

static int32_t mqtt_session_open(void)
{
    int32_t retval;

    if (ConnectNetwork(&g_mqtt_network, g_mqtt_broker_ip, PORT_MQTT) != SOCK_OK)
    {
        printf(" Network connect failed\r\n");
        return -1;
    }

    MQTTClientInit(&g_mqtt_client, &g_mqtt_network, DEFAULT_TIMEOUT,
                   g_mqtt_send_buf, ETHERNET_BUF_MAX_SIZE,
                   g_mqtt_recv_buf, ETHERNET_BUF_MAX_SIZE);

    g_mqtt_packet_connect_data.MQTTVersion = 3;
    g_mqtt_packet_connect_data.cleansession = 1;
    g_mqtt_packet_connect_data.willFlag = 0;
    g_mqtt_packet_connect_data.keepAliveInterval = MQTT_KEEP_ALIVE;
    g_mqtt_packet_connect_data.clientID.cstring = MQTT_CLIENT_ID;
    g_mqtt_packet_connect_data.username.cstring = MQTT_USERNAME;
    g_mqtt_packet_connect_data.password.cstring = MQTT_PASSWORD;

    retval = MQTTConnect(&g_mqtt_client, &g_mqtt_packet_connect_data);

    if (retval < 0)
    {
        printf(" MQTT connect failed : %ld\r\n", retval);
        return -1;
    }

    retval = MQTTSubscribe(&g_mqtt_client, MQTT_SUBSCRIBE_TOPIC, QOS0, message_arrived);

    if (retval < 0)
    {
        printf(" Subscribe failed : %ld\r\n", retval);
        return -1;
    }

    printf(" MQTT connected\r\n");
    printf(" Subscribed to '%s'\r\n", MQTT_SUBSCRIBE_TOPIC);
    printf(" Publishing to '%s' every %d seconds\r\n\r\n",
           MQTT_PUBLISH_TOPIC, MQTT_PUBLISH_PERIOD / 1000);

    return 0;
}

/* ============================================================ */
/* Main application entry                                        */
/* ============================================================ */
void app_main(void)
{
    int32_t retval = 0;
    uint8_t dhcp_retry = 0;
    uint8_t mqtt_connected = 0;
    uint32_t start_ms = 0;
    uint32_t end_ms = 0;

    printf("\r\n=== MQTT Publish & Subscribe Example ===\r\n\r\n");

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

    /* ---- MQTT setup ---- */
    NewNetwork(&g_mqtt_network, SOCKET_MQTT);

    /* Configure publish message */
    g_mqtt_message.qos = QOS0;
    g_mqtt_message.retained = 0;
    g_mqtt_message.dup = 0;
    g_mqtt_message.payload = MQTT_PUBLISH_PAYLOAD;
    g_mqtt_message.payloadlen = strlen(g_mqtt_message.payload);

    /* ---- Main loop ---- */
    while (1)
    {
        /* ---- DHCP renewal ---- */
        /* The lease is not permanent: DHCP_run() has to keep being called or
         * the address is lost when the lease expires. Once per second is
         * enough, and it keeps DHCP off the QSPI bus the rest of the time. */
        if (g_net_info.dhcp == NETINFO_DHCP && g_dhcp_tick)
        {
            g_dhcp_tick = 0;
            retval = DHCP_run();

            if (retval == DHCP_IP_CHANGED)
            {
                /* The session belongs to the old address — start over. */
                printf(" DHCP IP changed\r\n");

                mqtt_connected = 0;
                mqtt_session_close();
                continue;
            }
            else if (retval == DHCP_FAILED)
            {
                printf(" DHCP renewal failed\r\n");
            }
        }

        /* ---- (Re)connect to the broker ---- */
        if (!mqtt_connected)
        {
            if (mqtt_session_open() == 0)
            {
                mqtt_connected = 1;
                start_ms = HAL_GetTick();
            }
            else
            {
                mqtt_session_close();
                printf(" Retry in %d seconds\r\n", MQTT_RECONNECT_DELAY / 1000);
                HAL_Delay(MQTT_RECONNECT_DELAY);
            }

            continue;
        }

        /* A closed socket looks like "no packet arrived" to the MQTT library,
         * not like an error, so check the socket state directly. */
        if (getSn_SR(SOCKET_MQTT) != SOCK_ESTABLISHED)
        {
            printf(" Connection lost\r\n");

            mqtt_connected = 0;
            mqtt_session_close();
            continue;
        }

        /* Yield — process incoming messages and keep-alive */
        if ((retval = MQTTYield(&g_mqtt_client, MQTT_YIELD_TIMEOUT)) < 0)
        {
            printf(" Yield error : %ld\r\n", retval);

            mqtt_connected = 0;
            mqtt_session_close();
            continue;
        }

        end_ms = HAL_GetTick();

        /* Periodic publish. The subtraction stays correct across the 32-bit
         * HAL_GetTick() wrap-around at ~49.7 days. */
        if ((end_ms - start_ms) >= MQTT_PUBLISH_PERIOD)
        {
            retval = MQTTPublish(&g_mqtt_client, MQTT_PUBLISH_TOPIC, &g_mqtt_message);

            if (retval < 0)
            {
                printf(" Publish failed : %ld\r\n", retval);

                mqtt_connected = 0;
                mqtt_session_close();
                continue;
            }

            printf(" Published\r\n");

            start_ms = end_ms;
        }
    }
}

#endif /* EXAMPLE_MQTT */
