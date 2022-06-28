/********************************** (C) COPYRIGHT *******************************
 * File Name          : SLEEP.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2019/11/05
 * Description        : ˯�����ü����ʼ����USBģʽ��������Ż�˯�ߣ�����˯��
 *******************************************************************************/

/******************************************************************************/
/* ͷ�ļ����� */
#include "CH58x_common.h"
#include "HAL.h"
#include "RF_PHY/rf_AC.h"
#include "device_config.h"
#include "key_scan/keyscan.h"
#include "USB/usbuser.h"

/*******************************************************************************
 * @fn          CH58X_LowPower
 *
 * @brief       ����˯��
 *
 * input parameters
 *
 * @param       time-���ѵ�ʱ��㣨RTC����ֵ��
 *
 * output parameters
 *
 * @param       
 *
 * @return      None.
 */
u32 CH58X_LowPower( u32 time )
{
#if (defined (HAL_SLEEP)) && (HAL_SLEEP == TRUE)
  u32 tmp, irq_status;
extern bool isUSBinsert,is_idle;
extern uint32_t conn_interval;
extern uint8_t USB_sus_status;
    if(conn_interval == 2)
    {
       return 2;
    }

  if(!is_idle){
      return 2;
  }

  if(device_mode == MODE_USB && USB_sus_status){
      RstAllPins();
      usb_disable();
      LowPower_Shutdown(0);
  }

  SYS_DisableAllIrq( &irq_status );
  txWakeUp(&time);
  tmp = RTC_GetCycle32k();
  if ( ( time < tmp ) || ( ( time - tmp ) < 30 ) )
  {    // ���˯�ߵ����ʱ��
    SYS_RecoverIrq( irq_status );
    return 2;
  }
  RTC_SetTignTime( time );
  SYS_RecoverIrq( irq_status );
#if( DEBUG == Debug_UART1 )  // ʹ���������������ӡ��Ϣ��Ҫ�޸����д���
  while( ( R8_UART1_LSR & RB_LSR_TX_ALL_EMP ) == 0 )
  {
    __nop();
  }
#endif
// LOW POWER-sleepģʽ
  if ( !RTCTigFlag )
  {
    LowPower_Sleep( RB_PWR_RAM2K | RB_PWR_RAM30K | RB_PWR_EXTEND );
    if( !RTCTigFlag )   // ע�����ʹ����RTC����Ļ��ѷ�ʽ����Ҫע���ʱ32M����δ�ȶ�
    {
      time += WAKE_UP_RTC_MAX_TIME;
      if( time > 0xA8C00000 )   time -= 0xA8C00000;
      RTC_SetTignTime( time );
      LowPower_Idle();
    }
    HSECFG_Current( HSE_RCur_100 );     // ��Ϊ�����(�͹��ĺ�����������HSEƫ�õ���)
  }
  else
  {
    return 3;
  }
#endif
  return 0;
}

/*******************************************************************************
 * @fn          HAL_SleepInit
 *
 * @brief       ����˯�߻��ѵķ�ʽ-RTC���ѣ�����ģʽ
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       
 *
 * @return      None.
 */
void HAL_SleepInit( void )
{
#if (defined (HAL_SLEEP)) && (HAL_SLEEP == TRUE)
  R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG1;
  R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG2;
  SAFEOPERATE;
  R8_SLP_WAKE_CTRL |= RB_SLP_RTC_WAKE | RB_SLP_GPIO_WAKE;    // RTC����
  R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG1;
  R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG2;
  SAFEOPERATE;
  R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN;    // ����ģʽ
  R8_SAFE_ACCESS_SIG = 0;    //

  GPIOA_ITModeCfg( GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5, GPIO_ITMode_FallEdge );        // �½��ػ���
  PFIC_EnableIRQ( GPIO_A_IRQn );
  PFIC_EnableIRQ( RTC_IRQn );

  if(device_mode == MODE_USB){
      PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_USB_WAKE, 0 );
  }
#endif
}


