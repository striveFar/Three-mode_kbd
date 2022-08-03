#include "auto_gpio.h"

void gpio_init(enum GPIO_pins pin, GPIOModeTypeDef mode, enum GPIO_STATE state) {
    if (pin >= PA0 && pin <= PA15)
        GPIOA_ModeCfg((1 << pin), mode);
    else
        GPIOB_ModeCfg((1 << (pin - PA15)), mode);

    switch (state) {
    case LOW:
        gpio_reset(pin);
        break;
    case HIGH:
        gpio_set(pin);
        break;
    }
}

void gpio_set(enum GPIO_pins pin) {
    if (pin >= PA0 && pin <= PA15)
        GPIOA_SetBits(1 << pin);
    else
        GPIOA_SetBits(1 << (pin - PA15));
}

void gpio_reset(enum GPIO_pins pin) {
    if (pin >= PA0 && pin <= PA15)
        GPIOA_ResetBits(1 << pin);
    else
        GPIOA_ResetBits(1 << (pin - PA15));
}

void gpio_write(enum GPIO_pins pin, UINT8 val) {
    if (1 == val)
        gpio_set(pin);
    else if (0 == val)
        gpio_reset(pin);
}

UINT8 gpio_read(enum GPIO_pins pin) {
    if (pin >= PA0 && pin <= PA15)
        return !!(R32_PA_PIN & (1 << pin));
    else
        return !!(R32_PB_PIN & (1 << (pin - PA15)));
}
