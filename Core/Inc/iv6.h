#ifndef IV6_H
#define IV6_H

#include "main.h"
#include "74hc595.h"
#include <stdbool.h>
#include <stdint.h>

#define IV6_DIGITS_COUNT 4u
#define IV6_DOT_BIT      0x80u

typedef struct {
    HC595_t *Shift;
    uint8_t DigitSegments[IV6_DIGITS_COUNT];
    uint8_t ScanIndex;
} IV6_t;

void IV6_Init(IV6_t *display, HC595_t *shift);
void IV6_SetDigit(IV6_t *display, uint8_t index, uint8_t value);
void IV6_SetDigits(IV6_t *display, const uint8_t values[IV6_DIGITS_COUNT]);
void IV6_SetDot(IV6_t *display, uint8_t index, bool enabled);
void IV6_Blank(IV6_t *display);
void IV6_RefreshStep(IV6_t *display);

#endif
