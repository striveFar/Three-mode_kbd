/********************************** (C) COPYRIGHT *******************************
 * File Name          : MCU.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2019/11/05
 * Description        : Ӳ������������BLE��Ӳ����ʼ��
 *******************************************************************************/

/******************************************************************************/
/* ͷ�ļ����� */
#include <USB_task/USB_kbd.h>
#include "CH58x_common.h"
#include "HAL.h"

#include "device_config.h"
#include "RF_task/rf_task.h"
#include "BLE/hidkbd.h"
#include "key_scan/keyscan.h"
#include "backlight/backlight.h"
#include "RF_PHY/rf_process.h"


tmosTaskID halTaskID;

bool is_idle = false;

/*******************************************************************************
 * @fn          Lib_Calibration_LSI
 *
 * @brief       �ڲ�32kУ׼
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void Lib_Calibration_LSI( void )
{
    uint32_t irq_status;
    SYS_DisableAllIrq( &irq_status );
    Calibration_LSI( Level_64 );
    SYS_RecoverIrq( irq_status );
}

#if (defined (BLE_SNV)) && (BLE_SNV == TRUE)
/*******************************************************************************
 * @fn          Lib_Read_Flash
 *
 * @brief       Lib ����Flash�ص�
 *
 * input parameters
 *
 * @param       addr.
 * @param       num.
 * @param       pBuf.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
u32 Lib_Read_Flash( u32 addr, u32 num, u32 *pBuf )
{
  EEPROM_READ( addr, pBuf, num * 4 );
  return 0;
}

/*******************************************************************************
 * @fn          Lib_Write_Flash
 *
 * @brief       Lib ����Flash�ص�
 *
 * input parameters
 *
 * @param       addr.
 * @param       num.
 * @param       pBuf.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
u32 Lib_Write_Flash( u32 addr, u32 num, u32 *pBuf )
{
  EEPROM_ERASE( addr, EEPROM_PAGE_SIZE*2 );
  EEPROM_WRITE( addr, pBuf, num * 4 );
  return 0;
}
#endif


void test( uint8_t code, uint32_t status ){
//    LOG_DEBUG("code= %#x, status=%#x\n", code, status);
}

/*******************************************************************************
 * @fn          CH57X_BLEInit
 *
 * @brief       BLE ���ʼ��
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void CH57X_BLEInit( void )
{
  uint8 i;
  bleConfig_t cfg;
  if ( tmos_memcmp( VER_LIB, VER_FILE, strlen( VER_FILE )  ) == FALSE )
  {
    PRINT( "head file error...\n" );
    while( 1 )
      ;
  }

  tmos_memset( &cfg, 0, sizeof(bleConfig_t) );
  cfg.MEMAddr = ( u32 ) MEM_BUF;
  cfg.MEMLen = ( u32 ) BLE_MEMHEAP_SIZE;
  cfg.BufMaxLen = ( u32 ) BLE_BUFF_MAX_LEN;
  cfg.BufNumber = ( u32 ) BLE_BUFF_NUM;
  cfg.TxNumEvent = ( u32 ) BLE_TX_NUM_EVENT;
  cfg.TxPower = ( u32 ) BLE_TX_POWER;
#if (defined (BLE_SNV)) && (BLE_SNV == TRUE)
  FLASH_ROM_LOCK( 0 );                    // ����flash

  if(device_mode == MODE_BLE)
  if(device_bond.ID_Num == 0) {
      cfg.SNVAddr = ( u32 ) BLE_SNV_ADDR0;
  } else if(device_bond.ID_Num == 1){
      cfg.SNVAddr = ( u32 ) BLE_SNV_ADDR1;
  } else if(device_bond.ID_Num == 2){
      cfg.SNVAddr = ( u32 ) BLE_SNV_ADDR2;
  } else if(device_bond.ID_Num == 3){
      cfg.SNVAddr = ( u32 ) BLE_SNV_ADDR3;
  } else {
      //should not be here!
  }

  cfg.readFlashCB = Lib_Read_Flash;
  cfg.writeFlashCB = Lib_Write_Flash;
  cfg.staCB = test;
#endif
#if( CLK_OSC32K )	
  cfg.SelRTCClock = ( u32 ) CLK_OSC32K ;
#endif
  cfg.ConnectNumber = ( PERIPHERAL_MAX_CONNECTION & 3 ) | ( CENTRAL_MAX_CONNECTION << 2 );
  cfg.srandCB = SYS_GetSysTickCnt;
#if (defined TEM_SAMPLE)  && (TEM_SAMPLE == TRUE)
  cfg.tsCB = HAL_GetInterTempValue;    // �����¶ȱ仯У׼RF���ڲ�RC( ����7���϶� )
#if( CLK_OSC32K )
  cfg.rcCB = Lib_Calibration_LSI;    // �ڲ�32Kʱ��У׼
#endif
#endif
#if (defined (HAL_SLEEP)) && (HAL_SLEEP == TRUE)
  cfg.WakeUpTime = WAKE_UP_RTC_MAX_TIME;
  cfg.sleepCB = CH58X_LowPower;    // ����˯��
#endif
#if (defined (BLE_MAC)) && (BLE_MAC == TRUE)
  /* four mac */
  if(device_bond.ID_Num == 0) {
      for ( i = 0; i < 6; i++ )
        cfg.MacAddr[i] = MacAddr[5 - i];
  } else if(device_bond.ID_Num == 1){
      MacAddr[5] += 1;  //change your mac here.
      for ( i = 0; i < 6; i++ )
        cfg.MacAddr[i] = MacAddr[5 - i];
  } else if(device_bond.ID_Num == 2){
      MacAddr[5] += 2; //change your mac here.
      for ( i = 0; i < 6; i++ )
        cfg.MacAddr[i] = MacAddr[5 - i];
  } else if(device_bond.ID_Num == 3){
      MacAddr[5] += 3; //change your mac here.
      for ( i = 0; i < 6; i++ )
        cfg.MacAddr[i] = MacAddr[5 - i];
  } else {
      //should not be here!
  }

#else
  {
    GetMACAddress( MacAddr );

    if(device_bond.ID_Num == 0) {
        for ( i = 0; i < 6; i++ )
          cfg.MacAddr[i] = MacAddr[i];
    } else if(device_bond.ID_Num == 1){
        MacAddr[0] += 1;  //change your mac here.
        for ( i = 0; i < 6; i++ )
          cfg.MacAddr[i] = MacAddr[i];
    } else if(device_bond.ID_Num == 2){
        MacAddr[0] += 2; //change your mac here.
        for ( i = 0; i < 6; i++ )
          cfg.MacAddr[i] = MacAddr[i];
    } else if(device_bond.ID_Num == 3){
        MacAddr[0] += 3; //change your mac here.
        for ( i = 0; i < 6; i++ )
          cfg.MacAddr[i] = MacAddr[i];
    } else {
        //should not be here!
    }
  }
#endif
  if ( !cfg.MEMAddr || cfg.MEMLen < 4 * 1024 )
    while( 1 )
      ;
  i = BLE_LibInit( &cfg );
  if ( i )
  {
    PRINT( "LIB init error code: %x ...\n", i );
    while( 1 )
      ;
  }
}

static void HAL_ProcessTMOSMsg( tmos_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {


    default:
        break;
  }
}

/*******************************************************************************
 * @fn          HAL_ProcessEvent
 *
 * @brief       Ӳ����������
 *
 * input parameters
 *
 * @param       task_id.
 * @param       events.
 *
 * output parameters
 *
 * @param       events.
 *
 * @return      None.
 */
tmosEvents HAL_ProcessEvent( tmosTaskID task_id, tmosEvents events )
{
  uint8 * msgPtr;
  if ( events & SYS_EVENT_MSG )
  {    // ����HAL����Ϣ������tmos_msg_receive��ȡ��Ϣ��������ɺ�ɾ����Ϣ��
    msgPtr = tmos_msg_receive( task_id );
    if ( msgPtr )
    {
      HAL_ProcessTMOSMsg( (tmos_event_hdr_t *)msgPtr );
      /* De-allocate */
      tmos_msg_deallocate( msgPtr );
    }
    return events ^ SYS_EVENT_MSG;
  }

//  if ( events & LED_BLINK_EVENT )
//  {
//#if (defined HAL_LED) && (HAL_LED == TRUE)
//    HalLedUpdate( );
//#endif // HAL_LED
//    return events ^ LED_BLINK_EVENT;
//  }

//  if ( events & HAL_KEY_EVENT )
//  {
//#if (defined HAL_KEY) && (HAL_KEY == TRUE)
////     LOG_INFO("KEY start: %d", millis());
//
//    if(readKeyVal()){
//        LOG_INFO("key!");
//        is_idle = false;
//        struct rf_status_t temp_sta = {
//                .dongle_lost = 0,
//                .idle = 1,
//        };
//        set_rf_status(&temp_sta);
//
//        tmos_start_task(halTaskID, HAL_SLEEP_EVENT, LOW_POWER_SLEEP_TIME_S);  //�޼�ֵ�仯30s˯��
//        tmos_start_task(halTaskID, HAL_SHUTDOWN_EVENT, LOW_POWER_SHUTDOWN_TIME_S);  //�޼�ֵ�仯30s˯��
//        switch(device_mode){
//        case MODE_BLE:
//            OnBoard_SendMsg(hidEmuTaskId, KEY_MESSAGE, notify, NULL);
//            break;
//
//        case MODE_RF24:
//            OnBoard_SendMsg(RFtaskID, KEY_MESSAGE, notify, NULL);
//            break;
//
//        case MODE_USB:
//            OnBoard_SendMsg(USBTaskID, KEY_MESSAGE, notify, NULL);
//            break;
//        default:
//            break;
//        }
//    }
////    LOG_INFO("KEY end: %d", millis());
//    tmos_start_task( halTaskID, HAL_KEY_EVENT, MS1_TO_SYSTEM_TIME(10) );
//    return events ^ HAL_KEY_EVENT;
//#endif
//  }

//  if ( events & HAL_REG_INIT_EVENT )
//  {
//#if (defined BLE_CALIBRATION_ENABLE) && (BLE_CALIBRATION_ENABLE == TRUE)	// У׼���񣬵���У׼��ʱС��10ms
//    BLE_RegInit();    // У׼RF
//#if( CLK_OSC32K )
//    Lib_Calibration_LSI();    // У׼�ڲ�RC
//#endif
//    tmos_start_task( halTaskID, HAL_REG_INIT_EVENT, MS1_TO_SYSTEM_TIME( BLE_CALIBRATION_PERIOD ) );
//    return events ^ HAL_REG_INIT_EVENT;
//#endif
//  }
//
//  if ( events & HAL_SLEEP_EVENT )
//  {
//
//    if(device_mode == MODE_USB){
//      return events ^ HAL_SLEEP_EVENT;
//    }
//    LOG_INFO("Low power sleep !");
//
//#if( DEBUG == Debug_UART1 )  // ʹ���������������ӡ��Ϣ��Ҫ�޸����д���
//      while( ( R8_UART1_LSR & RB_LSR_TX_ALL_EMP ) == 0 )
//      {
//        __nop();
//      }
//#endif
//
//#if HAL_SLEEP
//    is_idle = true;
//#endif
//    return events ^ HAL_SLEEP_EVENT;
//  }
//
//  if ( events & HAL_SHUTDOWN_EVENT )
//  {
//    if(device_mode == MODE_USB){
//      return events ^ HAL_SHUTDOWN_EVENT;
//    }
//      LOG_INFO("Low power shut down!");
//#if( DEBUG == Debug_UART1 )  // ʹ���������������ӡ��Ϣ��Ҫ�޸����д���
//  while( ( R8_UART1_LSR & RB_LSR_TX_ALL_EMP ) == 0 )
//  {
//    __nop();
//  }
//#endif
//#if HAL_SLEEP
//      RstAllPins();;
//      LowPower_Shutdown(0);
//#endif
//    return events ^ HAL_SHUTDOWN_EVENT;
//  }
//
//
//  if(events & HAL_ADC_EVENT) {
//    static bool is_adc_init = FALSE;
//    if(!is_adc_init) {
//      ADC_ExtSingleChSampInit( SampleFreq_3_2, ADC_PGA_0 );
//      ADC_ChannelCfg( 3 );
//      is_adc_init = TRUE;
//    }
//
//    uint32_t adcsum = 0;
//    GPIOA_ModeCfg(GPIO_Pin_13, GPIO_ModeIN_Floating);
//    for(uint8_t i = 0; i < 20; i++ )
//    {
//      adcsum += ADC_ExcutSingleConver();      // ��������20��
//    }
//    GPIOA_ModeCfg(GPIO_Pin_13, GPIO_ModeIN_PU);
//    uint32_t adcavg = adcsum/20;
//    LOG_INFO("adc avg: %d", adcavg);
//
//#define MIN_BATTERY_VOL         2.5
//#define ADC_THRESHOLD (MIN_BATTERY_VOL*2048/1.05)  //����2.5V˯��
//
//    if(adcavg < (uint32_t)ADC_THRESHOLD) {
//      LOG_INFO("battery low...");
//#if( DEBUG == Debug_UART1 )  // ʹ���������������ӡ��Ϣ��Ҫ�޸����д���
//      while( ( R8_UART1_LSR & RB_LSR_TX_ALL_EMP ) == 0 )
//      __nop();
//#endif
//      tmos_start_task(halTaskID, HAL_SHUTDOWN_EVENT, MS1_TO_SYSTEM_TIME(10));
//    }
//    tmos_start_task( halTaskID, HAL_ADC_EVENT, MS1_TO_SYSTEM_TIME(60*1000 ) );   //1min ���һ��
//    return events ^ HAL_ADC_EVENT;
//  }
//
//
//
//  if ( events & HAL_TEST_EVENT )
//  {
//#include "RingBuffer/lwrb.h"
//
//    PRINT( "*\n" );
//    static uint8_t buf[8] = {0};
//
//    static bool flag = false;
//    uint8_t report_id = 0;
//
//    struct rf_status_t temp_sta = {
//            .dongle_lost = 0,
//            .idle = 1,
//    };
//    set_rf_status(&temp_sta);
//
//    lwrb_write(&KEY_buff, &report_id, 1);
//    lwrb_write(&KEY_buff, buf, 8);
//    OnBoard_SendMsg(RFtaskID, KEY_MESSAGE, 1, NULL);
//
//
////    OnBoard_SendMsg(hidEmuTaskId, KEY_MESSAGE, notify, NULL);
////    OnBoard_SendMsg(USBTaskID, KEY_MESSAGE, notify, NULL);
//
//    tmos_start_task( halTaskID, HAL_TEST_EVENT, MS1_TO_SYSTEM_TIME(1000));
//    return events ^ HAL_TEST_EVENT;
//  }
  return 0;
}

/*******************************************************************************
 * @fn          HAL_Init
 *
 * @brief       Ӳ����ʼ��
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void HAL_Init()
{
  halTaskID = TMOS_ProcessEventRegister( HAL_ProcessEvent );
  HAL_TimeInit();

#if (defined HAL_ADC) && (HAL_ADC == TRUE)
  tmos_start_task(halTaskID, HAL_ADC_EVENT, MS1_TO_SYSTEM_TIME(10));
#endif

#if (defined HAL_SLEEP) && (HAL_SLEEP == TRUE)
  HAL_SleepInit();
  tmos_start_task(halTaskID, HAL_SLEEP_EVENT, LOW_POWER_SLEEP_TIME_S);  //�޼�ֵ�仯30s˯��
  tmos_start_task(halTaskID, HAL_SHUTDOWN_EVENT, LOW_POWER_SHUTDOWN_TIME_S);  //�޼�ֵ�仯5min˯��
#endif

#if (defined HAL_LED) && (HAL_LED == TRUE)
  bkinit();
#endif

//#if (defined HAL_KEY) && (HAL_KEY == TRUE)
//  keyInit();
//  tmos_start_task( halTaskID, HAL_KEY_EVENT, MS1_TO_SYSTEM_TIME(10) );
//#endif

#if ( defined BLE_CALIBRATION_ENABLE ) && ( BLE_CALIBRATION_ENABLE == TRUE )
  tmos_start_task( halTaskID, HAL_REG_INIT_EVENT, MS1_TO_SYSTEM_TIME( BLE_CALIBRATION_PERIOD ) );    // ���У׼���񣬵���У׼��ʱС��10ms
#endif

//  tmos_start_task( halTaskID, HAL_TEST_EVENT, 1600 );    // ���һ����������
}

/*******************************************************************************
 * @fn          HAL_GetInterTempValue
 *
 * @brief       ���ʹ����ADC�жϲ��������ڴ˺�������ʱ�����ж�.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
uint16 HAL_GetInterTempValue( void )
{
  uint8 sensor, channel, config, tkey_cfg;
  uint16 adc_data;
  
  tkey_cfg = R8_TKEY_CFG;
  sensor = R8_TEM_SENSOR;
  channel = R8_ADC_CHANNEL;
  config = R8_ADC_CFG;
  ADC_InterTSSampInit();
  R8_ADC_CONVERT |= RB_ADC_START;
  while( R8_ADC_CONVERT & RB_ADC_START );
  adc_data = R16_ADC_DATA;
  R8_TEM_SENSOR = sensor;
  R8_ADC_CHANNEL = channel;
  R8_ADC_CFG = config;
  R8_TKEY_CFG = tkey_cfg;
  return ( adc_data );
}


uint8 OnBoard_SendMsg(uint8_t registeredTaskID, uint8 event, uint8 state, void *data)
{
    SendMSG_t *msgPtr;

  if ( registeredTaskID != TASK_NO_TASK )
  {
    // Send the address to the task
    msgPtr = ( SendMSG_t * ) tmos_msg_allocate( sizeof(SendMSG_t));
    if ( msgPtr )
    {
      msgPtr->hdr.event = event;
      msgPtr->hdr.status = state;
      msgPtr->pData = data;
      tmos_msg_send( registeredTaskID, ( uint8 * ) msgPtr );

    }
    return ( SUCCESS );
  }
  else
  {
    return ( FAILURE );
  }
}


/******************************** endfile @ mcu ******************************/
