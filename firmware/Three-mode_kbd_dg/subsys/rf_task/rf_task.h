#ifndef __RF_TASK_H
#define __RF_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "CONFIG.h"






typedef struct __attribute__((__packed__))
{
    uint8_t len;
    uint8_t report_id;
    uint8_t data[16];
}rf_data_t;



extern tmosTaskID RFtaskID;
void rf_task_init(void);


#endif /* __RF_TASK_H */
