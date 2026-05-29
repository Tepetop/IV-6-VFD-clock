#ifndef _74HC164_H
#define _74HC164_H

#include "stm32f1xx_hal.h"

/**
 * @brief Struktura konfiguracyjna rejestru 74HC164
 */
typedef struct {
    GPIO_TypeDef *DATA_Port;
    uint16_t DATA_Pin;
    GPIO_TypeDef *CLK_Port;
    uint16_t CLK_Pin;
} HC164_t;

/**
 * @brief Inicjalizuje rejestr 74HC164 podanymi portami i pinami GPIO
 * @param h164 Wskaźnik na strukturę obsługi rejestru
 */
void HC164_Init(HC164_t *h164, GPIO_TypeDef *DATA_Port, uint16_t DATA_Pin, GPIO_TypeDef *CLK_Port, uint16_t CLK_Pin);

/**
 * @brief Wysyła jeden bajt do rejestru 74HC164
 * @param h164 Wskaźnik na strukturę obsługi rejestru
 * @param data Bajt do wysłania
 */
void HC164_SendByte(HC164_t *h164, uint8_t data);

#endif // _74HC164_H
