#include "CONFIG.h"
#include "RingBuffer/lwrb.h"
#include "device_config.h"
#include "CH58x_common.h"
#include "HAL.h"
#include "rf_task.h"
#include "USB/usbuser.h"
#include "backlight/backlight.h"
#include "RF_PHY/rf_pair.h"
#include "RF_PHY/rf_process.h"
#include "../../../drivers/tmk_core/common/report.h"

tmosTaskID RFtaskID;

void tx_respond(uint8_t *buf, uint8_t len);
void tx_send_failed(void);

static rf_data_t send_buf = {
        .len = 0,  //report id + 数据长度
        .report_id = 0xff,
        .data = {0}
};

struct rf_process_cb rf_process_cbs = {
        .tx_respond_deal_cb = tx_respond,
        .tx_send_failed_cb = tx_send_failed,
        .rx_read_deal_cb = NULL,
};

void tx_send_failed(void)
{
    PRINT("send failed!\n");
    tmos_start_task(RFtaskID, RF_SEND_EVENT, MS1_TO_SYSTEM_TIME(1));
}


void tx_respond(uint8_t *buf, uint8_t len)
{
    rf_data_t *tempbuf = (rf_data_t *)buf;

    if(unlikely(tempbuf->len==1)) {
        (tempbuf->data[0] & 0x01 )?set_led_num(1):set_led_num(0);
        (tempbuf->data[0] & 0x02 )?set_led_cap(1):set_led_cap(0);
        (tempbuf->data[0] & 0x04 )?set_led_scr(1):set_led_scr(0);
        LOG_INFO("get led: %#x", tempbuf->data[0]);
    }

    lwrb_skip(&KEY_buff, send_buf.len + 1);
}

int rf_tx_deal(void)
{
    if (lwrb_get_full(&KEY_buff) == 0)
        return -1;

    struct rf_status_t *sta = get_rf_status();

    if(sta->dongle_lost) {
        PRINT("dongle lost, reset lwrb\n");
        lwrb_reset(&KEY_buff);
        return -1;
    }

    uint8_t report_id, key_len;

    lwrb_peek(&KEY_buff, 0, &report_id, 1);

    if(report_id == KEYNORMAL_ID || report_id == KEYBIT_ID) {
        key_len= KEYBOARD_REPORT_SIZE;
    } else if (report_id == SYS_ID) {
        key_len = sizeof(uint16_t);
    } else if(report_id == CONSUME_ID){
        key_len = sizeof(uint16_t);
    } else if(report_id == VENDOR_ID){
//        key_len = 2;
    } else if(report_id == MOUSE_ID){
        key_len = sizeof(report_mouse_t);
    } else {
        /* should not be here */
        LOG_DEBUG("report id error!");
        lwrb_reset(&KEY_buff);
    }

    lwrb_peek(&KEY_buff, 1, send_buf.data, key_len); //skip when get TX_RX

    send_buf.report_id = report_id;
    send_buf.len = key_len + sizeof(send_buf.report_id);

    rf_send((uint8_t *)&send_buf, sizeof(send_buf) - sizeof(send_buf.data) + key_len, false);

    return send_buf.len;
}



static void RF_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch (pMsg->event) {
    case KEY_MESSAGE:{
        SendMSG_t *msg = (SendMSG_t *)pMsg;
        msg->hdr.status?tmos_set_event(RFtaskID, RF_SEND_EVENT):0;
    }  break;

    case RF_PAIR_MESSAGE:{
        SendMSG_t *msg = (SendMSG_t *)pMsg;
        msg->hdr.status?tmos_set_event(RFtaskID, PAIR_EVENT):0;
    }break;

    default:
        break;
    }
}

uint16 RF_task_Event(uint8 task_id, uint16 events)
{
    if (events & SYS_EVENT_MSG) {
        uint8 *pMsg;

        if ((pMsg = tmos_msg_receive(task_id)) != NULL) {

            RF_ProcessTMOSMsg((tmos_event_hdr_t *) pMsg);
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }


    if (events & PAIR_EVENT) {
#ifdef CONFIG_RF_PAIR
        int ret = start_pair();
        PRINT("ret = %d\n", ret);
#endif
        return events ^ PAIR_EVENT;
    }

    if (events & RF_SEND_EVENT) {
        rf_tx_deal();
        return events ^ RF_SEND_EVENT;
    }


    return 0;
}


void RFtaskInit(void)
{

    RFtaskID = TMOS_ProcessEventRegister(RF_task_Event);

    rf_process_init();
    rf_pro_RegisterCB(&rf_process_cbs);

//    tmos_start_task(RFtaskID, PAIR_EVENT, MS1_TO_SYSTEM_TIME(10));
}

