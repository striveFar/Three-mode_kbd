/********************************** (C) COPYRIGHT *******************************
* File Name          : HAL.h
* Author             : WCH
* Version            : V1.0
* Date               : 2016/05/05
* Description        : 
*******************************************************************************/



/******************************************************************************/
#ifndef __HAL_H
#define __HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "RTC.h"
#include "SLEEP.h"	
#include "LED.h"
#include "KEY.h"
	
/* hal task Event */
#define   LED_BLINK_EVENT                               0x0001
#define   HAL_KEY_EVENT			                            0x0002
#define   HAL_REG_INIT_EVENT		                        0x2000
#define   HAL_SHUTDOWN_EVENT                                0x0008
#define   HAL_SLEEP_EVENT                                    0x0020
#define   HAL_TEST_EVENT		                            0x4000
#define   HAL_ADC_EVENT                                     0x0010

// hal sys_message
#define MESSAGE_UART                0xA0    // UART message
#define UART0_MESSAGE             (MESSAGE_UART|0 )    // UART0 message
#define UART1_MESSAGE             (MESSAGE_UART|1 )    // UART1 message

#define USB_MESSAGE              		0xB0    // USB message

#define KEY_MESSAGE                 0xaa
#define RF_PAIR_MESSAGE             0x55



#define LOW_POWER_SLEEP_TIME_S            MS1_TO_SYSTEM_TIME(1000*10)
#define LOW_POWER_SHUTDOWN_TIME_S         MS1_TO_SYSTEM_TIME(1000*60*5)

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern tmosTaskID halTaskID;

typedef struct  tag_uart_package
{
  tmos_event_hdr_t hdr;
  uint8            *pData;
} uartPacket_t;

typedef struct  SendMSG
{
  tmos_event_hdr_t hdr;
  void            *pData;
} SendMSG_t;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */
extern void HAL_Init( void );
extern tmosEvents HAL_ProcessEvent( tmosTaskID task_id, tmosEvents events );
extern void CH57X_BLEInit( void );
extern uint16 HAL_GetInterTempValue( void );
extern void Lib_Calibration_LSI( void );

uint8 OnBoard_SendMsg(uint8_t registeredTaskID, uint8 event, uint8 state, void *data);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
