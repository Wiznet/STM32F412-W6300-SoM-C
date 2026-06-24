/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define W6300_RSTn_Pin GPIO_PIN_0
#define W6300_RSTn_GPIO_Port GPIOC
#define W6300_INTn_Pin GPIO_PIN_1
#define W6300_INTn_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* ============================================================ */
/* Select ONE example to build                                   */
/* ============================================================ */
//#define EXAMPLE_DHCP_DNS
//#define EXAMPLE_LOOPBACK
//#define EXAMPLE_UDP
//#define EXAMPLE_UDP_MULTICAST
//#define EXAMPLE_TCP_SERVER_MULTI_SOCKET
//#define EXAMPLE_SNTP
//#define EXAMPLE_NETBIOS
//#define EXAMPLE_HTTP_SERVER
//#define EXAMPLE_MQTT
//#define EXAMPLE_TFTP
//#define EXAMPLE_PPPOE
//#define EXAMPLE_NETWORK_INSTALL
//#define EXAMPLE_UPNP
#define EXAMPLE_TCP_CLIENT_OVER_SSL
//#define EXAMPLE_TCP_SERVER_OVER_SSL

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
