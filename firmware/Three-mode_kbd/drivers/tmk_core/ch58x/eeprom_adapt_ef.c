#include <stdint.h>
#include <string.h>
#include "eeprom.h"
#include "HAL_FLASH/include/easyflash.h"
uint8_t eeprom_read_byte(const uint8_t *__p) {
    uint8_t ret = 0;
    const char * key = (const char *) __p;
    ef_get_env_blob(key, &ret, sizeof(uint8_t), NULL);
    return ret;
}
uint16_t eeprom_read_word(const uint16_t *__p) {
    uint16_t ret = 0;
    const char * key = (const char *) __p;
    ef_get_env_blob(key, &ret, sizeof(uint16_t), NULL);
    return ret;
}
uint32_t eeprom_read_dword(const uint32_t *__p) {
    uint32_t ret = 0;
    const char * key = (const char *) __p;
    ef_get_env_blob(key, &ret, sizeof(uint32_t), NULL);
    return ret;
}
/* 从 __src 开始读，因此 __src 是 key*/
void eeprom_read_block(void *__dst, const void *__src, uint32_t __n) {
    const char * key = (const char *) __src;
    ef_get_env_blob(key, __dst, __n, NULL);
}
void eeprom_write_byte(uint8_t *__p, uint8_t __value) {
    const char * key = (const char *) __p;
    ef_set_env_blob(key, &__value, sizeof(__value));
}
void eeprom_write_word(uint16_t *__p, uint16_t __value) {
    const char * key = (const char *) __p;
    ef_set_env_blob(key, &__value, sizeof(__value));
}
void eeprom_write_dword(uint32_t *__p, uint32_t __value) {
    const char * key = (const char *) __p;
    ef_set_env_blob(key, &__value, sizeof(__value));
}
void eeprom_write_block(const void *__src, void *__dst, uint32_t __n) {
    const char * key = (const char *) __dst;
    ef_set_env_blob(key, __src, __n);
}
void eeprom_update_byte(uint8_t *__p, uint8_t __value) {
    const char * key = (const char *) __p;
    uint8_t orig = eeprom_read_byte(key);
    if (orig != __value) {
        eeprom_write_byte((uint8_t *) key, __value);
    }
}
void eeprom_update_word(uint16_t *__p, uint16_t __value) {
    uint16_t orig = eeprom_read_word(__p);
    if (orig != __value) {
        eeprom_write_word(__p, __value);
    }
}
void eeprom_update_dword(uint32_t *__p, uint32_t __value) {
    uint32_t orig = eeprom_read_dword(__p);
    if (orig != __value) {
        eeprom_write_dword(__p, __value);
    }
}
void eeprom_update_block(const void *__src, void *__dst, uint32_t __n) {
    uint8_t read_buf[__n ];
    eeprom_read_block(read_buf, __dst, __n);
    if (memcmp(__src, read_buf, __n) != 0) {
        eeprom_write_block(__src, __dst, __n);
    }
}
