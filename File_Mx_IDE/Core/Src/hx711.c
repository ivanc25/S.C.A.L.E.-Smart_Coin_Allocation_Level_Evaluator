/*
 * hx711.c
 * Gestione di cinque convertitori HX711 con clock condiviso e linee dati separate.
 * Le funzioni "Task" sono non bloccanti e permettono al firmware BLE di continuare
 * a processare gli eventi durante tara e acquisizione pesi.
 */
#include "hx711.h"
#include <string.h>

int32_t hx711_offsets[5] = {0, 0, 0, 0, 0};
float hx711_scales[5] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

#define HX711_CHANNELS        5U
#define HX711_BLOCK_TIMEOUT_MS 1000U

static int64_t tare_sums[HX711_CHANNELS] = {0};
static uint8_t tare_count = 0U;
static uint8_t tare_active = 0U;

static int64_t avg_sums[HX711_CHANNELS] = {0};
static uint8_t avg_count = 0U;

static uint8_t HX711_WaitReadyAll(uint32_t timeout_ms);
static void HX711_ReadReadyAll(int32_t* dati_uscita);

uint8_t HX711_IsReadyAll(void)
{
    return (
        (HAL_GPIO_ReadPin(HX711_DATA1_GPIO_Port, HX711_DATA1_Pin) == GPIO_PIN_RESET) &&
        (HAL_GPIO_ReadPin(HX711_DATA2_GPIO_Port, HX711_DATA2_Pin) == GPIO_PIN_RESET) &&
        (HAL_GPIO_ReadPin(HX711_DATA3_GPIO_Port, HX711_DATA3_Pin) == GPIO_PIN_RESET) &&
        (HAL_GPIO_ReadPin(HX711_DATA4_GPIO_Port, HX711_DATA4_Pin) == GPIO_PIN_RESET) &&
        (HAL_GPIO_ReadPin(HX711_DATA5_GPIO_Port, HX711_DATA5_Pin) == GPIO_PIN_RESET)
    ) ? 1U : 0U;
}

static uint8_t HX711_WaitReadyAll(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();

    while (HX711_IsReadyAll() == 0U)
    {
        if ((HAL_GetTick() - start) >= timeout_ms)
        {
            return 0U;
        }
    }

    return 1U;
}

static void HX711_ReadReadyAll(int32_t* dati_uscita)
{
    uint32_t data[HX711_CHANNELS] = {0, 0, 0, 0, 0};
    uint32_t primask;

    /* Proteggiamo solo il breve burst di 24 bit: la temporizzazione del clock HX711
       resta stabile, mentre il BLE non viene bloccato per l'intero tempo di conversione. */
    primask = __get_PRIMASK();
    __disable_irq();

    for (uint8_t i = 0; i < 24U; i++)
    {
        HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);
        __NOP(); __NOP(); __NOP(); __NOP();

        for(uint8_t j = 0; j < HX711_CHANNELS; j++)
        {
            data[j] <<= 1;
        }

        HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);
        __NOP(); __NOP(); __NOP(); __NOP();

        if (HAL_GPIO_ReadPin(HX711_DATA1_GPIO_Port, HX711_DATA1_Pin) == GPIO_PIN_SET) data[0]++;
        if (HAL_GPIO_ReadPin(HX711_DATA2_GPIO_Port, HX711_DATA2_Pin) == GPIO_PIN_SET) data[1]++;
        if (HAL_GPIO_ReadPin(HX711_DATA3_GPIO_Port, HX711_DATA3_Pin) == GPIO_PIN_SET) data[2]++;
        if (HAL_GPIO_ReadPin(HX711_DATA4_GPIO_Port, HX711_DATA4_Pin) == GPIO_PIN_SET) data[3]++;
        if (HAL_GPIO_ReadPin(HX711_DATA5_GPIO_Port, HX711_DATA5_Pin) == GPIO_PIN_SET) data[4]++;
    }

    /* 25° impulso: seleziona canale A con guadagno 128 per la lettura successiva. */
    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);
    __NOP(); __NOP(); __NOP(); __NOP();
    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);

    if (primask == 0U)
    {
        __enable_irq();
    }

    for(uint8_t i = 0; i < HX711_CHANNELS; i++)
    {
        if (data[i] & 0x800000U)
        {
            data[i] |= 0xFF000000U;
        }
        dati_uscita[i] = (int32_t)data[i];
    }
}

uint8_t HX711_ReadAllIfReady(int32_t* dati_uscita)
{
    if (HX711_IsReadyAll() == 0U)
    {
        return 0U;
    }

    HX711_ReadReadyAll(dati_uscita);
    return 1U;
}

void HX711_ReadAll(int32_t* dati_uscita)
{
    /* Variante bloccante usata solo dove è accettabile attendere il dato pronto. */
    if (HX711_WaitReadyAll(HX711_BLOCK_TIMEOUT_MS) == 0U)
    {
        for (uint8_t i = 0; i < HX711_CHANNELS; i++)
        {
            dati_uscita[i] = hx711_offsets[i];
        }
        return;
    }

    HX711_ReadReadyAll(dati_uscita);
}

void HX711_TareAllBegin(void)
{
    memset(tare_sums, 0, sizeof(tare_sums));
    tare_count = 0U;
    tare_active = 1U;
}

uint8_t HX711_TareAllTask(uint8_t times)
{
    int32_t temp_data[HX711_CHANNELS];

    if (times == 0U)
    {
        times = 1U;
    }

    if (tare_active == 0U)
    {
        HX711_TareAllBegin();
    }

    if (HX711_ReadAllIfReady(temp_data) == 0U)
    {
        return 0U;
    }

    for(uint8_t j = 0; j < HX711_CHANNELS; j++)
    {
        tare_sums[j] += temp_data[j];
    }
    tare_count++;

    if (tare_count >= times)
    {
        for(uint8_t j = 0; j < HX711_CHANNELS; j++)
        {
            hx711_offsets[j] = (int32_t)(tare_sums[j] / times);
        }
        tare_active = 0U;
        return 1U;
    }

    return 0U;
}

/* Calcola l'offset a vuoto mediando più campioni.
   Versione bloccante mantenuta per compatibilità; nel main con BLE viene usata HX711_TareAllTask(). */
void HX711_TareAll(uint8_t times)
{
    int64_t sums[HX711_CHANNELS] = {0};
    int32_t temp_data[HX711_CHANNELS];

    if (times == 0U)
    {
        times = 1U;
    }

    for (uint8_t i = 0; i < times; i++)
    {
        HX711_ReadAll(temp_data);
        for(uint8_t j = 0; j < HX711_CHANNELS; j++)
        {
            sums[j] += temp_data[j];
        }
    }

    for(uint8_t j = 0; j < HX711_CHANNELS; j++)
    {
        hx711_offsets[j] = (int32_t)(sums[j] / times);
    }
}

/* Imposta i fattori di scala ottenuti dalla calibrazione con un peso di riferimento. */
void HX711_SetScales(float s1, float s2, float s3, float s4, float s5)
{
    hx711_scales[0] = s1;
    hx711_scales[1] = s2;
    hx711_scales[2] = s3;
    hx711_scales[3] = s4;
    hx711_scales[4] = s5;
}

uint8_t HX711_GetWeightsAveragedTask(float* pesi_uscita, uint8_t times)
{
    /* Acquisizione cooperativa: restituisce 1 solo quando la media richiesta è completa. */
    int32_t temp_data[HX711_CHANNELS];

    if (times == 0U)
    {
        times = 1U;
    }

    if (HX711_ReadAllIfReady(temp_data) == 0U)
    {
        return 0U;
    }

    for(uint8_t j = 0; j < HX711_CHANNELS; j++)
    {
        avg_sums[j] += temp_data[j];
    }
    avg_count++;

    if (avg_count < times)
    {
        return 0U;
    }

    for(uint8_t j = 0; j < HX711_CHANNELS; j++)
    {
        int32_t val_medio = (int32_t)(avg_sums[j] / times);
        pesi_uscita[j] = (float)(val_medio - hx711_offsets[j]) / hx711_scales[j];
    }

    memset(avg_sums, 0, sizeof(avg_sums));
    avg_count = 0U;
    return 1U;
}

/* Restituisce il peso medio su n letture.
   Versione bloccante mantenuta per compatibilità; nel main con BLE viene usata HX711_GetWeightsAveragedTask(). */
void HX711_GetWeights(float* pesi_uscita, uint8_t times)
{
    int64_t sums[HX711_CHANNELS] = {0};
    int32_t temp_data[HX711_CHANNELS];

    if (times == 0U)
    {
        times = 1U;
    }

    for (uint8_t i = 0; i < times; i++)
    {
        HX711_ReadAll(temp_data);
        for(uint8_t j = 0; j < HX711_CHANNELS; j++)
        {
            sums[j] += temp_data[j];
        }
    }

    for(uint8_t j = 0; j < HX711_CHANNELS; j++)
    {
        int32_t val_medio = (int32_t)(sums[j] / times);
        pesi_uscita[j] = (float)(val_medio - hx711_offsets[j]) / hx711_scales[j];
    }
}
