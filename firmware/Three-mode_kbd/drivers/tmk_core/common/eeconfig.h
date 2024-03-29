/*
Copyright 2013 Jun Wako <wakojun@gmail.com>

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

#ifndef EECONFIG_H
#define EECONFIG_H

#include <stdint.h>
#include <stdbool.h>


#define EECONFIG_MAGIC_NUMBER                       (uint16_t)0xFEED

/* eeprom parameteter address */
/*
#define EECONFIG_MAGIC                              (uint16_t *)0
#define EECONFIG_DEBUG                              (uint8_t *)2
#define EECONFIG_DEFAULT_LAYER                      (uint8_t *)3
#define EECONFIG_KEYMAP                             (uint8_t *)4
#define EECONFIG_MOUSEKEY_ACCEL                     (uint8_t *)5
#define EECONFIG_BACKLIGHT                          (uint8_t *)6
#define EECONFIG_AUDIO                              (uint8_t *)7
#define EECONFIG_RGBLIGHT                           (uint32_t *)8
#define EECONFIG_UNICODEMODE                        (uint8_t *)12
#define EECONFIG_STENOMODE                          (uint8_t *)13
// EEHANDS for two handed boards
#define EECONFIG_HANDEDNESS                         (uint8_t *)14
#define EECONFIG_OS_CYCLE                           (uint8_t *)15
#define EECONFIG_SEND_MODE                          (uint8_t *)16 // show to send keycode
#define EECONFIG_MAX EECONFIG_SEND_MODE
// Size of EEPROM being used, other code can refer to this for available EEPROM
#define EECONFIG_SIZE 17
*/

#define EECONFIG_MAGIC              		    "EECONFIG_MAGIC"               
#define EECONFIG_DEBUG              		    "EECONFIG_DEBUG"               
#define EECONFIG_DEFAULT_LAYER      		    "EECONFIG_DEFAULT_LAYER"       
#define EECONFIG_KEYMAP             		    "EECONFIG_KEYMAP"       
#define EECONFIG_MOUSEKEY_ACCEL     		    "EECONFIG_MOUSEKEY_ACCEL"
#define EECONFIG_BACKLIGHT          		    "EECONFIG_BACKLIGHT" 
#define EECONFIG_AUDIO              		    "EECONFIG_AUDIO" 
#define EECONFIG_RGBLIGHT           		    "EECONFIG_RGBLIGHT" 
#define EECONFIG_UNICODEMODE        		    "EECONFIG_UNICODEMODE" 
#define EECONFIG_STENOMODE          		    "EECONFIG_STENOMODE" 
// EEHANDS for two handed boards
#define EECONFIG_HANDEDNESS                         "EECONFIG_HANDEDNESS"
#define EECONFIG_OS_CYCLE                           "EECONFIG_OS_CYCLE" 
#define EECONFIG_SEND_MODE                          "EECONFIG_SEND_MODE"
#define EECONFIG_MAX EECONFIG_SEND_MODE             "EECONFIG_MAX EECONFIG_SEND_MODE"
// Size of EEPROM being used, other code can refer to this for available EEPROM
#define EECONFIG_SIZE 17
/* debug bit */
#define EECONFIG_DEBUG_ENABLE                       (1<<0)
#define EECONFIG_DEBUG_MATRIX                       (1<<1)
#define EECONFIG_DEBUG_KEYBOARD                     (1<<2)
#define EECONFIG_DEBUG_MOUSE                        (1<<3)

/* keyconf bit */
#define EECONFIG_KEYMAP_SWAP_CONTROL_CAPSLOCK       (1<<0)
#define EECONFIG_KEYMAP_CAPSLOCK_TO_CONTROL         (1<<1)
#define EECONFIG_KEYMAP_SWAP_LALT_LGUI              (1<<2)
#define EECONFIG_KEYMAP_SWAP_RALT_RGUI              (1<<3)
#define EECONFIG_KEYMAP_NO_GUI                      (1<<4)
#define EECONFIG_KEYMAP_SWAP_GRAVE_ESC              (1<<5)
#define EECONFIG_KEYMAP_SWAP_BACKSLASH_BACKSPACE    (1<<6)
#define EECONFIG_KEYMAP_NKRO                        (1<<7)


bool eeconfig_is_enabled(void);

void eeconfig_init(void);

void eeconfig_enable(void);

void eeconfig_disable(void);

uint8_t eeconfig_read_debug(void);
void eeconfig_update_debug(uint8_t val);

uint8_t eeconfig_read_default_layer(void);
void eeconfig_update_default_layer(uint8_t val);

uint8_t eeconfig_read_keymap(void);
void eeconfig_update_keymap(uint8_t val);

#ifdef BACKLIGHT_ENABLE
uint8_t eeconfig_read_backlight(void);
void eeconfig_update_backlight(uint8_t val);
#endif

#ifdef AUDIO_ENABLE
uint8_t eeconfig_read_audio(void);
void eeconfig_update_audio(uint8_t val);
#endif

/* 操作系统周期 */
#ifdef OS_CYCLE
uint8_t eeconfig_read_os(void);
void eeconfig_update_os(uint8_t val);
#endif


uint8_t eeconfig_read_send_mode(void);

void eeconfig_update_send_mode(uint8_t val);

#endif
