/*
 * hx711.h
 * Interfaccia per acquisizione pesi da cinque HX711.
 */
#ifndef INC_HX711_H_
#define INC_HX711_H_

#include "main.h"

// Prototipi delle funzioni per 5 bilance
void HX711_ReadAll(int32_t* dati_uscita);
void HX711_TareAll(uint8_t times);
void HX711_SetScales(float s1, float s2, float s3, float s4, float s5);
void HX711_GetWeights(float* pesi_uscita, uint8_t times);

// Non-blocking helpers used when BLE is active. They never wait for the HX711 conversion.
uint8_t HX711_IsReadyAll(void);
uint8_t HX711_ReadAllIfReady(int32_t* dati_uscita);
void HX711_TareAllBegin(void);
uint8_t HX711_TareAllTask(uint8_t times);
uint8_t HX711_GetWeightsAveragedTask(float* pesi_uscita, uint8_t times);

#endif /* INC_HX711_H_ */
