/********************************** (C) COPYRIGHT *******************************
* File Name          : hidkbd.h
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/10
* Description        : 
*******************************************************************************/

#ifndef HIDKBD_H
#define HIDKBD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "CH58xBLE_LIB.h"

/*********************************************************************
 * CONSTANTS
 */

// Task Events
#define START_DEVICE_EVT                              0x0001
#define START_REPORT_EVT                              0x0002
#define START_PARAM_UPDATE_EVT                        0x0004
#define BLE_SEND_DATA_EVT                             0x0008
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */
  
/*********************************************************************
 * GLOBAL VARIABLES
 */
extern size_t hidEmu_Mtu;
extern uint16 hidEmuConnHandle;
extern uint8 hidEmuTaskId;
/*
 * Task Initialization for the BLE Application
 */
extern void HidEmu_Init( void );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 HidEmu_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 
