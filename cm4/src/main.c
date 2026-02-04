#include "common.h"

#define LED_RED_PIN 14

int main(void)
{
    /* Enable Clock for GPIOB */
    RCC_AHB4ENR |= RCC_AHB4ENR_GPIOBEN;

    /* Setup PB14 as Output */
    GPIOB_MODER &= ~(0x3 << (LED_RED_PIN * 2));
    GPIOB_MODER |= (GPIO_MODER_OUT << (LED_RED_PIN * 2));

    while (1) {
        GPIOB_ODR ^= (1 << LED_RED_PIN); // Toggle Red
        delay(2000000);
    }
}
