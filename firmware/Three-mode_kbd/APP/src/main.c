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
#include "config.h"

#include "RF_PHY/rf_AC.h"
#include "I2C/myi2c.h"


bool isUSBinsert = false;
/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4)))   u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

uint8_t MacAddr[6] = { 0x84, 0xC2, 0xE4, 0x13, 0x33, 0x44 };

/*******************************************************************************
 * Function Name  : Main_Circulation
 * Description    : 主循环
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
__attribute__((section(".highcode")))
void Main_Circulation() {
    while (1) {
        rf_tx_process();
        TMOS_SystemProcess();
    }
}

/*******************************************************************************
 * Function Name  : main
 * Description    : 主函数
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
    SysTick_Config(FREQ_SYS/1000);  //1ms
    PowerMonitor(ENABLE, HALevel_2V5);
#if (defined (DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif

#if (defined (HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg( GPIO_Pin_All, GPIO_ModeIN_PD);
    GPIOB_ModeCfg( GPIO_Pin_All, GPIO_ModeIN_PD);
#endif

#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    DEBUG_Init();

    //电池检测
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_PD);
    GPIOA_SetBits(GPIO_Pin_14);
    GPIOA_ModeCfg( GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    if(GPIOA_ReadPortPin(GPIO_Pin_15)){
        GPIOA_ResetBits(GPIO_Pin_14);
        isUSBinsert = true;
        PRINT("USB insert\n");
    } else{
        GPIOA_ModeCfg( GPIO_Pin_14, GPIO_ModeIN_Floating);
        isUSBinsert = false;
        PRINT("NO USB\n");
    }

    /*DataFlash初始化*/
    if (easyflash_init() != SUCCESS) {
        LOG_INFO("Date Flash init error!");
    }
    ef_print_env();
    ReadDeviceInfo("all");  //must process after easyflash_init()

    ring_buffer_init();

    LOG_INFO("device id:%#x", device_bond.ID_Num);
    Mode_Init(device_mode);

    Main_Circulation();
}

/******************************** endfile @ main ******************************/
