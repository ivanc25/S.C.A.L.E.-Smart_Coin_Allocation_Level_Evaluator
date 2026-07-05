/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    PiggyBank_app.c
  * @author  MCD Application Team
  * @brief   PiggyBank_app application definition.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_common.h"
#include "app_ble.h"
#include "ble.h"
#include "piggybank_app.h"
#include "piggybank.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

typedef enum
{
  Coins_NOTIFICATION_OFF,
  Coins_NOTIFICATION_ON,
  /* USER CODE BEGIN Service1_APP_SendInformation_t */

  /* USER CODE END Service1_APP_SendInformation_t */
  PIGGYBANK_APP_SENDINFORMATION_LAST
} PIGGYBANK_APP_SendInformation_t;

typedef struct
{
  PIGGYBANK_APP_SendInformation_t     Coins_Notification_Status;
  /* USER CODE BEGIN Service1_APP_Context_t */
  uint32_t LimitValue;
  /* USER CODE END Service1_APP_Context_t */
  uint16_t              ConnectionHandle;
} PIGGYBANK_APP_Context_t;

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PIGGYBANK_COINS_PAYLOAD_LEN 5U  /* Un byte per ciascun taglio: 2€, 1€, 50c, 20c, 10c. */
/* USER CODE END PD */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static PIGGYBANK_APP_Context_t PIGGYBANK_APP_Context;

uint8_t a_PIGGYBANK_UpdateCharData[247];

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void PIGGYBANK_Coins_SendNotification(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void PIGGYBANK_Notification(PIGGYBANK_NotificationEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service1_Notification_1 */

  /* USER CODE END Service1_Notification_1 */
  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service1_Notification_Service1_EvtOpcode */

    /* USER CODE END Service1_Notification_Service1_EvtOpcode */

    case PIGGYBANK_COINS_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char1_NOTIFY_ENABLED_EVT */
      PIGGYBANK_APP_Context.Coins_Notification_Status = Coins_NOTIFICATION_ON;
      /* USER CODE END Service1Char1_NOTIFY_ENABLED_EVT */
      break;

    case PIGGYBANK_COINS_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char1_NOTIFY_DISABLED_EVT */
      PIGGYBANK_APP_Context.Coins_Notification_Status = Coins_NOTIFICATION_OFF;
      /* USER CODE END Service1Char1_NOTIFY_DISABLED_EVT */
      break;

    case PIGGYBANK_LIMIT_READ_EVT:
      /* USER CODE BEGIN Service1Char2_READ_EVT */

      /* USER CODE END Service1Char2_READ_EVT */
      break;

    case PIGGYBANK_LIMIT_WRITE_EVT:
      /* USER CODE BEGIN Service1Char2_WRITE_EVT */
      /* LIMIT è un uint32 little-endian espresso in centesimi di euro. */
      if (p_Notification->DataTransfered.Length >= 4U)
      {
        PIGGYBANK_APP_Context.LimitValue = ((uint32_t)p_Notification->DataTransfered.p_Payload[0]) |
                                          ((uint32_t)p_Notification->DataTransfered.p_Payload[1] << 8) |
                                          ((uint32_t)p_Notification->DataTransfered.p_Payload[2] << 16) |
                                          ((uint32_t)p_Notification->DataTransfered.p_Payload[3] << 24);
        APP_DBG_MSG("BLE LIMIT write: %lu cents\r\n", PIGGYBANK_APP_Context.LimitValue);
      }
      /* USER CODE END Service1Char2_WRITE_EVT */
      break;

    default:
      /* USER CODE BEGIN Service1_Notification_default */

      /* USER CODE END Service1_Notification_default */
      break;
  }
  /* USER CODE BEGIN Service1_Notification_2 */

  /* USER CODE END Service1_Notification_2 */
  return;
}

void PIGGYBANK_APP_EvtRx(PIGGYBANK_APP_ConnHandleNotEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service1_APP_EvtRx_1 */

  /* USER CODE END Service1_APP_EvtRx_1 */

  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service1_APP_EvtRx_Service1_EvtOpcode */

    /* USER CODE END Service1_APP_EvtRx_Service1_EvtOpcode */
    case PIGGYBANK_CONN_HANDLE_EVT :
      PIGGYBANK_APP_Context.ConnectionHandle = p_Notification->ConnectionHandle;
      /* USER CODE BEGIN Service1_APP_CENTR_CONN_HANDLE_EVT */
      APP_DBG_MSG("PiggyBank connected, handle=0x%04X\r\n", PIGGYBANK_APP_Context.ConnectionHandle);
      /* USER CODE END Service1_APP_CENTR_CONN_HANDLE_EVT */
      break;
    case PIGGYBANK_DISCON_HANDLE_EVT :
      PIGGYBANK_APP_Context.ConnectionHandle = 0xFFFF;
      /* USER CODE BEGIN Service1_APP_DISCON_HANDLE_EVT */
      APP_DBG_MSG("PiggyBank disconnected\r\n");
      /* USER CODE END Service1_APP_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN Service1_APP_EvtRx_default */

      /* USER CODE END Service1_APP_EvtRx_default */
      break;
  }

  /* USER CODE BEGIN Service1_APP_EvtRx_2 */

  /* USER CODE END Service1_APP_EvtRx_2 */

  return;
}

void PIGGYBANK_APP_Init(void)
{
  PIGGYBANK_APP_Context.ConnectionHandle = 0xFFFF;
  PIGGYBANK_Init();

  /* USER CODE BEGIN Service1_APP_Init */
  PIGGYBANK_APP_Context.Coins_Notification_Status = Coins_NOTIFICATION_OFF;
  PIGGYBANK_APP_Context.LimitValue = 0U;
  /* USER CODE END Service1_APP_Init */
  return;
}

/* USER CODE BEGIN FD */
void PIGGYBANK_APP_SendCoins(const uint8_t coins[5])
{
  /* Copia il payload applicativo nel buffer BLE e invia la notifica se abilitata. */
  if (coins == NULL)
  {
    return;
  }

  memcpy(a_PIGGYBANK_UpdateCharData, coins, PIGGYBANK_COINS_PAYLOAD_LEN);
  PIGGYBANK_Coins_SendNotification();
}

uint32_t PIGGYBANK_APP_GetLimitValue(void)
{
  /* Valore soglia usato dal main per decidere la chiusura del servo. */
  return PIGGYBANK_APP_Context.LimitValue;
}
/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
__USED void PIGGYBANK_Coins_SendNotification(void) /* Property Notification */
{
  PIGGYBANK_APP_SendInformation_t notification_on_off = Coins_NOTIFICATION_OFF;
  PIGGYBANK_Data_t piggybank_notification_data;

  piggybank_notification_data.p_Payload = (uint8_t*)a_PIGGYBANK_UpdateCharData;
  piggybank_notification_data.Length = 0;

  /* USER CODE BEGIN Service1Char1_NS_1*/
  notification_on_off = PIGGYBANK_APP_Context.Coins_Notification_Status;
  piggybank_notification_data.Length = PIGGYBANK_COINS_PAYLOAD_LEN;
  /* USER CODE END Service1Char1_NS_1*/

  if (notification_on_off != Coins_NOTIFICATION_OFF && PIGGYBANK_APP_Context.ConnectionHandle != 0xFFFF)
  {
    PIGGYBANK_NotifyValue(PIGGYBANK_COINS, &piggybank_notification_data, PIGGYBANK_APP_Context.ConnectionHandle);
  }

  /* USER CODE BEGIN Service1Char1_NS_Last*/

  /* USER CODE END Service1Char1_NS_Last*/

  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/

/* USER CODE END FD_LOCAL_FUNCTIONS*/
