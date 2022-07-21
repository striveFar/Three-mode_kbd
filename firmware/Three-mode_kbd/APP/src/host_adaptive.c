#include <stdint.h>
#include "../../drivers/tmk_core/common/host_driver.h"
#include "../../drivers/tmk_core/common/host.h"
#include "../../drivers/tmk_core/common/report.h"
#include "../../drivers/tmk_core/common/print.h"
#include "../../drivers/tmk_core/common/debug.h"
#include "../subsys/USB_task/USB_kbd.h"
#include "../subsys/RF_task/rf_task.h"
#include "../subsys/BLE/hidkbd.h"
#include "../subsys/HAL/HAL.h"
#include "RingBuffer/lwrb.h"
#include "device_config.h"

static uint8_t bluefruit_keyboard_leds = 0;

static uint8_t keyboard_leds(void);
static void send_keyboard(report_keyboard_t *report);
static void send_mouse(report_mouse_t *report);
static void send_system(uint16_t data);
static void send_consumer(uint16_t data);

host_driver_t consumer_host_driver = { keyboard_leds, send_keyboard, send_mouse,
        send_system, send_consumer };

/* TODO:实现基本功能 */
static uint8_t keyboard_leds(void) {
    return bluefruit_keyboard_leds;
}
static void send_keyboard(report_keyboard_t *report) {
    size_t wl = lwrb_write(&KEY_buff, report,
            sizeof(report_keyboard_t));
    if (wl != sizeof(report_keyboard_t))
        lwrb_skip(&KEY_buff, wl);
    switch (device_mode) {
    case MODE_BLE:
        OnBoard_SendMsg(hidEmuTaskId, KEY_MESSAGE, 1, NULL);
        break;

    case MODE_RF24:
        OnBoard_SendMsg(RFtaskID, KEY_MESSAGE, 1, NULL);
        break;

    case MODE_USB:
        OnBoard_SendMsg(USBTaskID, KEY_MESSAGE, 1, NULL);
        break;
    default:
        break;
    }
}
static void send_mouse(report_mouse_t *report) {

}
static void send_system(uint16_t data) {

}
static void send_consumer(uint16_t data) {

}
