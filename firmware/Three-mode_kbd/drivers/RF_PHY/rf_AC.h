#ifndef __RF_AC_H
#define __RF_AC_H

#include <stdbool.h>
#include "config.h"

#define RF_AC_RCV_END_EVT       0x0001
#define RF_AC_TX_END_EVT        0x0002
#define RF_AC_RCV_EVT           0x0004
#define RF_AC_SEND_EVT          0x0008

struct rf_AC_cb{
  void (* rf_AC_TxResp)(uint8_t *data, size_t len);
  void (* rf_AC_Rx)(uint8_t *data, size_t len);
  void(* read_rssi)(char rssi);
};

typedef struct rf_ac_data {
  uint8_t data[200];
  uint8_t crc;
}rf_ac_data_t;



void rf_tx_process(void);

/* 设置发送数据 */
void rf_set_txdata( uint8_t *data, uint8_t index, uint8_t len);

void rf_clear_txdata(void);

/* 设置接收模式应答数据 */
void rf_set_rxrespdata( uint8_t *data, uint8_t index, size_t len );

int rf_ACInit(rfConfig_t* cfg);
void rf_AC( uint8_t *data, uint8_t crc, bool flag );
void AC_timesync(size_t len);

void rf_AC_RegisterCB(struct rf_AC_cb *cb);

void txWakeUp(uint32_t *time);
#endif
