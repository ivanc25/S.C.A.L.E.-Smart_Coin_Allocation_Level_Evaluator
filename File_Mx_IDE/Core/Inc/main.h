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
#include "stm32wb0x_hal.h"
#include "app_entry.h"
#include "app_common.h"
#include "app_debug.h"
#include "compiler.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void MX_USART1_UART_Init(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HX711_DATA2_Pin GPIO_PIN_3
#define HX711_DATA2_GPIO_Port GPIOB
#define HX711_DATA3_Pin GPIO_PIN_8
#define HX711_DATA3_GPIO_Port GPIOA
#define HX711_DATA4_Pin GPIO_PIN_11
#define HX711_DATA4_GPIO_Port GPIOA
#define HX711_DATA5_Pin GPIO_PIN_15
#define HX711_DATA5_GPIO_Port GPIOB
#define HX711_SCK_Pin GPIO_PIN_7
#define HX711_SCK_GPIO_Port GPIOB
#define HX711_DATA1_Pin GPIO_PIN_6
#define HX711_DATA1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* Nessuna uscita per MOSFET/motore: il progetto finale controlla solo il servo tramite soglia BLE. */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
