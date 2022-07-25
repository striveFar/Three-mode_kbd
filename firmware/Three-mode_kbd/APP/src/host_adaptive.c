#include <stdint.h>
#include "../../drivers/tmk_core/common/host_driver.h"
#include "../../drivers/tmk_core/common/host.h"
#include "../../drivers/tmk_core/common/report.h"
#include "../../drivers/tmk_core/common/print.h"
#include "../../drivers/tmk_core/common/debug.h"
#include "../subsys/USB_task/USB_kbd.h"
#include "../subsys/RF_task/rf_task.h"
#include "../drivers/USB/usbuser.h"
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

static void send_mode_select(uint8_t cur_device_mode) {
    switch (cur_device_mode) {
        case MODE_BLE:
            OnBoard_SendMsg(hidEmuTaskId, KEY_MESSAGE, notify, NULL);
            break;

        case MODE_RF24:
            OnBoard_SendMsg(RFtaskID, KEY_MESSAGE, notify, NULL);
            break;

        case MODE_USB:
            OnBoard_SendMsg(USBTaskID, KEY_MESSAGE, notify, NULL);
            break;
        default:
            break;
        }
}

/* TODO:实现基本功能 */
static uint8_t keyboard_leds(void) {
    return bluefruit_keyboard_leds;
}
static void send_keyboard(report_keyboard_t *report) {

    size_t wl = 0;
#if defined(NKRO_ENABLE) || defined(NKRO_6KRO_ENABLE)
    uint8_t report_id = KEYNORMAL_ID;
#else
    uint8_t report_id = KEYBIT_ID;
#endif

    wl = lwrb_write(&KEY_buff, &report_id, sizeof(report_id));
    if (wl != sizeof(report_id)) {
        lwrb_skip(&KEY_buff, wl);
        LOG_INFO("send_keyboard: report_id write error!");
    }

    wl = lwrb_write(&KEY_buff, report, sizeof(report));
    if (wl != sizeof(report)) {
        lwrb_skip(&KEY_buff, wl);
        LOG_INFO("send_keyboard: :report_keyboard_t write error!");
    }
    send_mode_select(device_mode);
}

static void send_mouse(report_mouse_t *report) {

    size_t wl = 0;
    uint8_t report_id = MOUSE_ID;
    wl = lwrb_write(&KEY_buff, &report_id, sizeof(report_id));
    if (wl != sizeof(report_id)) {
        lwrb_skip(&KEY_buff, wl);
        LOG_INFO("send_mouse: report_id write error!");
    }

    wl = lwrb_write(&KEY_buff, report, sizeof(report));
    if (wl != sizeof(report)) {
        lwrb_skip(&KEY_buff, wl);
        LOG_INFO("send_mouse: report_mouse_t write error!");
    }
    send_mode_select(device_mode);
}

static void send_system(uint16_t data) {
    size_t wl = 0;
    uint8_t report_id = SYS_ID;
    wl = lwrb_write(&KEY_buff, &report_id, sizeof(report_id));
    if (wl != sizeof(report_id)) {
        lwrb_skip(&KEY_buff, wl);
        LOG_INFO("send_system: report_id write error!");
    }

    wl = lwrb_write(&KEY_buff, &data, sizeof(data));
    if (wl != sizeof(data)) {
        lwrb_skip(&KEY_buff, wl);
        LOG_INFO("send_system: :system data write error!");
    }
    send_mode_select(device_mode);
}

static void send_consumer(uint16_t data) {
    size_t wl = 0;
    uint8_t report_id = CONSUME_ID;
    wl = lwrb_write(&KEY_buff, &report_id, sizeof(report_id));
    if (wl != sizeof(report_id)) {
        lwrb_skip(&KEY_buff, wl);
        LOG_INFO("send_consumer: report_id write error!");
    }

    wl = lwrb_write(&KEY_buff, &data, sizeof(data));
    if (wl != sizeof(data)) {
        lwrb_skip(&KEY_buff, wl);
        LOG_INFO("send_consumer: consumer data write error!");
    }
    send_mode_select(device_mode);
}
