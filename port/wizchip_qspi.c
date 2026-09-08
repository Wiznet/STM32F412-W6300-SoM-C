/**
 * wizchip_qspi.c
 *
 * STM32F412 + W6300 QSPI port for ioLibrary_Driver
 * Hybrid mode: short transfers use polling, long transfers use DMA.
 */

#include <stdio.h>
#include "wizchip_qspi.h"

/* ============================================================ */
/* DMA threshold                                                 */
/* Transfers shorter than this length use polling for efficiency */
/* (DMA setup overhead exceeds gain for tiny transfers).         */
/* Try 4, 8, 16, 32, 64 to find the sweet spot.                  */
/* ============================================================ */
#define QSPI_DMA_THRESHOLD     16

/* ============================================================ */
/* How long wizchip_initialize() waits for the PHY link          */
/* ============================================================ */
#define PHY_LINK_TIMEOUT_MS    5000

/* ============================================================ */
/* DMA completion flags                                          */
/* Set by HAL callbacks, cleared before each transfer.           */
/* ============================================================ */
static volatile uint8_t g_qspi_tx_done = 0;
static volatile uint8_t g_qspi_rx_done = 0;

/* Override HAL weak callbacks to set completion flags */
void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi_p)
{
    g_qspi_tx_done = 1;
}

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi_p)
{
    g_qspi_rx_done = 1;
}

/* ============================================================ */
/* CS callbacks (W6300 NCS handled by QSPI HW automatically)     */
/* ============================================================ */
static inline void wizchip_select(void)   { }
static inline void wizchip_deselect(void) { }

/* ============================================================ */
/* Build QSPI command structure for W6300                        */
/* ============================================================ */
static void w6300_qspi_make_cmd(QSPI_CommandTypeDef *cmd,
                                uint8_t opcode, uint32_t addr,
                                uint16_t len, uint8_t is_read)
{
#if   (_WIZCHIP_QSPI_MODE_ == QSPI_SINGLE_MODE)
    cmd->AddressMode = QSPI_ADDRESS_1_LINE;
    cmd->DataMode    = QSPI_DATA_1_LINE;
#elif (_WIZCHIP_QSPI_MODE_ == QSPI_DUAL_MODE)
    cmd->AddressMode = QSPI_ADDRESS_2_LINES;
    cmd->DataMode    = QSPI_DATA_2_LINES;
#elif (_WIZCHIP_QSPI_MODE_ == QSPI_QUAD_MODE)
    cmd->AddressMode = QSPI_ADDRESS_4_LINES;
    cmd->DataMode    = QSPI_DATA_4_LINES;
#endif

    /* Instruction phase: 1-line, 1 byte opcode */
    cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd->Instruction     = opcode;

    /* Address phase: 16-bit */
    cmd->AddressSize = QSPI_ADDRESS_16_BITS;
    cmd->Address     = addr & 0xFFFF;

    /* Data phase */
    cmd->NbData = len;

    /* Dummy / Alternate bytes:
     *   Read  -> DummyCycles (mode-dependent)
     *   Write -> AlternateBytes (avoids Dual/Quad sampling issue)
     */
    if (is_read)
    {
#if   (_WIZCHIP_QSPI_MODE_ == QSPI_SINGLE_MODE)
        cmd->DummyCycles = 8;
#elif (_WIZCHIP_QSPI_MODE_ == QSPI_DUAL_MODE)
        cmd->DummyCycles = 4;
#elif (_WIZCHIP_QSPI_MODE_ == QSPI_QUAD_MODE)
        cmd->DummyCycles = 2;
#endif
        cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    }
    else
    {
        cmd->DummyCycles = 0;
#if   (_WIZCHIP_QSPI_MODE_ == QSPI_SINGLE_MODE)
        cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_1_LINE;
#elif (_WIZCHIP_QSPI_MODE_ == QSPI_DUAL_MODE)
        cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;
#elif (_WIZCHIP_QSPI_MODE_ == QSPI_QUAD_MODE)
        cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
#endif
        cmd->AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
        cmd->AlternateBytes     = 0x00;
    }

    /* Misc */
    cmd->DdrMode          = QSPI_DDR_MODE_DISABLE;
    cmd->DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd->SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;
}

/* ============================================================ */
/* W6300 QSPI Write                                              */
/* ============================================================ */
void W6300_QspiWriteByte(uint8_t opcode, uint16_t addr, uint8_t *pbuf, uint16_t len)
{
    QSPI_CommandTypeDef sCommand = {0};
    w6300_qspi_make_cmd(&sCommand, opcode, addr, len, 0);

    if (HAL_QSPI_Command(&hqspi, &sCommand, 1000) != HAL_OK)
        return;

    if (len >= QSPI_DMA_THRESHOLD)
    {
        /* DMA transfer */
        g_qspi_tx_done = 0;
        if (HAL_QSPI_Transmit_DMA(&hqspi, pbuf) != HAL_OK)
            return;

        /* Wait for DMA completion */
        while (!g_qspi_tx_done)
            ;
    }
    else
    {
        /* Polling transfer */
        HAL_QSPI_Transmit(&hqspi, pbuf, 1000);
    }

    while (__HAL_QSPI_GET_FLAG(&hqspi, QSPI_FLAG_BUSY))
        ;
}

/* ============================================================ */
/* W6300 QSPI Read                                               */
/* ============================================================ */
void W6300_QspiReadByte(uint8_t opcode, uint16_t addr, uint8_t *pbuf, uint16_t len)
{
    QSPI_CommandTypeDef sCommand = {0};
    w6300_qspi_make_cmd(&sCommand, opcode, addr, len, 1);

    if (HAL_QSPI_Command(&hqspi, &sCommand, 1000) != HAL_OK)
        return;

    if (len >= QSPI_DMA_THRESHOLD)
    {
        /* DMA transfer */
        g_qspi_rx_done = 0;
        if (HAL_QSPI_Receive_DMA(&hqspi, pbuf) != HAL_OK)
            return;

        while (!g_qspi_rx_done)
            ;
    }
    else
    {
        /* Polling transfer */
        HAL_QSPI_Receive(&hqspi, pbuf, 1000);
    }

    while (__HAL_QSPI_GET_FLAG(&hqspi, QSPI_FLAG_BUSY))
        ;
}

/* ============================================================ */
/* W6300 Reset                                                   */
/* ============================================================ */
void wizchip_reset(void)
{
    HAL_GPIO_WritePin(W6300_RSTn_GPIO_Port, W6300_RSTn_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);

    HAL_GPIO_WritePin(W6300_RSTn_GPIO_Port, W6300_RSTn_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
}

/* ============================================================ */
/* WIZchip Initialization                                        */
/* ============================================================ */
void wizchip_initialize(void)
{
#if   (_WIZCHIP_QSPI_MODE_ == QSPI_SINGLE_MODE)
    printf("QSPI Mode: SINGLE\r\n");
#elif (_WIZCHIP_QSPI_MODE_ == QSPI_DUAL_MODE)
    printf("QSPI Mode: DUAL\r\n");
#elif (_WIZCHIP_QSPI_MODE_ == QSPI_QUAD_MODE)
    printf("QSPI Mode: QUAD\r\n");
#endif
    printf("QSPI DMA threshold: %d bytes\r\n", QSPI_DMA_THRESHOLD);

    reg_wizchip_qspi_cbfunc(W6300_QspiReadByte, W6300_QspiWriteByte);
//    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);

    /* All sockets 2KB TX / 2KB RX (proven-stable). Bumping a single socket's
     * RX to 16KB reliably stalled iperf on this W6300 driver, so keep 2KB. */
#ifdef EXAMPLE_IPERF
    /* Throughput build: socket 0 = DHCP, socket 1 = iperf data, rest unused.
     * Socket 1 gets a large RX buffer so recv() can pull a big chunk per cycle;
     * keep IPERF_BUF_MAX_SIZE (app_main.c) <= this buffer.
     *
     * Only enlarge this together with SF_TCP_NODELAY on the socket (see
     * examples/iperf/app_main.c) - without it an RX buffer above one MSS makes
     * the W6300 fall back to its delayed-ACK timer and throughput collapses.
     *
     * The datasheet contradicts itself on the total: DS 4.2.27 (p.72) says
     * total RX > 16KB "causes a malfunction", while DS sect.2 p.18, Figure 3
     * and the 4KB x 8 reset default say 32KB. 18KB works here; if in doubt drop
     * socket 1 to 8KB, still far above one MSS. */
    uint8_t memsize[2][8] = {
        {2,  2, 0, 0, 0, 0, 0, 0},   /* TX: total  4KB */
        {2, 16, 0, 0, 0, 0, 0, 0}    /* RX: total 18KB (socket 1 = 16KB) */
    };
#else
    uint8_t memsize[2][8] = {
        {2, 2, 2, 2, 2, 2, 2, 2},   /* TX */
        {2, 2, 2, 2, 2, 2, 2, 2}    /* RX */
    };
#endif

    if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
    {
        printf(" W6x00 initialized fail\r\n");
        return;
    }
    printf("[3] CW_INIT_WIZCHIP done\r\n");

    uint8_t temp;
    uint32_t start_ms = HAL_GetTick();

    do {
        if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1)
        {
            printf(" Unknown PHY link status\r\n");
            return;
        }

        /* Subtraction stays correct across the 32-bit HAL_GetTick() wrap at
         * ~49.7 days; HAL_GetTick() + timeout does not. */
        if ((HAL_GetTick() - start_ms) >= PHY_LINK_TIMEOUT_MS)
        {
            printf(" PHY Link timeout (no cable?)\r\n");
            break;
        }
    } while (temp == PHY_LINK_OFF);

    printf("[4] CW_GET_PHYLINK done\r\n");

    if (temp == PHY_LINK_OFF)
        printf(" W6300 PHY Link DOWN\r\n");
    else
        printf(" W6300 PHY Link UP\r\n");
}

/* ============================================================ */
/* Network                                                       */
/* ============================================================ */
void network_initialize(wiz_NetInfo net_info)
{
    ctlnetwork(CN_SET_NETINFO, (void *)&net_info);
}

void print_network_information(wiz_NetInfo net_info)
{
    uint8_t tmp_str[8] = {0,};

    ctlnetwork(CN_GET_NETINFO, (void *)&net_info);
    ctlwizchip(CW_GET_ID, (void *)tmp_str);

#if _WIZCHIP_ <= W5500
    if (net_info.dhcp == NETINFO_DHCP) {
        printf("==========================================================\r\n");
        printf(" %s network configuration : DHCP\r\n\r\n", (char *)tmp_str);
    } else {
        printf("==========================================================\r\n");
        printf(" %s network configuration : static\r\n\r\n", (char *)tmp_str);
    }
    printf(" MAC         : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           net_info.mac[0], net_info.mac[1], net_info.mac[2],
           net_info.mac[3], net_info.mac[4], net_info.mac[5]);
    printf(" IP          : %d.%d.%d.%d\r\n",
           net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
    printf(" Subnet Mask : %d.%d.%d.%d\r\n",
           net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3]);
    printf(" Gateway     : %d.%d.%d.%d\r\n",
           net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3]);
    printf(" DNS         : %d.%d.%d.%d\r\n",
           net_info.dns[0], net_info.dns[1], net_info.dns[2], net_info.dns[3]);
    printf("==========================================================\r\n\r\n");
#else
    printf("==========================================================\r\n");
    printf(" %s network configuration\r\n\r\n", (char *)tmp_str);
    printf(" MAC         : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           net_info.mac[0], net_info.mac[1], net_info.mac[2],
           net_info.mac[3], net_info.mac[4], net_info.mac[5]);
    printf(" IP          : %d.%d.%d.%d\r\n",
           net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
    printf(" Subnet Mask : %d.%d.%d.%d\r\n",
           net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3]);
    printf(" Gateway     : %d.%d.%d.%d\r\n",
           net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3]);
    printf(" DNS         : %d.%d.%d.%d\r\n",
           net_info.dns[0], net_info.dns[1], net_info.dns[2], net_info.dns[3]);
    print_ipv6_addr((uint8_t *)" GW6 ", net_info.gw6);
    print_ipv6_addr((uint8_t *)" LLA ", net_info.lla);
    print_ipv6_addr((uint8_t *)" GUA ", net_info.gua);
    print_ipv6_addr((uint8_t *)" SUB6", net_info.sn6);
    print_ipv6_addr((uint8_t *)" DNS6", net_info.dns6);
    printf("==========================================================\r\n\r\n");
#endif
}

void print_ipv6_addr(uint8_t *name, uint8_t *ip6addr)
{
    printf("%s        : ", name);
    printf("%04X:%04X",   ((uint16_t)ip6addr[0]  << 8) | ip6addr[1],
                          ((uint16_t)ip6addr[2]  << 8) | ip6addr[3]);
    printf(":%04X:%04X",  ((uint16_t)ip6addr[4]  << 8) | ip6addr[5],
                          ((uint16_t)ip6addr[6]  << 8) | ip6addr[7]);
    printf(":%04X:%04X",  ((uint16_t)ip6addr[8]  << 8) | ip6addr[9],
                          ((uint16_t)ip6addr[10] << 8) | ip6addr[11]);
    printf(":%04X:%04X\r\n", ((uint16_t)ip6addr[12] << 8) | ip6addr[13],
                             ((uint16_t)ip6addr[14] << 8) | ip6addr[15]);
}
