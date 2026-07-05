#include "servo_tg9e.h"

/* Riferimenti al timer PWM usato per generare il segnale del servocomando. */
static TIM_HandleTypeDef *servo_htim;
static uint32_t servo_channel;

void ServoTG9e_Init(TIM_HandleTypeDef *htim, uint32_t channel)
{
    servo_htim = htim;
    servo_channel = channel;

    /* Avvia il PWM sul canale configurato da CubeMX. */
    HAL_TIM_PWM_Start(servo_htim, servo_channel);
}

void ServoTG9e_SetAngle(uint8_t angolo)
{
    /* Limita il comando meccanico all'intervallo ammesso dal servo. */
    if (angolo > 180U)
    {
        angolo = 180U;
    }

    /* Timer configurato a 1 tick = 1 us: 0° -> 500 us, 180° -> 2500 us. */
    uint32_t usPWM = 500U + ((uint32_t)angolo * 2000U / 180U);

    /* Aggiorna il duty cycle modificando il registro Capture/Compare. */
    __HAL_TIM_SET_COMPARE(servo_htim, servo_channel, usPWM);
}
