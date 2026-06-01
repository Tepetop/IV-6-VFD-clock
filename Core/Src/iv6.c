#include "iv6.h"

#define IV6_SEGMENTS_OFF 0xFFu
#define IV6_GRIDS_OFF    0xFFu

/**
 * @brief Segment encoding lookup for digits 0..9.
 */
static const uint8_t iv6_digit_lut[10] = {
    0b10000100, // 0
    0b10111110, // 1
    0b11001000, // 2
    0b10011000, // 3
    0b10110010, // 4
    0b10010001, // 5
    0b10000001, // 6
    0b10111100, // 7
	0b10000000, // 8
	0b10110000, // 9
    // W HEX
    // 0x84, // 0
    // 0xBE, // 1
    // 0xC8, // 2
    // 0x98, // 3
    // 0xB2, // 4
    // 0x91, // 5
    // 0x81, // 6
    // 0xBC, // 7
    // 0x80, // 8
	// 0xB0, // 9
};


/**
 * @brief Checks whether display and shift-register contexts are valid.
 * @param display Pointer to display context.
 * @retval true Display can be refreshed.
 * @retval false Display context is invalid.
 */
static inline bool IV6_IsReady(const IV6_t *display)
{
	return (display != NULL) && (display->Shift != NULL);
}

/**
 * @brief Validates a display digit index.
 * @param index Digit index.
 * @retval true Index is in range.
 * @retval false Index is out of range.
 */
static inline bool IV6_IsValidIndex(uint8_t index)
{
	return index < IV6_DIGITS_COUNT;
}

/**
 * @brief Fills all digits with one segment pattern.
 * @param display Pointer to display context.
 * @param segments Segment byte to store for each digit.
 */
static void IV6_FillDigits(IV6_t *display, uint8_t segments)
{
	for (uint8_t i = 0u; i < IV6_DIGITS_COUNT; i++) {
		display->DigitSegments[i] = segments;
	}
}

/**
 * @brief Replaces digit segments while keeping current dot bit.
 * @param current_segments Existing segment pattern.
 * @param encoded_digit Newly encoded pattern.
 * @return Segment pattern with preserved dot state.
 */
static uint8_t IV6_PreserveDotState(uint8_t current_segments, uint8_t encoded_digit)
{
	uint8_t dot_state = (uint8_t)(current_segments & IV6_DOT_BIT);
	return (uint8_t)((encoded_digit & (uint8_t)~IV6_DOT_BIT) | dot_state);
}

/**
 * @brief Computes the next scan index in circular order.
 * @param index Current scan index.
 * @return Next index in range [0, IV6_DIGITS_COUNT).
 */
static uint8_t IV6_NextScanIndex(uint8_t index)
{
	index++;
	return (index < IV6_DIGITS_COUNT) ? index : 0u;
}

/**
 * @brief Encodes a decimal digit into IV-6 segment mask.
 * @param digit Decimal value.
 * @return Encoded segments, or all-off mask for out-of-range value.
 */
static uint8_t IV6_EncodeDigit(uint8_t digit)
{
	return (digit < 10u) ? iv6_digit_lut[digit] : IV6_SEGMENTS_OFF;
}

/**
 * @brief Builds active-low grid mask for one digit.
 * @param index Active digit index.
 * @return Grid byte with selected digit enabled.
 */
static uint8_t IV6_GridMask(uint8_t index)
{
	if (IV6_IsValidIndex(index) == false) {
		return IV6_GRIDS_OFF;
	}

	return (uint8_t)(IV6_GRIDS_OFF & (uint8_t)~(1u << index));
}

/**
 * @copydoc IV6_Init
 */
void IV6_Init(IV6_t *display, HC595_t *shift)
{
	if ((display == NULL) || (shift == NULL)) {
		return;
	}

	display->Shift = shift;
	display->ScanIndex = 0u;
	IV6_FillDigits(display, IV6_SEGMENTS_OFF);

	HC595_SetShiftClear(shift, false);
	HC595_WriteDisplayFrame(shift, IV6_SEGMENTS_OFF, IV6_GRIDS_OFF);
}

/**
 * @copydoc IV6_SetDigit
 */
void IV6_SetDigit(IV6_t *display, uint8_t index, uint8_t value)
{
	if (IV6_IsReady(display) == false || IV6_IsValidIndex(index) == false) {
		return;
	}

	display->DigitSegments[index] = IV6_PreserveDotState(display->DigitSegments[index], IV6_EncodeDigit(value));
}

/**
 * @copydoc IV6_SetDigits
 */
void IV6_SetDigits(IV6_t *display, const uint8_t values[IV6_DIGITS_COUNT])
{
	if (IV6_IsReady(display) == false || (values == NULL)) {
		return;
	}

	for (uint8_t i = 0u; i < IV6_DIGITS_COUNT; i++) {
		display->DigitSegments[i] = IV6_PreserveDotState(display->DigitSegments[i], IV6_EncodeDigit(values[i]));
	}
}

/**
 * @copydoc IV6_SetDot
 */
void IV6_SetDot(IV6_t *display, uint8_t index, bool enabled)
{
	if (IV6_IsReady(display) == false || IV6_IsValidIndex(index) == false) {
		return;
	}

	if (enabled) {
		display->DigitSegments[index] &= (uint8_t)~IV6_DOT_BIT;
	} else {
		display->DigitSegments[index] |= IV6_DOT_BIT;
	}
}

/**
 * @copydoc IV6_Blank
 */
void IV6_Blank(IV6_t *display)
{
	if (IV6_IsReady(display) == false) {
		return;
	}

	IV6_FillDigits(display, IV6_SEGMENTS_OFF);
	display->ScanIndex = 0u;
	HC595_WriteDisplayFrame(display->Shift, IV6_SEGMENTS_OFF, IV6_GRIDS_OFF);
}

/**
 * @copydoc IV6_RefreshStep
 */

void IV6_RefreshStep(IV6_t *display)
{
	if (IV6_IsReady(display) == false) {
		return;
	}

	uint8_t index = display->ScanIndex;
	if (IV6_IsValidIndex(index) == false) {
		index = 0u;
	}

	HC595_WriteDisplayFrame(display->Shift, display->DigitSegments[index], IV6_GridMask(index));

	display->ScanIndex = IV6_NextScanIndex(index);
}

