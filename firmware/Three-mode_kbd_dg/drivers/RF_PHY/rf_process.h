/********************************** (C) COPYRIGHT *******************************
* File Name          : rf_phy.h
* Author             : WCH
* Version            : V1.0
* Date               : 2018/11/12
* Description        : 
*******************************************************************************/

#ifndef __RF_PROCESS_H
#define __RF_PROCESS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "CH58xBLE_LIB.h"
#include "rf_ac_cfg.h"

#define SBP_RF_TX_TIMEOUT_EVT                   (1<<0)
#define SBP_RF_DGLOSS_TIMEOUT_EVT               (1<<1)
#define SBP_RF_CHANNEL_HOP_TX_EVT               (1<<4)
#define SBP_RF_CHANNEL_HOP_RX_EVT               (1<<5)

#define RF_MSG_READ_EVT                     0x3e

#define RF_RESEND_NUM_MAX                   5
#define RF_TX_TIMEOUT_MS                    TX_ONCETIME*2

#define BUFFER_LENGTH     512
#define MAGIC_WORD                  0x4B434652

struct rf_process_cb{
    void (* tx_respond_deal_cb)(uint8_t *data, uint8_t len);
    void (* rx_read_deal_cb)(uint8_t *data, uint8_t len);
    void (* tx_send_failed_cb)(void);
};

struct rf_status_t {
    uint8_t idle : 1;
    uint8_t sending :1;
    uint8_t receiving : 1;
    uint8_t paired : 1;
    uint8_t dongle_lost : 1;
    uint8_t error : 1;
};

typedef struct __attribute__((__packed__)) rf_buf
{
  uint8_t num;
  uint32_t magicword;
  uint8_t data[20];
}rf_buf_t;

extern rfConfig_t myrfConfig;
extern uint8_t rf_process_taskID;

void rf_process_init( void );
void rf_readtask_register(uint8_t taksid);
int rf_rx_available(void) ;
int rf_peek(uint8_t *data, size_t len);
int rf_read(uint8_t *data ,size_t len);
bool rf_send(uint8_t *data, size_t len, bool force);
void rf_pro_RegisterCB( struct rf_process_cb *cb );
struct rf_status_t *get_rf_status(void);
void set_rf_status(struct rf_status_t *sta);
void rf_shut_hop(void);
void rf_start_hop(void);

#ifdef __cplusplus
}
#endif

#endif /* __RF_PROCESS_H */
