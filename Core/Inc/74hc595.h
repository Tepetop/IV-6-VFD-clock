#ifndef _74HC595_H
#define _74HC595_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    GPIO_TypeDef *Port;
    uint16_t PinSER;
    uint16_t PinSRCLK;
    uint16_t PinSRCLR;
    uint16_t PinRCLK;
    uint16_t PinOE;
    uint16_t PinMask;
} HC595_t;

void HC595_Init(
    HC595_t *h595,
    GPIO_TypeDef *port,
    uint16_t ser_pin,
    uint16_t srclk_pin,
    uint16_t srclr_pin,
    uint16_t rclk_pin,
    uint16_t oe_pin
);

void HC595_SetOutputEnable(HC595_t *h595, bool enable);
void HC595_SetShiftClear(HC595_t *h595, bool clear_active);
void HC595_ShiftByte(HC595_t *h595, uint8_t data);
void HC595_ShiftWord(HC595_t *h595, uint16_t data);
void HC595_Latch(HC595_t *h595);
void HC595_WriteWord(HC595_t *h595, uint16_t data);

#endif // _74HC595_H
