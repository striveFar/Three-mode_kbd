#include <stdint.h>
#include "../../drivers/tmk_core/common/host.h"
#include "../../drivers/tmk_core/common/report.h"
#include "../../drivers/tmk_core/common/print.h"
#include "../../drivers/tmk_core/common/debug.h"
#include "../../drivers/tmk_core/common/host_driver.h"

static uint8_t bluefruit_keyboard_leds = 0;

static uint8_t keyboard_leds(void);
static void send_keyboard(report_keyboard_t *report);
static void send_mouse(report_mouse_t *report);
static void send_system(uint16_t data);
static void send_consumer(uint16_t data);

host_driver_t consumer_host_driver = {
        keyboard_leds,
        send_keyboard,
        send_mouse,
        send_system,
        send_consumer
};


static uint8_t keyboard_leds(void)
{
    return bluefruit_keyboard_leds;
}
static void send_keyboard(report_keyboard_t *report)
{

}
static void send_mouse(report_mouse_t *report)
{

}
static void send_system(uint16_t data)
{

}
static void send_consumer(uint16_t data)
{

}
