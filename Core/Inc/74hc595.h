#ifndef HC595_H
#define HC595_H

/**
 * @file 74hc595.h
 * @brief 74HC595 shift-register driver API.
 */

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define HC595_GRID_MASK_4BIT 0x0Fu
#define HC595_UNUSED_NIBBLE  0x0Fu

/**
 * @brief Runtime context for a single 74HC595 device.
 */
typedef struct {
    GPIO_TypeDef *Port;  /**< GPIO port used by all connected control pins. */
    uint16_t PinSER;     /**< Serial data input pin (SER / DS). */
    uint16_t PinSRCLK;   /**< Shift clock pin (SRCLK / SHCP). */
    uint16_t PinSRCLR;   /**< Shift-register clear pin (SRCLR, active low). */
    uint16_t PinRCLK;    /**< Latch clock pin (RCLK / STCP). */
    uint16_t PinOE;      /**< Output-enable pin (OE, active low). */
    uint16_t PinMask;    /**< Bitwise OR of all configured control pins. */
} HC595_t;

/**
 * @brief Initializes a 74HC595 context with pin assignments.
 * @param h595 Pointer to driver context to initialize.
 * @param port GPIO port for all configured pins.
 * @param ser_pin GPIO mask for SER.
 * @param srclk_pin GPIO mask for SRCLK.
 * @param srclr_pin GPIO mask for SRCLR.
 * @param rclk_pin GPIO mask for RCLK.
 * @param oe_pin GPIO mask for OE.
 */
void HC595_Init(
    HC595_t *h595,
    GPIO_TypeDef *port,
    uint16_t ser_pin,
    uint16_t srclk_pin,
    uint16_t srclr_pin,
    uint16_t rclk_pin,
    uint16_t oe_pin
);

/**
 * @brief Controls output-enable state.
 * @param h595 Pointer to initialized driver context.
 * @param enable True to enable outputs, false to disable outputs.
 */
void HC595_SetOutputEnable(HC595_t *h595, bool enable);

/**
 * @brief Controls shift-register clear line.
 * @param h595 Pointer to initialized driver context.
 * @param clear_active True to assert clear (active low SRCLR), false to release clear.
 */
void HC595_SetShiftClear(HC595_t *h595, bool clear_active);

/**
 * @brief Shifts one byte into the shift register (MSB first).
 * @param h595 Pointer to initialized driver context.
 * @param data Byte to shift.
 */
void HC595_ShiftByte(HC595_t *h595, uint8_t data);

/**
 * @brief Shifts two bytes into the shift register (high byte first).
 * @param h595 Pointer to initialized driver context.
 * @param data 16-bit value to shift.
 */
void HC595_ShiftWord(HC595_t *h595, uint16_t data);

/**
 * @brief Transfers the shift-register contents to output latches.
 * @param h595 Pointer to initialized driver context.
 */
void HC595_Latch(HC595_t *h595);

/**
 * @brief Writes a full 16-bit frame and latches it to outputs.
 * @param h595 Pointer to initialized driver context.
 * @param data 16-bit value to write.
 */
void HC595_WriteWord(HC595_t *h595, uint16_t data);

/**
 * @brief Writes one display frame in fixed serial order.
 * @param h595 Pointer to initialized driver context.
 * @param segments Segment byte (bit 7 is the dot segment).
 * @param grids Active-low grid mask; only low nibble is used (bit0..bit3 for lamp0..lamp3).
 *
 * Sent bit order:
 * 1) 8 segment bits (MSB first)
 * 2) 4 grid bits (bit0 to bit3)
 * 3) 4 unused bits set high
 */
void HC595_WriteDisplayFrame(HC595_t *h595, uint8_t segments, uint8_t grids);

#endif // HC595_H
