#ifndef SERVO_TG9E_H
#define SERVO_TG9E_H

#include "stm32wb0x_hal.h"

/* Inizializza il servo associandolo al timer/canale PWM scelto. */
void ServoTG9e_Init(TIM_HandleTypeDef *htim, uint32_t channel);

/* Imposta l'angolo del servo nell'intervallo 0..180 gradi. */
void ServoTG9e_SetAngle(uint8_t angolo);

#endif /* SERVO_TG9E_H */
