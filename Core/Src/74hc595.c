#include "74hc595.h"

static inline void HC595_PulsePin(HC595_t *h595, uint16_t pin)
{
    h595->Port->BSRR = pin;
    h595->Port->BSRR = ((uint32_t)pin << 16u);
}

static inline void HC595_SetData(HC595_t *h595, GPIO_PinState state)
{
    uint16_t odr = (uint16_t)(h595->Port->ODR & (uint16_t)~h595->PinSER);
    if (state == GPIO_PIN_SET) {
        odr |= h595->PinSER;
    }
    h595->Port->ODR = odr;
}

void HC595_Init(
    HC595_t *h595,
    GPIO_TypeDef *port,
    uint16_t ser_pin,
    uint16_t srclk_pin,
    uint16_t srclr_pin,
    uint16_t rclk_pin,
    uint16_t oe_pin
)
{
    if ((h595 == NULL) || (port == NULL)) {
        return;
    }

    h595->Port = port;
    h595->PinSER = ser_pin;
    h595->PinSRCLK = srclk_pin;
    h595->PinSRCLR = srclr_pin;
    h595->PinRCLK = rclk_pin;
    h595->PinOE = oe_pin;
    h595->PinMask = (uint16_t)(ser_pin | srclk_pin | srclr_pin | rclk_pin | oe_pin);

    /* Safe start: outputs disabled (OE=1), shift clear released (SRCLR=1). */
    h595->Port->BSRR = ((uint32_t)h595->PinMask << 16u);
    h595->Port->BSRR = (h595->PinOE | h595->PinSRCLR);
}

void HC595_SetOutputEnable(HC595_t *h595, bool enable)
{
    if ((h595 == NULL) || (h595->Port == NULL)) {
        return;
    }

    h595->Port->BSRR = enable ? ((uint32_t)h595->PinOE << 16u) : h595->PinOE;
}

void HC595_SetShiftClear(HC595_t *h595, bool clear_active)
{
    if ((h595 == NULL) || (h595->Port == NULL)) {
        return;
    }

    h595->Port->BSRR = clear_active ? ((uint32_t)h595->PinSRCLR << 16u) : h595->PinSRCLR;
}

void HC595_ShiftByte(HC595_t *h595, uint8_t data)
{
    if ((h595 == NULL) || (h595->Port == NULL)) {
        return;
    }

    for (uint8_t i = 0; i < 8u; i++) {
        HC595_SetData(h595, ((data >> (7u - i)) & 0x01u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HC595_PulsePin(h595, h595->PinSRCLK);
    }
}

void HC595_ShiftWord(HC595_t *h595, uint16_t data)
{
    HC595_ShiftByte(h595, (uint8_t)(data >> 8u));
    HC595_ShiftByte(h595, (uint8_t)data);
}

void HC595_Latch(HC595_t *h595)
{
    if ((h595 == NULL) || (h595->Port == NULL)) {
        return;
    }

    HC595_PulsePin(h595, h595->PinRCLK);
}

void HC595_WriteWord(HC595_t *h595, uint16_t data)
{
    if ((h595 == NULL) || (h595->Port == NULL)) {
        return;
    }

    HC595_SetOutputEnable(h595, false);
    HC595_ShiftWord(h595, data);
    HC595_Latch(h595);
    HC595_SetOutputEnable(h595, true);
}
