#include "74hc595.h"

/**
 * @brief Checks whether the driver context and GPIO port are valid.
 * @param h595 Pointer to driver context.
 * @retval true Context is ready for GPIO access.
 * @retval false Context is invalid.
 */
static inline bool HC595_IsReady(const HC595_t *h595)
{
    return (h595 != NULL) && (h595->Port != NULL);
}

/**
 * @brief Sets or resets a GPIO pin with one BSRR write.
 * @param port GPIO port pointer.
 * @param pin GPIO pin mask.
 * @param high True to set pin high, false to drive low.
 */
static inline void HC595_WritePin(GPIO_TypeDef *port, uint16_t pin, bool high)
{
    port->BSRR = high ? pin : ((uint32_t)pin << 16u);
}

/**
 * @brief Generates a high-to-low pulse on the selected pin.
 * @param h595 Pointer to initialized driver context.
 * @param pin GPIO pin mask to pulse.
 */
static inline void HC595_PulsePin(HC595_t *h595, uint16_t pin)
{
    HC595_WritePin(h595->Port, pin, true);
    HC595_WritePin(h595->Port, pin, false);
}

/**
 * @brief Writes current SER data level.
 * @param h595 Pointer to initialized driver context.
 * @param state Target SER level.
 */
static inline void HC595_SetData(HC595_t *h595, GPIO_PinState state)
{
    HC595_WritePin(h595->Port, h595->PinSER, (state == GPIO_PIN_SET));
}

/**
 * @brief Shifts one byte without validity checks.
 * @param h595 Pointer to initialized driver context.
 * @param data Byte to shift (MSB first).
 */
static void HC595_ShiftByteUnchecked(HC595_t *h595, uint8_t data)
{
    /* 74HC595 shifts most-significant bit first. */
    for (uint8_t mask = 0x80u; mask != 0u; mask >>= 1u) {
        HC595_SetData(h595, ((data & mask) != 0u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HC595_PulsePin(h595, h595->PinSRCLK);
    }
}

/**
 * @copydoc HC595_Init
 */
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
    HC595_WritePin(h595->Port, h595->PinOE, true);
    HC595_WritePin(h595->Port, h595->PinSRCLR, true);
}

/**
 * @copydoc HC595_SetOutputEnable
 */
void HC595_SetOutputEnable(HC595_t *h595, bool enable)
{
    if (!HC595_IsReady(h595)) {
        return;
    }

    HC595_WritePin(h595->Port, h595->PinOE, !enable);
}

/**
 * @copydoc HC595_SetShiftClear
 */
void HC595_SetShiftClear(HC595_t *h595, bool clear_active)
{
    if (!HC595_IsReady(h595)) {
        return;
    }

    HC595_WritePin(h595->Port, h595->PinSRCLR, !clear_active);
}

/**
 * @copydoc HC595_ShiftByte
 */
void HC595_ShiftByte(HC595_t *h595, uint8_t data)
{
    if (!HC595_IsReady(h595)) {
        return;
    }

    HC595_ShiftByteUnchecked(h595, data);
}

/**
 * @copydoc HC595_ShiftWord
 */
void HC595_ShiftWord(HC595_t *h595, uint16_t data)
{
    if (!HC595_IsReady(h595)) {
        return;
    }

    HC595_ShiftByteUnchecked(h595, (uint8_t)(data >> 8u));
    HC595_ShiftByteUnchecked(h595, (uint8_t)data);
}

/**
 * @copydoc HC595_Latch
 */
void HC595_Latch(HC595_t *h595)
{
    if (!HC595_IsReady(h595)) {
        return;
    }

    HC595_PulsePin(h595, h595->PinRCLK);
}

/**
 * @copydoc HC595_WriteWord
 */
void HC595_WriteWord(HC595_t *h595, uint16_t data)
{
    if (!HC595_IsReady(h595)) {
        return;
    }

    HC595_SetOutputEnable(h595, false);
    HC595_ShiftByteUnchecked(h595, (uint8_t)(data >> 8u));
    HC595_ShiftByteUnchecked(h595, (uint8_t)data);
    HC595_Latch(h595);
    HC595_SetOutputEnable(h595, true);
}
