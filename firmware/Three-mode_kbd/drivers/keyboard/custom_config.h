/*
Copyright 2014 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include "CH58x_common.h"

#if 0
// duplicated name against mbed USBDeivce
#define VENDOR_ID       0xFEED
#define PRODUCT_ID      0x3BED
#endif
#define DEVICE_VER      0x0001
#define MANUFACTURER    tmk.
#define PRODUCT         Infinitiy
#define DESCRIPTION     Massdrop Infinity keyboard firmware by tmk

#define NKRO_EPSIZE 32


/* matrix size */
//#define MATRIX_ROWS 6   // Strobe
//#define MATRIX_COLS 17   // Sense

#define MATRIX_ROWS 9   // Strobe
#define MATRIX_COLS 7   // Sense

#ifndef DEBOUNCE
#define DEBOUNCE    5
#endif

//static const uint8_t row_pin_array[MATRIX_ROWS] = { PA0, PA1, PA2, PA3, PA4, PA5 };
//
//static const uint8_t column_pin_array[MATRIX_COLS] = { PA8, PB4, PB5, PB6, PB7, PB8, PB9, PB14,
//        PB15, PB16, PB17, PB18, PB19, PB20, PB21, PB22, PB23 };

static const uint8_t row_pin_array[MATRIX_ROWS] = { PA0, PA1, PA2, PA3, PA4, PA5, PA8, PB4, PB5,};

static const uint8_t column_pin_array[MATRIX_COLS] = { PB6, PB7, PB8, PB9, PB14,
        PB15, PB16 };

/* key combination for command */
#define IS_COMMAND() (keyboard_report->mods == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_RSHIFT)))

#define BACKLIGHT_LEVELS 10
/* for prototype */
//#define INFINITY_PROTOTYPE

/* Version 1.1a of the board */
//#define INFINITY_LED


/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

#endif
