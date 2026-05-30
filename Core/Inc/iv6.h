#ifndef IV6_H
#define IV6_H

/**
 * @file iv6.h
 * @brief IV-6 multiplexed display driver API.
 */

#include "main.h"
#include "74hc595.h"
//#include <stdbool.h>
#include <stdint.h>


#define IV6_DIGITS_COUNT 4u    /**< Number of IV-6 digits in the display. */
#define IV6_DOT_BIT      0x80u /**< Bit mask for dot segment state. */

/**
 * @brief Runtime context for IV-6 display multiplexing.
 */
typedef struct {
    HC595_t *Shift;                             /**< Attached 74HC595 driver context. */
    uint8_t DigitSegments[IV6_DIGITS_COUNT];    /**< Per-digit encoded segment pattern. */
    uint8_t ScanIndex;                          /**< Active scan position for next refresh step. */
} IV6_t;

/**
 * @brief Initializes display state and blanks outputs.
 * @param display Pointer to display context.
 * @param shift Pointer to initialized 74HC595 context.
 */
void IV6_Init(IV6_t *display, HC595_t *shift);

/**
 * @brief Sets one numeric digit while preserving dot state.
 * @param display Pointer to display context.
 * @param index Digit index in range [0, IV6_DIGITS_COUNT).
 * @param value Numeric value in range [0, 9]. Out-of-range values blank the digit.
 */
void IV6_SetDigit(IV6_t *display, uint8_t index, uint8_t value);

/**
 * @brief Sets all display digits in one call.
 * @param display Pointer to display context.
 * @param values Array of IV6_DIGITS_COUNT numeric values.
 */
void IV6_SetDigits(IV6_t *display, const uint8_t values[IV6_DIGITS_COUNT]);

/**
 * @brief Enables or disables the dot segment for one digit.
 * @param display Pointer to display context.
 * @param index Digit index in range [0, IV6_DIGITS_COUNT).
 * @param enabled True to enable dot, false to disable dot.
 */
void IV6_SetDot(IV6_t *display, uint8_t index, bool enabled);

/**
 * @brief Clears all digits and resets scan position.
 * @param display Pointer to display context.
 */
void IV6_Blank(IV6_t *display);

/**
 * @brief Refreshes one multiplex step for the current scan index.
 * @param display Pointer to display context.
 */
void IV6_RefreshStep(IV6_t *display);

#endif
