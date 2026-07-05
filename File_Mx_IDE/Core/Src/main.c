/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hx711.h"
#include <stdio.h>
#include <string.h>
#include "servo_tg9e.h"
#include "piggybank_app.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define HX711_TARE_SAMPLES          20U
#define HX711_AVERAGE_SAMPLES       5U
#define APP_STATUS_PERIOD_MS        500U
#define APP_BLE_NOTIFY_PERIOD_MS    500U
#define APP_UART_STATUS_ENABLE      1U
#define SERVO_OPEN_ANGLE            0U
#define SERVO_CLOSED_ANGLE          90U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

PKA_HandleTypeDef hpka;

RNG_HandleTypeDef hrng;

TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
char messaggio_uart[180];
float pesi_reali[5] = {0.0f};
int monete_contate[5] = {0};

/* Peso nominale di una moneta per ogni canale fisico HX711.
   Ordine dei canali cablati: [0]=2€, [1]=50c, [2]=1€, [3]=20c, [4]=10c.
   I canali 50c e 1€ sono quindi mantenuti separati qui e riordinati solo
   nel pacchetto BLE, in modo da rispettare l'ordine mostrato nell'app. */
const float PESI_MONETE[5] = {8.50f, 7.80f, 7.50f, 5.74f, 4.10f};

/* Stato logico del servo: 0 = aperto, 1 = chiuso.
   La chiusura avviene esclusivamente quando il totale raggiunge il limite BLE. */
volatile uint8_t g_servo_closed_by_limit = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);
static void MX_TIM17_Init(void);
static void MX_RNG_Init(void);
static void MX_RADIO_Init(void);
static void MX_RADIO_TIMER_Init(void);
static void MX_PKA_Init(void);
/* USER CODE BEGIN PFP */
static void App_UpdateCoinCounts(void);
static uint32_t App_GetTotalCents(void);
static void App_SendBleCoins(void);
static void App_HandleUartDebug(void);
static void App_UpdateServoFromLimit(void);
static void App_PrintStatusUart(void);
static void App_PrintHelp(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Converte i pesi misurati in numero di monete.
   La piccola banda morta attorno allo zero riduce oscillazioni dovute al rumore. */
static void App_UpdateCoinCounts(void)
{
  for (uint8_t i = 0U; i < 5U; i++)
  {
    if ((pesi_reali[i] > -2.0f) && (pesi_reali[i] < 2.0f))
    {
      pesi_reali[i] = 0.0f;
    }

    if (pesi_reali[i] <= 0.0f)
    {
      monete_contate[i] = 0;
    }
    else
    {
      monete_contate[i] = (int)((pesi_reali[i] / PESI_MONETE[i]) + 0.5f);
    }
  }
}

/* Calcola il valore totale usando l'ordine fisico dei cinque canali HX711. */
static uint32_t App_GetTotalCents(void)
{
  uint32_t total = 0U;

  if (monete_contate[0] > 0) { total += (uint32_t)monete_contate[0] * 200U; } /* channel 0 = 2€ */
  if (monete_contate[1] > 0) { total += (uint32_t)monete_contate[1] *  50U; } /* channel 1 = 50c */
  if (monete_contate[2] > 0) { total += (uint32_t)monete_contate[2] * 100U; } /* channel 2 = 1€ */
  if (monete_contate[3] > 0) { total += (uint32_t)monete_contate[3] *  20U; } /* channel 3 = 20c */
  if (monete_contate[4] > 0) { total += (uint32_t)monete_contate[4] *  10U; } /* channel 4 = 10c */

  return total;
}

/* Le notifiche BLE inviano un byte per moneta: il conteggio viene saturato a 8 bit. */
static uint8_t App_ClampCoinCountToU8(int count)
{
  if (count < 0)
  {
    return 0U;
  }
  if (count > 255)
  {
    return 255U;
  }
  return (uint8_t)count;
}

static void App_SendBleCoins(void)
{
  /* Formato del pacchetto BLE COINS inviato all'app:
     byte 0 = 2€, byte 1 = 1€, byte 2 = 50c, byte 3 = 20c, byte 4 = 10c.
     Il riordino tra canale fisico e visualizzazione avviene solo in questo punto. */
  uint8_t coin_payload[5];

  coin_payload[0] = App_ClampCoinCountToU8(monete_contate[0]); /* 2€  */
  coin_payload[1] = App_ClampCoinCountToU8(monete_contate[2]); /* 1€  */
  coin_payload[2] = App_ClampCoinCountToU8(monete_contate[1]); /* 50c */
  coin_payload[3] = App_ClampCoinCountToU8(monete_contate[3]); /* 20c */
  coin_payload[4] = App_ClampCoinCountToU8(monete_contate[4]); /* 10c */

  PIGGYBANK_APP_SendCoins(coin_payload);
}

static void App_HandleUartDebug(void)
{
  uint8_t dato_ricevuto = 0U;

  /* Lettura UART non bloccante: la seriale è usata solo per richiesta di help/stato. */
  if (HAL_UART_Receive(&huart1, &dato_ricevuto, 1U, 0U) == HAL_OK)
  {
    if ((dato_ricevuto == '?') || (dato_ricevuto == 'h') || (dato_ricevuto == 'H'))
    {
      App_PrintHelp();
    }
  }
}

/* Aggiorna il servo solo quando cambia la condizione di soglia.
   Questo evita di riscrivere continuamente il PWM a ogni ciclo del main loop. */
static void App_UpdateServoFromLimit(void)
{
  static uint8_t last_closed_state = 0xFFU;
  uint32_t limit_cents = PIGGYBANK_APP_GetLimitValue();
  uint32_t total_cents = App_GetTotalCents();
  uint8_t should_close = ((limit_cents > 0U) && (total_cents >= limit_cents)) ? 1U : 0U;

  if (should_close != last_closed_state)
  {
    if (should_close != 0U)
    {
      ServoTG9e_SetAngle(SERVO_CLOSED_ANGLE);
      APP_DBG_MSG("Servo CLOSED: total=%lu cents limit=%lu cents\r\n", total_cents, limit_cents);
    }
    else
    {
      ServoTG9e_SetAngle(SERVO_OPEN_ANGLE);
      APP_DBG_MSG("Servo OPEN: total=%lu cents limit=%lu cents\r\n", total_cents, limit_cents);
    }

    g_servo_closed_by_limit = should_close;
    last_closed_state = should_close;
  }
}

static void App_PrintStatusUart(void)
{
#if (APP_UART_STATUS_ENABLE != 0U)
  uint32_t total_cents = App_GetTotalCents();
  uint32_t limit_cents = PIGGYBANK_APP_GetLimitValue();

  snprintf(messaggio_uart, sizeof(messaggio_uart),
           "2E:%d (%.1fg) | 50c:%d (%.1fg) | 1E:%d (%.1fg) | 20c:%d (%.1fg) | 10c:%d (%.1fg) | total:%lu.%02lu EUR | limit:%lu.%02lu EUR | servo:%s\r\n",
           monete_contate[0], pesi_reali[0],
           monete_contate[1], pesi_reali[1],
           monete_contate[2], pesi_reali[2],
           monete_contate[3], pesi_reali[3],
           monete_contate[4], pesi_reali[4],
           total_cents / 100U, total_cents % 100U,
           limit_cents / 100U, limit_cents % 100U,
           (g_servo_closed_by_limit != 0U) ? "CLOSED" : "OPEN");

  HAL_UART_Transmit(&huart1, (uint8_t*)messaggio_uart, strlen(messaggio_uart), 10U);
#endif
}

static void App_PrintHelp(void)
{
  const char help[] =
      "\r\nPiggyBank no-motor firmware\r\n"
      "  BLE COINS : notify 5 coin-count bytes\r\n"
      "  BLE LIMIT : write uint32 little-endian cents\r\n"
      "  Servo     : OPEN until total >= LIMIT, then CLOSED\r\n"
      "  Motor/MOSFET and UART servo control are disabled.\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)help, strlen(help), 100U);
}

int __io_putchar(int ch)
{
  uint8_t c = (uint8_t)ch;
  HAL_UART_Transmit(&huart1, &c, 1U, HAL_MAX_DELAY);
  return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM17_Init();
  MX_RNG_Init();
  MX_RADIO_Init();
  MX_RADIO_TIMER_Init();
  MX_PKA_Init();
  /* USER CODE BEGIN 2 */
  char test[] = "\r\n--- PIGGYBANK START: BLE + HX711 + SERVO LIMIT CONTROL ---\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)test, strlen(test), 50U);

  /* Fattori di calibrazione dei cinque canali HX711, ottenuti con pesi noti. */
  HX711_SetScales(1050.0f, 1044.5f, 1007.0f, 1073.9f, 1017.39f);
  HX711_TareAllBegin();

  /* Servo collegato a TIM17 CH1 / PA9.
     All'avvio resta aperto; la chiusura dipende soltanto dalla soglia BLE LIMIT. */
  ServoTG9e_Init(&htim17, TIM_CHANNEL_1);
  ServoTG9e_SetAngle(SERVO_OPEN_ANGLE);
  g_servo_closed_by_limit = 0U;

  App_PrintHelp();

  char menu[] = "BLE name: PiggyBank. Use the web app/nRF Connect, not the normal phone Bluetooth menu.\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)menu, strlen(menu), 50U);

  char ble_init_msg[] = "Starting BLE stack...\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)ble_init_msg, strlen(ble_init_msg), 50U);

  /* USER CODE END 2 */

  /* Init code for STM32_BLE */
  MX_APPE_Init(NULL);

  /* USER CODE BEGIN BLE_Init_Done */
  char ble_done_msg[] = "BLE init finished. Scan for BLE device: PiggyBank\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)ble_done_msg, strlen(ble_done_msg), 50U);
  /* USER CODE END BLE_Init_Done */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  uint8_t hx711_tare_done = 0U;
  uint32_t last_status_tick = 0U;
  uint32_t last_ble_notify_tick = 0U;

  while (1)
  {
    uint32_t now = HAL_GetTick();

    App_HandleUartDebug();

    /* La tara viene eseguita in modo cooperativo: il BLE continua a essere processato. */
    if (hx711_tare_done == 0U)
    {
      if (HX711_TareAllTask(HX711_TARE_SAMPLES) != 0U)
      {
        hx711_tare_done = 1U;
        char tare_msg[] = "HX711 tare completed\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t*)tare_msg, strlen(tare_msg), 50U);
      }
    }
    else
    {
      /* Ogni nuova media valida aggiorna conteggio monete, servo e notifica BLE. */
      if (HX711_GetWeightsAveragedTask(pesi_reali, HX711_AVERAGE_SAMPLES) != 0U)
      {
        App_UpdateCoinCounts();
        App_UpdateServoFromLimit();

        if ((now - last_ble_notify_tick) >= APP_BLE_NOTIFY_PERIOD_MS)
        {
          last_ble_notify_tick = now;
          App_SendBleCoins();
        }
      }

      /* Stampa periodica su UART utile durante collaudo e calibrazione. */
      if ((now - last_status_tick) >= APP_STATUS_PERIOD_MS)
      {
        last_status_tick = now;
        App_PrintStatusUart();
      }
    }
    /* USER CODE END WHILE */
    MX_APPE_Process();
    App_UpdateServoFromLimit();

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource and SYSCLKDivider
  */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_RC64MPLL;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_RC64MPLL_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_WAIT_STATES_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLK_DIV4;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief PKA Initialization Function
  * @param None
  * @retval None
  */
static void MX_PKA_Init(void)
{

  /* USER CODE BEGIN PKA_Init 0 */

  /* USER CODE END PKA_Init 0 */

  /* USER CODE BEGIN PKA_Init 1 */

  /* USER CODE END PKA_Init 1 */
  hpka.Instance = PKA;
  if (HAL_PKA_Init(&hpka) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN PKA_Init 2 */

  /* USER CODE END PKA_Init 2 */

}

/**
  * @brief RADIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_RADIO_Init(void)
{

  /* USER CODE BEGIN RADIO_Init 0 */

  /* USER CODE END RADIO_Init 0 */

  RADIO_HandleTypeDef hradio = {0};

  /* USER CODE BEGIN RADIO_Init 1 */

  /* USER CODE END RADIO_Init 1 */

  if (__HAL_RCC_RADIO_IS_CLK_DISABLED())
  {
    /* Radio Peripheral reset */
    __HAL_RCC_RADIO_FORCE_RESET();
    __HAL_RCC_RADIO_RELEASE_RESET();

    /* Enable Radio peripheral clock */
    __HAL_RCC_RADIO_CLK_ENABLE();
  }
  hradio.Instance = RADIO;
  HAL_RADIO_Init(&hradio);
  /* USER CODE BEGIN RADIO_Init 2 */

  /* USER CODE END RADIO_Init 2 */

}

/**
  * @brief RADIO_TIMER Initialization Function
  * @param None
  * @retval None
  */
static void MX_RADIO_TIMER_Init(void)
{

  /* USER CODE BEGIN RADIO_TIMER_Init 0 */

  /* USER CODE END RADIO_TIMER_Init 0 */

  RADIO_TIMER_InitTypeDef RADIO_TIMER_InitStruct = {0};

  /* USER CODE BEGIN RADIO_TIMER_Init 1 */

  /* USER CODE END RADIO_TIMER_Init 1 */

  if (__HAL_RCC_RADIO_IS_CLK_DISABLED())
  {
    /* Radio Peripheral reset */
    __HAL_RCC_RADIO_FORCE_RESET();
    __HAL_RCC_RADIO_RELEASE_RESET();

    /* Enable Radio peripheral clock */
    __HAL_RCC_RADIO_CLK_ENABLE();
  }
  /* Wait to be sure that the Radio Timer is active */
  while(LL_RADIO_TIMER_GetAbsoluteTime(WAKEUP) < 0x10);
  RADIO_TIMER_InitStruct.XTAL_StartupTime = 320;
  RADIO_TIMER_InitStruct.enableInitialCalibration = FALSE;
  RADIO_TIMER_InitStruct.periodicCalibrationInterval = 0;
  HAL_RADIO_TIMER_Init(&RADIO_TIMER_InitStruct);
  /* USER CODE BEGIN RADIO_TIMER_Init 2 */

  /* USER CODE END RADIO_TIMER_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 63;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 19999;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim17, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim17, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */
  HAL_TIM_MspPostInit(&htim17);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : HX711_DATA2_Pin HX711_DATA5_Pin HX711_DATA1_Pin */
  GPIO_InitStruct.Pin = HX711_DATA2_Pin|HX711_DATA5_Pin|HX711_DATA1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_SWDIO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : HX711_DATA3_Pin HX711_DATA4_Pin */
  GPIO_InitStruct.Pin = HX711_DATA3_Pin|HX711_DATA4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : HX711_SCK_Pin */
  GPIO_InitStruct.Pin = HX711_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HX711_SCK_GPIO_Port, &GPIO_InitStruct);

  /**/
  HAL_PWREx_DisableGPIOPullUp(PWR_GPIO_B, PWR_GPIO_BIT_3|PWR_GPIO_BIT_15|PWR_GPIO_BIT_7|PWR_GPIO_BIT_6);

  /**/
  HAL_PWREx_DisableGPIOPullUp(PWR_GPIO_A, PWR_GPIO_BIT_8|PWR_GPIO_BIT_11);

  /**/
  HAL_PWREx_DisableGPIOPullDown(PWR_GPIO_B, PWR_GPIO_BIT_3|PWR_GPIO_BIT_15|PWR_GPIO_BIT_7|PWR_GPIO_BIT_6);

  /**/
  HAL_PWREx_DisableGPIOPullDown(PWR_GPIO_A, PWR_GPIO_BIT_8|PWR_GPIO_BIT_11);

  /**/
  HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_A, PWR_GPIO_BIT_2);

  /*RT DEBUG GPIO_Init */
  RT_DEBUG_GPIO_Init();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* Motor/MOSFET output removed: no PB14 gate pin is configured. */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* No motor/PWM callbacks in this version. */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  if (huart1.Instance != NULL)
  {
    char err_msg[] = "\r\n*** ERROR_HANDLER: firmware stopped before/inside BLE init ***\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)err_msg, strlen(err_msg), 100U);
  }
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
