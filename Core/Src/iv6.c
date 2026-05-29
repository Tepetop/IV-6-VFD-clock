#include "iv6.h"

static const uint8_t iv6_digit_lut[10] = {
	0x84u, // 0
	0xBEu, // 1
	0xC8u, // 2
	0x98u, // 3
	0xB2u, // 4
	0x91u, // 5
	0x81u, // 6
	0xBCu, // 7
	0x80u, // 8
	0xB0u  // 9
};

static uint8_t IV6_EncodeDigit(uint8_t digit)
{
	return (digit < 10u) ? iv6_digit_lut[digit] : 0xFFu;
}

static uint8_t IV6_GridMask(uint8_t index)
{
	if (index >= IV6_DIGITS_COUNT) {
		return 0xFFu;
	}

	return (uint8_t)(0xFFu & (uint8_t)~(1u << index));
}

static uint16_t IV6_BuildFrame(uint8_t segments, uint8_t grids)
{
	return (uint16_t)(((uint16_t)segments << 8u) | (uint16_t)grids);
}

void IV6_Init(IV6_t *display, HC595_t *shift)
{
	if ((display == NULL) || (shift == NULL)) {
		return;
	}

	display->Shift = shift;
	display->ScanIndex = 0u;

	for (uint8_t i = 0u; i < IV6_DIGITS_COUNT; i++) {
		display->DigitSegments[i] = 0xFFu;
	}

	HC595_SetShiftClear(shift, false);
	HC595_WriteWord(shift, IV6_BuildFrame(0xFFu, 0xFFu));
}

void IV6_SetDigit(IV6_t *display, uint8_t index, uint8_t value)
{
	if ((display == NULL) || (display->Shift == NULL) || (index >= IV6_DIGITS_COUNT)) {
		return;
	}

	uint8_t dot_state = (uint8_t)(display->DigitSegments[index] & IV6_DOT_BIT);
	display->DigitSegments[index] = (uint8_t)((IV6_EncodeDigit(value) & (uint8_t)~IV6_DOT_BIT) | dot_state);
}

void IV6_SetDigits(IV6_t *display, const uint8_t values[IV6_DIGITS_COUNT])
{
	if ((display == NULL) || (display->Shift == NULL) || (values == NULL)) {
		return;
	}

	for (uint8_t i = 0u; i < IV6_DIGITS_COUNT; i++) {
		IV6_SetDigit(display, i, values[i]);
	}
}

void IV6_SetDot(IV6_t *display, uint8_t index, bool enabled)
{
	if ((display == NULL) || (display->Shift == NULL) || (index >= IV6_DIGITS_COUNT)) {
		return;
	}

	if (enabled) {
		display->DigitSegments[index] &= (uint8_t)~IV6_DOT_BIT;
	} else {
		display->DigitSegments[index] |= IV6_DOT_BIT;
	}
}

void IV6_Blank(IV6_t *display)
{
	if ((display == NULL) || (display->Shift == NULL)) {
		return;
	}

	for (uint8_t i = 0u; i < IV6_DIGITS_COUNT; i++) {
		display->DigitSegments[i] = 0xFFu;
	}

	display->ScanIndex = 0u;
	HC595_WriteWord(display->Shift, IV6_BuildFrame(0xFFu, 0xFFu));
}

void IV6_RefreshStep(IV6_t *display)
{
	if ((display == NULL) || (display->Shift == NULL)) {
		return;
	}

	uint8_t index = display->ScanIndex;
	if (index >= IV6_DIGITS_COUNT) {
		index = 0u;
	}

	uint16_t frame = IV6_BuildFrame(display->DigitSegments[index], IV6_GridMask(index));
	HC595_WriteWord(display->Shift, frame);

	display->ScanIndex = (uint8_t)((index + 1u) % IV6_DIGITS_COUNT);
}

