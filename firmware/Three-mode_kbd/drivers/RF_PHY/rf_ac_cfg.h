#ifndef __RF_AC_CFG_H
#define __RF_AC_CFG_H

#include "CH58xBLE_LIB.h"

#define RECVREG                       (*((PUINT32V)(0x4000C100+0x38)))
#define RECVCMPSTA                    (RECVREG&BV(1))
#define RECVCMPSTACLR                 (RECVREG &= ~(BV(1)))


#define EN_SLEEP     100           //˯��ʱ�����Ӽ��                                                                      ��λms

#define DIS_SLEEP      1             //��˯��ʱ�����Ӽ�� (ʵ�����Ӽ���ڴ˻�����+0.5)  ��λms

#define TX_ONCETIME                   DIS_SLEEP         //���ͼ��  ��Сֵ4ms   ���޲���
#define RX_TIMEOUT                    200


#define RFIDLE        0
#define RFTX          1
#define RFRX          2

#define SYSTOMS(fsys)               (fsys/1000)
#define SYSTOUS(fsys)               (SYSTOMS(fsys)/1000)
#define RECVTIME(len,fsys)          (len*8*SYSTOUS(fsys))    //1M PHY  1bit:1us
#define AHEADTIME(fsys)             (SYSTOUS(fsys)*(100+200)) //100:ΪӲ����������ʱ��  200:��ǰ�򿪽��ܵ�ʱ��

#endif /* __RF_AC_CFG_H */
