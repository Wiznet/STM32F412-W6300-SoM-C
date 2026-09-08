/**
 * @file    app_main.c
 * @brief   MQTTS (MQTT over TLS) example for STM32F412 + W6300 SoM
 *
 * @details Connects to AWS IoT Core over TLS 1.2 on port 8883, subscribes to
 *          a topic and periodically publishes a message.
 *
 *          Same MQTT client as the plain mqtt example -- the Paho embedded
 *          client bundled with ioLibrary_Driver. Only its transport changes:
 *          mqtts_transport.c drives mbedTLS instead of raw sockets.
 *
 *          The board authenticates with the ATECC608C-TNGTLS device
 *          certificate. The private key stays in slot 0 of the secure
 *          element, so no key material is stored in flash.
 *
 *          Based on the WIZnet-PICO-AWS-C aws_iot_mqtt example, reworked for
 *          the Paho client and the on-board secure element.
 *
 * @note    SysTick must call app_timer_tick() every 1 ms.
 */

#include "main.h"

#ifdef EXAMPLE_MQTTS

#include <stdio.h>
#include <string.h>

#include "wizchip_conf.h"
#include "wizchip_qspi.h"
#include "socket.h"
#include "dhcp.h"
#include "dns.h"

#include "MQTTClient.h"
#include "mqtts_transport.h"
#include "atecc608.h"

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
#define DNS_RETRY_COUNT        5

/* Socket allocation. Socket 1 is fixed: TLS_SOCKET_NUM in port/wizchip_tls.h
 * is what the mbedTLS BIO callbacks read and write. */
#define SOCKET_DHCP            0
#define SOCKET_MQTT            1
#define SOCKET_DNS             2

/* ============================================================ */
/* AWS IoT Core                                                  */
/* ============================================================ */
/* Your account endpoint: AWS IoT console -> Settings -> Device data endpoint,
 * or `aws iot describe-endpoint --endpoint-type iot:Data-ATS`. */
#define MQTT_BROKER_DOMAIN     "account-specific-prefix-ats.iot.ap-northeast-2.amazonaws.com"
#define PORT_MQTTS             8883

/* With the ATECC608C-TNGTLS the client ID must match the thing name you
 * registered for this device certificate. */
#define MQTT_CLIENT_ID         "w6300-som"

/* AWS IoT authenticates with the client certificate, never a password. */
#define MQTT_PUBLISH_TOPIC     "$aws/things/w6300-som/shadow/update"
#define MQTT_SUBSCRIBE_TOPIC   "$aws/things/w6300-som/shadow/update/accepted"
#define MQTT_PUBLISH_PAYLOAD   "{\"state\":{\"reported\":{\"hello\":\"w6300\"}}}"

#define MQTT_PUBLISH_PERIOD    (1000 * 10)  /* 10 seconds */
#define MQTT_KEEP_ALIVE        60           /* seconds */
#define DEFAULT_TIMEOUT        (1000 * 5)   /* MQTT command timeout, ms */

/* MQTTYield() takes a timeout in milliseconds, not the keep-alive interval. */
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

static uint8_t g_broker_ip[4] = {0};

/* ============================================================ */
/* Timers                                                        */
/* ============================================================ */
static volatile uint16_t g_msec_cnt = 0;
static volatile uint8_t g_dhcp_tick = 0;

/* ============================================================ */
/* DHCP                                                          */
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

        DNS_time_handler();

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
    mqtts_transport_close(&g_mqtt_network);
}

static int32_t mqtt_session_open(void)
{
    int32_t retval;

    if (mqtts_transport_connect(&g_mqtt_network, g_broker_ip, PORT_MQTTS,
                                MQTT_BROKER_DOMAIN) != 0)
        return -1;

    MQTTClientInit(&g_mqtt_client, &g_mqtt_network, DEFAULT_TIMEOUT,
                   g_mqtt_send_buf, ETHERNET_BUF_MAX_SIZE,
                   g_mqtt_recv_buf, ETHERNET_BUF_MAX_SIZE);

    g_mqtt_packet_connect_data.MQTTVersion = 4;   /* AWS IoT speaks MQTT 3.1.1 */
    g_mqtt_packet_connect_data.cleansession = 1;
    g_mqtt_packet_connect_data.willFlag = 0;
    g_mqtt_packet_connect_data.keepAliveInterval = MQTT_KEEP_ALIVE;
    g_mqtt_packet_connect_data.clientID.cstring = MQTT_CLIENT_ID;
    /* No username/password: the client certificate is the credential. */
    g_mqtt_packet_connect_data.username.cstring = NULL;
    g_mqtt_packet_connect_data.password.cstring = NULL;

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

    printf(" MQTT connected over TLS\r\n");
    printf(" Subscribed to '%s'\r\n", MQTT_SUBSCRIBE_TOPIC);
    printf(" Publishing to '%s' every %d seconds\r\n\r\n",
           MQTT_PUBLISH_TOPIC, MQTT_PUBLISH_PERIOD / 1000);

    return 0;
}

/* ============================================================ */
/* Resolve the broker endpoint                                   */
/* ============================================================ */
static int32_t resolve_broker(void)
{
    uint8_t dns_retry = 0;

    DNS_init(SOCKET_DNS, g_ethernet_buf);

    printf(" Resolving %s\r\n", MQTT_BROKER_DOMAIN);

    while (dns_retry <= DNS_RETRY_COUNT)
    {
        if (DNS_run(g_net_info.dns, (uint8_t *)MQTT_BROKER_DOMAIN, g_broker_ip) > 0)
        {
            printf(" Broker IP : %d.%d.%d.%d\r\n\r\n",
                   g_broker_ip[0], g_broker_ip[1],
                   g_broker_ip[2], g_broker_ip[3]);
            return 0;
        }

        dns_retry++;
        printf(" DNS timeout, retry %d\r\n", dns_retry);
        HAL_Delay(1000);
    }

    printf(" DNS failed\r\n");
    return -1;
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

    printf("\r\n========================================\r\n");
    printf(" MQTTS Example (AWS IoT Core)\r\n");
    printf(" STM32F412 + W6300 + ATECC608C-TNGTLS\r\n");
    printf("========================================\r\n\r\n");

    /* ---- Secure element ---- */
    if (atecc608_init() != ATCA_SUCCESS)
    {
        printf("[ATECC608] Init failed\r\n");
        while (1)
            ;
    }
    printf("[ATECC608] Init OK\r\n");
    atecc608_print_serial_number();

    /* ---- W6300 HW init ---- */
    wizchip_reset();
    wizchip_initialize();

    /* ---- Network init ---- */
    if (g_net_info.dhcp == NETINFO_DHCP)
    {
        printf(" DHCP client running\r\n");
        DHCP_init(SOCKET_DHCP, g_ethernet_buf);
        reg_dhcp_cbfunc(cb_dhcp_assign, cb_dhcp_assign, cb_dhcp_conflict);

        /* Wait for the first lease */
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

        dhcp_retry = 0;
    }
    else
    {
        network_initialize(g_net_info);
        print_network_information(g_net_info);
    }

    /* ---- Resolve the AWS endpoint ---- */
    if (resolve_broker() != 0)
    {
        while (1)
            ;
    }

    /* ---- TLS setup ---- */
    if (mqtts_transport_init() != 0)
    {
        printf(" TLS init failed\r\n");
        while (1)
            ;
    }

    /* ---- Configure publish message ---- */
    g_mqtt_message.qos = QOS0;
    g_mqtt_message.retained = 0;
    g_mqtt_message.dup = 0;
    g_mqtt_message.payload = MQTT_PUBLISH_PAYLOAD;
    g_mqtt_message.payloadlen = strlen(g_mqtt_message.payload);

    /* ---- Main loop ---- */
    while (1)
    {
        /* ---- DHCP renewal ---- */
        if (g_net_info.dhcp == NETINFO_DHCP && g_dhcp_tick)
        {
            g_dhcp_tick = 0;
            retval = DHCP_run();

            if (retval == DHCP_IP_CHANGED)
            {
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

#endif /* EXAMPLE_MQTTS */
