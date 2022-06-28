#include <stdbool.h>

#include "rf_pair.h"
#include "CH58x_common.h"
#include "rf_process.h"
#include "config.h"
#include "rf_AC.h"

#ifdef CONFIG_RF_PAIR

#ifdef CONFIG_RF_PAIR_DEBUG
#define RF_PAIR_DBG  PRINT
#else
#define RF_PAIR_DBG
#endif
uint8_t rf_pair_task = 0;

static uint8_t pair_tx_step = 0;
static uint8_t pair_rx_step = 0;
static uint8_t Mac_addr[6] = {0};
static uint8_t rcv_addr[4] = {0};

void pair_set_resp(void)
{
    rf_pair_buf_t resp = {0};
    tmos_memcpy(resp.addr, rcv_addr, sizeof(uint32_t));
    rf_set_rxrespdata((uint8_t *)&resp, 5, sizeof(rf_pair_buf_t));
}

uint8_t pair_success(uint32_t addr)  //TODO: 可以改变算法，让MAC地址更随机
{
    myrfConfig.accessAddress = addr;
    RF_Config(&myrfConfig);

    device_pair.Pair_flag = true;
    device_pair.Mac_addr = addr;
    RF_PAIR_DBG("pair success: %#x\n", addr);
    rf_rx_reset();

    uint8_t ret ;
    tmos_stop_task(rf_pair_task, RF_PAIR_TIMEOUT_EVT);

#ifdef CONFIG_RF_HOP
    rf_start_hop();
#endif
    ret = SaveDeviceInfo("device_pair");
    return ret;
}

void pair_failed(void)
{
    ReadDeviceInfo("device_pair");
    myrfConfig.accessAddress = device_pair.Mac_addr;
    PRINT("access addr %#x\n", myrfConfig.accessAddress);
    uint8_t ret = RF_Config(&myrfConfig);
    PRINT("ret: %d\n", ret);
    rf_rx_reset();
#ifdef CONFIG_RF_HOP
    rf_start_hop();
#endif
}

int pair_rx_process(rf_pair_buf_t *rxbuf)
{
    int ret = -1;
    if(device_pair.Pair_flag)
        return ret;

    switch(pair_rx_step){
    case 0:
        RF_PAIR_DBG("pair step 0");
        if(rxbuf->type == PARI_PACKET1){

            tmos_memcpy(rcv_addr, rxbuf->addr, sizeof(uint32_t));

            pair_set_resp();

            pair_rx_step++;
            tmos_start_task(rf_pair_task, RF_PAIR_RX_TIMEOUT_EVT, MS1_TO_SYSTEM_TIME(200)); //200ms内无法进入下一阶段超时
            ret = 1;
        }break ;

    case 1:
        RF_PAIR_DBG("pair step 1");
        if(rxbuf->type == PARI_PACKET2){
            uint32_t temp = 0;
            temp |= rcv_addr[0]<<0;
            temp |= rcv_addr[1]<<8;
            temp |= rcv_addr[2]<<16;
            temp |= rcv_addr[3]<<24;

            pair_rx_step = pair_success(temp) == SUCCESS?0:1;
            tmos_stop_task(rf_pair_task, RF_PAIR_RX_TIMEOUT_EVT);
            ret = 0;

        } else{
            pair_set_resp();
            ret = 2;
        }
        break;
    }
    return ret;
}

uint16_t RF_Pair_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if (events & SYS_EVENT_MSG) {
        uint8 *pMsg;

        if ((pMsg = tmos_msg_receive(task_id)) != NULL) {

            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & RF_PAIR_RX_TIMEOUT_EVT){
        pair_rx_step = 0;
        pair_failed();

        RF_PAIR_DBG("pait_rx timeout!");
        return (events ^ RF_PAIR_RX_TIMEOUT_EVT);
    }


    if(events & RF_PAIR_TIMEOUT_EVT){
       pair_failed();

#ifdef CONFIG_RF_HOP
      tmos_set_event( rf_pair_task , SBP_RF_CHANNEL_HOP_RX_EVT );
#endif
        return (events ^ RF_PAIR_TIMEOUT_EVT);
    }

    return 0;
}

int start_pair(void)
{
    rf_pair_buf_t temp = {0};
    RF_PAIR_DBG("start pair\n");
#ifdef CONFIG_RF_HOP
    rf_shut_hop();
#endif
    device_pair.Pair_flag = 0; //FIXME: if pair faild, need to restart to communicate to old dongle

    tmos_start_task(rf_pair_task, RF_PAIR_TIMEOUT_EVT, MS1_TO_SYSTEM_TIME(1000 * 10));
    myrfConfig.accessAddress = DEFAULT_MAC;

    RF_PAIR_DBG("access addr %#x\n", myrfConfig.accessAddress);
    if(RF_Config(&myrfConfig)){
        rf_rx_reset();
        return -1;
    }

    return 0;
}


void rf_pair_task_init(void)
{
    rf_pair_task = TMOS_ProcessEventRegister(RF_Pair_ProcessEvent);

#if ((defined (BLE_MAC)) && (BLE_MAC == TRUE))
       for ( int i = 0; i < 6; i++ )
           Mac_addr[i] = MacAddr[5 - i];
#else
       GetMACAddress( Mac_addr );
#endif
       RF_PAIR_DBG("got MAC=[");
       for(int i = 0; i < 6; i++) {
           if(i) RF_PAIR_DBG(" ");
           RF_PAIR_DBG("%#x", Mac_addr[i]);
       }RF_PAIR_DBG("]\n");
}

#endif /* CONFGI_RF_PAIR */
