#ifndef __RF_TASK_H
#define __RF_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "CONFIG.h"
#include "../../../drivers/tmk_core/common/report.h"

#define PAIR_EVENT                  0x0001
#define RF_SEND_EVENT               0x0002

#define RF_SLEEP_EVENT              0x0010
#define RF_TEST_EVENT               0x0020   //rf�����¼����Զ���ԣ�Ȼ��ÿ��10ms��һ�οհ�����ֻ��ID������ͣ����ݶ�Ϊ��

typedef struct __attribute__((__packed__))
{
    uint8_t len;
    uint8_t report_id;
    uint8_t data[KEYBOARD_REPORT_SIZE]; //use the max len
}rf_data_t;

extern tmosTaskID RFtaskID;

void RFtaskInit( void );


#ifdef __cplusplus
}
#endif
#endif

