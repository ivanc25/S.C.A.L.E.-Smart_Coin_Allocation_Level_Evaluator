/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    PiggyBank_app.h
  * @author  MCD Application Team
  * @brief   Header for PiggyBank_app.c
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
#ifndef PIGGYBANK_APP_H
#define PIGGYBANK_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  PIGGYBANK_CONN_HANDLE_EVT,
  PIGGYBANK_DISCON_HANDLE_EVT,

  /* USER CODE BEGIN Service1_OpcodeNotificationEvt_t */

  /* USER CODE END Service1_OpcodeNotificationEvt_t */

  PIGGYBANK_LAST_EVT,
} PIGGYBANK_APP_OpcodeNotificationEvt_t;

typedef struct
{
  PIGGYBANK_APP_OpcodeNotificationEvt_t          EvtOpcode;
  uint16_t                                 ConnectionHandle;

  /* USER CODE BEGIN PIGGYBANK_APP_ConnHandleNotEvt_t */

  /* USER CODE END PIGGYBANK_APP_ConnHandleNotEvt_t */
} PIGGYBANK_APP_ConnHandleNotEvt_t;
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions ------------------------------------------------------- */
void PIGGYBANK_APP_Init(void);
void PIGGYBANK_APP_EvtRx(PIGGYBANK_APP_ConnHandleNotEvt_t *p_Notification);
/* USER CODE BEGIN EF */
/* Aggiorna e notifica il conteggio monete nel formato: 2€, 1€, 50c, 20c, 10c. */
void PIGGYBANK_APP_SendCoins(const uint8_t coins[5]);

/* Restituisce il limite BLE in centesimi di euro. */
uint32_t PIGGYBANK_APP_GetLimitValue(void);
/* USER CODE END EF */

#ifdef __cplusplus
}
#endif

#endif /*PIGGYBANK_APP_H */
