#ifndef IV6_H
#define IV6_H

#include <main.h>


// Ustawienie pinów dla lampy. MSB idzie jako pierwszy. pin 10 to segment 7, pin 11 to segment 8 czyli kropka.
//          Ciag idzie tak: 11, 10, 6, 5, 4, 3, 2, 1, 0, dodatkowo logika jest odwrotna, czyli 0 to zapalony segment, a 1 to zgaszony segment                 
extern uint8_t iv6_digits[10];


/*      FUNCTIONS PROTOTYPES       */
void IV6_WritePin(GPIO_PinState state);
void iv6_set_digit(uint8_t digit);


#endif
