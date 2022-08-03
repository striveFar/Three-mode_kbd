#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "timer.h"
#include "wait.h"
#include "matrix.h"
#include "../keyboard/custom_config.h"
#include "../../tmk_core/ch58x/auto_gpio.h"

/*
 * Infinity Pinusage:
 * Column pins are input with internal pull-down. Row pins are output and strobe with high.
 * Key is high or 1 when it turns on.
 */

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];
static bool debouncing = false;
static uint16_t debouncing_time = 0;


void matrix_init(void)
{
    /* Column(sense) */
    for(int i = 0; i < MATRIX_COLS; i++)
        gpio_init(column_pin_array[i], GPIO_ModeIN_PD, NONE);

    /* Row(strobe) */
    for(int i = 0; i < MATRIX_ROWS; i++)
            gpio_init(row_pin_array[i], GPIO_ModeOut_PP_5mA, LOW);
}

uint8_t matrix_scan(void)
{
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix_row_t r = 0;

        gpio_write(row_pin_array[i], 1);
        wait_us(1); // need wait to settle pin state
        for (int j = 0; j < MATRIX_COLS; j++) {
            if (gpio_read(column_pin_array[j])) {
                r |= (1<<j);
            }
        }
        gpio_write(row_pin_array[i], 0);

        if (matrix_debouncing[i] != r) {
            matrix_debouncing[i] = r;
            debouncing = true;
            debouncing_time = timer_read();
        }
    }

    if (debouncing && timer_elapsed(debouncing_time) > DEBOUNCE) {
        for (int i = 0; i < MATRIX_ROWS; i++) {
            matrix[i] = matrix_debouncing[i];
        }
        debouncing = false;
    }
    return 1;
}

bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & (1<<col));
}

matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

void matrix_print(void)
{
}
