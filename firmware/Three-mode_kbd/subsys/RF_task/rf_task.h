#ifndef __RF_TASK_H
#define __RF_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "CONFIG.h"

#define PAIR_EVENT                  0x0001
#define RF_SEND_EVENT               0x0002

#define RF_SLEEP_EVENT              0x0010
#define RF_TEST_EVENT               0x0020   //rf测试事件，自动配对，然后每隔10ms发一次空包，即只发ID与包类型，数据段为空

typedef struct __attribute__((__packed__))
{
    uint8_t len;
    uint8_t report_id;
    uint8_t data[16];
}rf_data_t;

extern tmosTaskID RFtaskID;

void RFtaskInit( void );


#ifdef __cplusplus
}
#endif
#endif

