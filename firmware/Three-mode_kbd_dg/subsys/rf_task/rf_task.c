#include <rf_task/rf_task.h>
#include "RF_PHY/rf_process.h"
#include "CH58xBLE_LIB.h"
#include "HAL.h"
#include "USB_task/USB_kbd.h"
#include "RingBuffer/lwrb.h"

tmosTaskID RFtaskID = INVALID_TASK_ID;

void rf_read_deal(uint8_t *data, uint8_t len);

struct rf_process_cb rf_process_cbs = {
        .tx_respond_deal_cb = NULL,
        .rx_read_deal_cb = rf_read_deal,
};


void rf_read_deal(uint8_t *data, uint8_t len)
{
    rf_data_t *tempbuf = (rf_data_t *)data;

    size_t wl = lwrb_write(&KEY_buff, &tempbuf->report_id, tempbuf->len);
    if(wl == tempbuf->len){
        OnBoard_SendMsg(USBTaskID, KEY_MESSAGE, 1, NULL);
    } else{
        lwrb_skip(&KEY_buff, wl);
    }
}

static void RF_ProcessTMOSMsg(tmos_event_hdr_t *pMsg) {
    switch (pMsg->event) {

    default:
        break;
    }
}

uint16 rf_task_ProcessEvent(uint8 task_id, uint16 events) {
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

    return 0;
}


void rf_task_init(void)
{
    RFtaskID = TMOS_ProcessEventRegister(rf_task_ProcessEvent);
    rf_process_init();
    rf_pro_RegisterCB(&rf_process_cbs);
}
