#include "backlight.h"
#include "CH58x_common.h"
#include "device_config.h"

static int led_level;

#if I2C_LED
#include "I2C/myi2c.h"
void set_led_num(bool s) {
    myi2c_beginTransmission(/*device address*/);     // Initialize the Tx buffer
    myi2c_write(/*data*/);// Put slave register address in Tx buffer

    myi2c_endTransmission(true);
}

void set_led_cap(bool s) {
    myi2c_beginTransmission(/*device address*/);     // Initialize the Tx buffer
    myi2c_write(/*data*/);// Put slave register address in Tx buffer

    myi2c_endTransmission(true);
}

void set_led_scr(bool s) {
    myi2c_beginTransmission(/*device address*/);     // Initialize the Tx buffer
    myi2c_write(/*data*/);// Put slave register address in Tx buffer

    myi2c_endTransmission(true);
}

void set_bk(uint8_t line, bk_levels_t level) {
    myi2c_beginTransmission(/*device address*/);     // Initialize the Tx buffer
    myi2c_write(/*data*/);// Put slave register address in Tx buffer

    myi2c_endTransmission(true);
}

void disbale_bk(uint8_t line) {
    myi2c_beginTransmission(/*device address*/);     // Initialize the Tx buffer
    myi2c_write(/*data*/);// Put slave register address in Tx buffer

    myi2c_endTransmission(true);
}

void bkinit(void) {
    myi2c_Init();

    myi2c_beginTransmission(/*device address*/);     // Initialize the Tx buffer
    myi2c_write(/*data*/);// Put slave register address in Tx buffer

    myi2c_endTransmission(true);
}

#else

void set_led_num(bool s) {
    (s) ? GPIOB_ResetBits(GPIO_Pin_13) : GPIOB_SetBits(GPIO_Pin_13);
}

void set_led_cap(bool s) {
    (s) ? GPIOA_ResetBits(GPIO_Pin_12) : GPIOA_SetBits(GPIO_Pin_12);
}

void set_led_scr(bool s) {
    (s) ? GPIOB_ResetBits(GPIO_Pin_12) : GPIOB_SetBits(GPIO_Pin_12);
}

void set_bk(uint8_t line, bk_levels_t level) {
    PWMX_ACTOUT(line & BK_LINEALL, level, Low_Level, ENABLE);
}

void disbale_bk(uint8_t line) {
    PWMX_ACTOUT(line & BK_LINEALL, 0, Low_Level, DISABLE);
}

void bkinit(void) {
    GPIOPinRemap(ENABLE, RB_PIN_PWMX);

    //����״̬������
    GPIOA_ResetBits(LED_CAP);
    GPIOB_ResetBits(LED_SCR | LED_NUM);
    GPIOA_ModeCfg(LED_CAP, GPIO_ModeOut_PP_5mA);
    GPIOB_ModeCfg( LED_SCR | LED_NUM, GPIO_ModeOut_PP_5mA);

    //���������
    GPIOA_SetBits(LR0 | LR1);
    GPIOB_SetBits(LR2 | LR3 | LR4 | LR5);
    GPIOA_ModeCfg(LR0 | LR1, GPIO_ModeOut_PP_5mA);
    GPIOB_ModeCfg( LR2 | LR3 | LR4 | LR5, GPIO_ModeOut_PP_5mA);

    PWMX_CLKCfg(4);                   // cycle = 4/Fsys
    PWMX_CycleCfg(PWMX_Cycle_64);     // ���� = 64*cycle

    led_level = device_led.led_level;

    if (!device_led.led_en)
        disbale_bk(BK_LINEALL);
}
#endif

bool enhance_bk(uint8_t line) {
    led_level += BK_STEP;
    if (led_level > level_8) {
        led_level = level_8;
        return false;
    }
    set_bk(line, led_level);
    device_led.led_level = led_level;
    return true;
}

bool weaken_bk(uint8_t line) {
    led_level -= BK_STEP;
    if (led_level < level_0) {
        led_level = level_0;
        return false;
    }
    set_bk(line, led_level);
    device_led.led_level = led_level;
    return true;
}
