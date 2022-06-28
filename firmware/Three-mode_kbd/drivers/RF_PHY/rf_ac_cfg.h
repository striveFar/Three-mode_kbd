#ifndef __RF_AC_CFG_H
#define __RF_AC_CFG_H

#include "CH58xBLE_LIB.h"

#define RECVREG                       (*((PUINT32V)(0x4000C100+0x38)))
#define RECVCMPSTA                    (RECVREG&BV(1))
#define RECVCMPSTACLR                 (RECVREG &= ~(BV(1)))


#define EN_SLEEP     100           //睡眠时的链接间隔                                                                      单位ms

#define DIS_SLEEP      1             //不睡眠时的链接间隔 (实际链接间隔在此基础上+0.5)  单位ms

#define TX_ONCETIME                   DIS_SLEEP         //发送间隔  最小值4ms   上限不定
#define RX_TIMEOUT                    200


#define RFIDLE        0
#define RFTX          1
#define RFRX          2

#define SYSTOMS(fsys)               (fsys/1000)
#define SYSTOUS(fsys)               (SYSTOMS(fsys)/1000)
#define RECVTIME(len,fsys)          (len*8*SYSTOUS(fsys))    //1M PHY  1bit:1us
#define AHEADTIME(fsys)             (SYSTOUS(fsys)*(100+200)) //100:为硬件启动发送时间  200:提前打开接受的时间

#endif /* __RF_AC_CFG_H */
