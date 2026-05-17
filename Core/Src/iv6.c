#include <iv6.h>

// Ustawienie pinów dla lampy. MSB idzie jako pierwszy. pin 10 to segment 7, pin 11 to segment 8 czyli kropka.
// Ciag idzie tak: 11, 10, 6, 5, 4, 3, 2, 1, 0, dodatkowo logika jest odwrotna, czyli 0 to zapalony segment, a 1 to zgaszony segment
uint8_t iv6_digits[10] =
{
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
    // 0b01111111 //  kropka
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
    // 0x7F  //  kropka
};
#include <iv6.h>



/* @desc Tymczasowa funkcja do ustawiania pinów i toglowanie zegarem
*
*
*/
void IV6_WritePin(GPIO_PinState state)
{
  HAL_GPIO_WritePin(CLK_PIN_GPIO_Port, CLK_PIN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DATA_A_GPIO_Port, DATA_A_Pin, state);
  HAL_GPIO_WritePin(CLK_PIN_GPIO_Port, CLK_PIN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(CLK_PIN_GPIO_Port, CLK_PIN_Pin, GPIO_PIN_RESET);
}

/* @desc funkcja ustawiajaca cyfry na lampie
*
*
*/
void iv6_set_digit(uint8_t digit)
{
    if (digit < 10)
    {
        // Ustawienie pinów zgodnie z wartością z tablicy iv6_digits
        // Piny 1-11 są ustawiane na podstawie bitów w iv6_digits[digit]
        for (int i = 0; i < 9; i++)
        {
            // Ustawienie pinu i+1 na podstawie bitu i w iv6_digits[digit]
            // Logika jest odwrotna, więc 0 to zapalony segment, a 1 to zgaszony segment
            if ((iv6_digits[digit] >> (8 - i)) & 0x01)
            {
                // Zgaszenie segmentu (ustawienie pinu na HIGH)
                IV6_WritePin(GPIO_PIN_SET);
            }
            else
            {
                // Zapalenie segmentu (ustawienie pinu na LOW)
                IV6_WritePin(GPIO_PIN_RESET);
            }
        }
    }
}