#ifndef __RF_PAIR_H
#define __RF_PAIR_H

#include "CH58xBLE_LIB.h"
#include "rf_process.h"
#include "device_config.h"

#define RF_PAIR_TX_TIMEOUT_EVT              (1<<0)
#define RF_PAIR_RX_TIMEOUT_EVT              (1<<1)
#define RF_PAIR_TXPKT1_EVT                  (1<<2)
#define RF_PAIR_TXPKT2_EVT                  (1<<3)
#define RF_PAIR_TIMEOUT_EVT                 (1<<4)


#define PARI_PACKET1            0x51
#define PARI_PACKET2            0x52
typedef struct rf_pair_buf{
    uint8_t type;
    uint8_t addr[4];
} rf_pair_buf_t;

int pair_rx_process(rf_pair_buf_t *rxbuf);
int pair_tx_process(rf_pair_buf_t *rxbuf );
int start_pair(void);
void rf_pair_task_init(void);

#endif /* __RF_PAIR_H */
