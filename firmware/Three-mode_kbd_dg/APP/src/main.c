/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.1
* Date               : 2020/08/06
* Description        : 外设从机应用主函数及任务系统初始化
*******************************************************************************/


/******************************************************************************/
/* 头文件包含 */
#include "CH58x_common.h"
#include "HAL/config.h"
#include "HAL_FLASH/include/easyflash.h"
#include "device_config.h"
#include "RingBuffer/lwrb.h"
#include "device_config.h"
#include "RF_PHY/rf_AC.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE/4];

const uint8_t MacAddr[6] = {0x84,0xC2,0xE4,0x03,0x33,0x44};

/*******************************************************************************
* Function Name  : Main_Circulation
* Description    : 主循环
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
__attribute__((section(".highcode")))
void Main_Circulation()
{
  while(1){
    rf_tx_process();
    TMOS_SystemProcess( );
  }
}

/*******************************************************************************
* Function Name  : main
* Description    : 主函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main( void ) 
{
#if (defined (DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
  PWR_DCDCCfg( ENABLE );
#endif

  SystemInit();
  GPIOA_ModeCfg( GPIO_Pin_All, GPIO_ModeIN_PD );
  GPIOB_ModeCfg( GPIO_Pin_All, GPIO_ModeIN_PD );

#ifdef DEBUG
  GPIOA_SetBits(bTXD1);
  GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
  UART1_DefInit();
#endif
  DEBUG_Init();

  /*DataFlash初始化*/
  if( easyflash_init() != SUCCESS )
  {
    LOG_INFO("Date Flash init error!");
  }
  ef_print_env();

  if(GPIOB_ReadPortPin(GPIO_Pin_10) ){  //或者由上位机发送指令
      DelayMs(100);
      if(GPIOB_ReadPortPin(GPIO_Pin_10)){
          ResetDeviceInfo("all");
      }
  }
  
  ReadDeviceInfo("all");

  if(GPIOB_ReadPortPin(GPIO_Pin_4) ){  //或者由上位机发送指令
      DelayMs(100);
      if(GPIOB_ReadPortPin(GPIO_Pin_4)){
          device_mode = MODE_TSET;
      }
  }

  ring_buffer_init();
  Mode_Init(device_mode);

  Main_Circulation();
}


/******************************** endfile @ main ******************************/
