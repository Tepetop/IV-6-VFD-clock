#include "74hc164.h"
#include <stdint.h>

// Helper: Set clock pin state
static inline void HC164_SetCLK(HC164_t *h164, GPIO_PinState state) {
    // Podejscie z rejestrami
     h164->CLK_Port->BSRR = (state == GPIO_PIN_SET) ? h164->CLK_Pin : (h164->CLK_Pin << 16u);
    // Podejscia z HAL
    //HAL_GPIO_WritePin(h164->CLK_Port, h164->CLK_Pin, state);
}

// Helper: Set data pin state
static inline void HC164_SetDATA(HC164_t *h164, GPIO_PinState state) {
    // Podejscie z rejestrami
     h164->DATA_Port->BSRR = (state == GPIO_PIN_SET) ? h164->DATA_Pin : (h164->DATA_Pin << 16u);
    // Podejscia z HAL
    //HAL_GPIO_WritePin(h164->DATA_Port, h164->DATA_Pin, state);
}

void HC164_Init(HC164_t *h164, GPIO_TypeDef *DATA_Port, uint16_t DATA_Pin, GPIO_TypeDef *CLK_Port, uint16_t CLK_Pin) 
{
    if(h164 == NULL || DATA_Port == NULL || CLK_Port == NULL) 
    {
        return;
    }

    h164->DATA_Port = DATA_Port;
    h164->DATA_Pin = DATA_Pin;
    h164->CLK_Port = CLK_Port;
    h164->CLK_Pin = CLK_Pin;
}

void HC164_SendByte(HC164_t *h164, uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        HC164_SetCLK(h164, GPIO_PIN_RESET);
        HC164_SetDATA(h164, ((data >> (7 - i)) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        // ustawic odpowiedni czas zgodny z diagramem czasowym 74HC164
        HC164_SetCLK(h164, GPIO_PIN_SET);
    }
    // Wyjsciowy stan niski sygnalu CLK
    HC164_SetCLK(h164, GPIO_PIN_RESET);
}
