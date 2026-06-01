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
 * @brief Short timing guard between GPIO edges.
 */
static inline void HC595_EdgeDelay(void)
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

/**
 * @brief Generates a low-high-low pulse on the selected pin.
 * @param h595 Pointer to initialized driver context.
 * @param pin GPIO pin mask to pulse.
 */
static inline void HC595_PulsePin(HC595_t *h595, uint16_t pin)
{
    HC595_WritePin(h595->Port, pin, false);
    HC595_EdgeDelay();
    HC595_WritePin(h595->Port, pin, true);
    HC595_EdgeDelay();
    HC595_WritePin(h595->Port, pin, false);
    HC595_EdgeDelay();
}

/**
 * @brief Shifts one data bit into the register.
 * @param h595 Pointer to initialized driver context.
 * @param state Bit value to shift.
 */
static inline void HC595_ShiftBitUnchecked(HC595_t *h595, bool state)
{
    HC595_WritePin(h595->Port, h595->PinSER, state);
    HC595_PulsePin(h595, h595->PinSRCLK);
}

/**
 * @brief Shifts one byte without validity checks.
 * @param h595 Pointer to initialized driver context.
 * @param data Byte to shift (MSB first).
 */
static void HC595_ShiftByteUnchecked(HC595_t *h595, uint8_t data)
{
    /* 74HC595 shifts on rising edge, so SER is prepared before each clock pulse. */
    for (uint8_t mask = 0x80u; mask != 0u; mask >>= 1u) {
        HC595_ShiftBitUnchecked(h595, (data & mask) != 0u);
    }
}

/**
 * @brief Shifts low nibble as 4 bits from bit0 to bit3.
 * @param h595 Pointer to initialized driver context.
 * @param nibble Low-nibble value to shift.
 */
static void HC595_ShiftNibbleBitOrderUnchecked(HC595_t *h595, uint8_t nibble)
{
    for (uint8_t bit = 0u; bit < 4u; bit++) {
        HC595_ShiftBitUnchecked(h595, ((nibble >> bit) & 0x01u) != 0u);
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

    /* Safe start: clocks low, shift clear released (SRCLR=1), outputs enabled (OE=0). */
    h595->Port->BSRR = ((uint32_t)h595->PinMask << 16u);
    HC595_WritePin(h595->Port, h595->PinOE, false);
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

    /* High byte first to keep explicit 16-bit serial order. */
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

    HC595_ShiftWord(h595, data);
    HC595_Latch(h595);
}

/**
 * @copydoc HC595_WriteDisplayFrame
 */
void HC595_WriteDisplayFrame(HC595_t *h595, uint8_t segments, uint8_t grids)
{
    if (!HC595_IsReady(h595)) {
        return;
    }

    /* Exact layout required by hardware chain: 8 segment bits, 4 grid bits, 4 unused bits. */
    HC595_ShiftByteUnchecked(h595, segments);
    HC595_ShiftNibbleBitOrderUnchecked(h595, HC595_UNUSED_NIBBLE);
    HC595_ShiftNibbleBitOrderUnchecked(h595, (uint8_t)(grids & HC595_GRID_MASK_4BIT));
    HC595_Latch(h595);
}
