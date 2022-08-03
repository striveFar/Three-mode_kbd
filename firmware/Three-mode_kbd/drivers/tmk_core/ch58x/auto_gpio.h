#ifndef AUTO_GPIO_H
#define AUTO_GPIO_H

#include "CH58x_common.h"
#include <stdint.h>
#include <stdio.h>

void gpio_init(enum GPIO_pins pin, GPIOModeTypeDef mode, enum GPIO_STATE state);
void gpio_set(enum GPIO_pins pin);
void gpio_reset(enum GPIO_pins pin);
void gpio_write(enum GPIO_pins pin, UINT8 val);
UINT8 gpio_read(enum GPIO_pins pin);

#endif
