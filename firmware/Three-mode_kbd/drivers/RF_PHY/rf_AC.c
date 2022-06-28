/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : rf_AC.c
* Version            : V1.0
* Date               : 2020/08/06
* Description        :
*******************************************************************************/

#include "rf_ac_cfg.h"
#include "CH58x_common.h"
#include "rf_AC.h"

#ifdef CONFIG_RF_AC_DEBUG
#define RF_AC_DBG  PRINT
#else
#define RF_AC_DBG
#endif

uint8 rf_AC_taskID = INVALID_TASK_ID;

uint8_t send_data[BLE_BUFF_MAX_LEN - 7] = { 0 };
size_t ac_send_len = 0;

uint8_t rcv_data[BLE_BUFF_MAX_LEN - 7] = { 0 };
size_t rcv_len = 0;

uint8_t autoenable = 0;
uint32_t sysclk;           //系统主频
uint8_t worksta = RFIDLE;
rf_ac_data_t rf_buf;

#if(CONFIG_RF_MODE == 1)
uint32_t rtctime = 0;
uint32_t conn_interval  = 0;
#elif(CONFIG_RF_MODE == 2)
static bool is_resp_sent = true;
#endif

static struct rf_AC_cb *ac_cbs;

void rf_AC_RegisterCB( struct rf_AC_cb *cb )
{
  ac_cbs = cb;
}

#if(CONFIG_RF_MODE == 1)
void txWakeUp(uint32_t *time)
{
    *time = (rtctime-WAKE_UP_RTC_MAX_TIME-2)< (*time)?(rtctime-WAKE_UP_RTC_MAX_TIME-2):(*time);
}
#endif

uint16 RF_AC_ProcessEvent( uint8 task_id, uint16 events )
{
  if (events & SYS_EVENT_MSG)
  {
    uint8 *pMsg;

    if ((pMsg = tmos_msg_receive( task_id )) != NULL)
    {
      // Release the TMOS message
      tmos_msg_deallocate( pMsg );
    }
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

#if(CONFIG_RF_MODE == 2)
  //接收端成功接收到数据后并成功应答
  if (events & RF_AC_RCV_END_EVT)
  {
    RF_Shut( );
    RF_Rx( rcv_data, rcv_len, 0xFF, 0xFF );
    is_resp_sent = true;
    rcv_len = 0;
    //回调函数
    if(!ac_cbs)
      return (events ^ RF_AC_RCV_END_EVT);

    /* CRC错误 视为发送失败 */
    if(rf_buf.crc == 1 || rf_buf.crc == 2){
      RF_AC_DBG("rcv crc error\n");
      tmos_memset(&rf_buf, 0, sizeof(rf_buf));
    }

    if(rf_buf.data[1] && ac_cbs->read_rssi)
      ac_cbs->read_rssi((char)rf_buf.data[0]);

    if (ac_cbs->rf_AC_Rx)
      ac_cbs->rf_AC_Rx( rf_buf.data+2, rf_buf.data[1] );

    //clear
    tmos_memset(&rf_buf, 0, sizeof(rf_buf));
    return (events ^ RF_AC_RCV_END_EVT);
  }
#endif

#if(CONFIG_RF_MODE == 1)
  //发送端发送后得到应答
  if (events & RF_AC_TX_END_EVT)
  {
    //回调函数
    if(!ac_cbs)
      return (events ^ RF_AC_TX_END_EVT);

    if(rf_buf.crc == 1 || rf_buf.crc == 2){
      RF_AC_DBG("tx respond crc error\n");
      tmos_memset(&rf_buf, 0, sizeof(rf_buf));
    }

    if(rf_buf.data[1] && ac_cbs->read_rssi)
      ac_cbs->read_rssi((char)rf_buf.data[0]);

    if (ac_cbs->rf_AC_TxResp)
      ac_cbs->rf_AC_TxResp(rf_buf.data+2, rf_buf.data[1]);
    //clear
    tmos_memset(&rf_buf, 0, sizeof(rf_buf));
    return (events ^ RF_AC_TX_END_EVT);
  }
#endif

  return 0;
}

int rf_ACInit( rfConfig_t* cfg )
{
  if (!cfg)
    return -1;

  rf_AC_taskID = TMOS_ProcessEventRegister( RF_AC_ProcessEvent );

  ac_cbs = NULL;

  sysclk = GetSysClock( );
  autoenable = cfg->LLEMode & 1;
  RF_AC_DBG("autoenable = %d\n",autoenable);

  PFIC_SetPriority(TMR0_IRQn, 0xf0);

#if(CONFIG_RF_MODE == RFTX)
  worksta = RFTX;
  conn_interval = DIS_SLEEP;
  rtctime = RTC_GetCycle32k()+32*TX_ONCETIME;

#elif(CONFIG_RF_MODE == RFRX)
  // RX mode
  uint8_t state = RF_Rx( rcv_data, rcv_len, 0xFF, 0xFF );
  RF_AC_DBG("RX mode.state = %x\n",state);
#endif


  return 0;
}

void rf_clear_txdata(void)
{
#if(CONFIG_RF_MODE == 1)
    ac_send_len = 0;
#endif
}


void rf_set_txdata( uint8_t *data, uint8_t index, uint8_t len)
{
#if(CONFIG_RF_MODE == 1)
  //TODO: 可以定义一个缓冲区，发送填入缓冲区，发送函数轮询
   if(data)
       tmos_memcpy( send_data + index, data, len );

   ac_send_len += len;
#endif
}


void rf_set_rxrespdata( uint8_t *data, uint8_t index, size_t len )
{
#if(CONFIG_RF_MODE == 2)
    if(!is_resp_sent && index == 0)
        return ;
    is_resp_sent = false;
    tmos_memcpy( rcv_data + index, data, len );
    rcv_len += len;
    RF_AC_DBG("rcv_len=%d", rcv_len);

    RF_Shut( );
    RF_Rx( rcv_data, rcv_len, 0xFF, 0xFF );

    RF_AC_DBG("set rx resp[");
    for(int i = 0; i < rcv_len; i++) {
        if(i) RF_AC_DBG(" ");
        RF_AC_DBG("%#x", rcv_data[i]);
    }RF_AC_DBG("]\n");
#endif
}

void rf_AC( uint8_t *data, uint8_t crc, bool flag )
{
  if(data) {
    rf_buf.crc = crc;
    tmos_memcpy(rf_buf.data, data, data[1]+2);
  }

  if (!(flag ^ autoenable))
  {
    //FIXME: 如果被其他任务占用导致无法及时执行则会导致协议失效
#if(CONFIG_RF_MODE == RFTX)
    tmos_set_event( rf_AC_taskID, RF_AC_TX_END_EVT );
#elif(CONFIG_RF_MODE == RFRX)
    tmos_set_event(rf_AC_taskID, RF_AC_RCV_END_EVT);
#endif
  }
}

void AC_timesync( size_t len )
{
#if(CONFIG_RF_MODE == 2)
  if (autoenable == 0)
  {
    TMR0_TimerInit( SYSTOMS(sysclk) * TX_ONCETIME - AHEADTIME( sysclk ) - RECVTIME( len, sysclk ) );              //时间同步
  }
  else
  {
    TMR0_TimerInit( SYSTOMS(sysclk) * TX_ONCETIME - AHEADTIME( sysclk ) - RECVTIME( BLE_BUFF_MAX_LEN, sysclk ));
  }
  TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
  PFIC_EnableIRQ( TMR0_IRQn );
#endif
}

__attribute__((section(".highcode")))
void rf_tx_process( void )                     //主循环不断查询
{
#if(CONFIG_RF_MODE == 1)
    static uint32_t timeout_con = 0;
    if(((RTC_GetCycle32k()+0xA8C00000-rtctime)%(32*conn_interval) > (32*conn_interval-4)) && (worksta & RFTX))
    {
      while((RTC_GetCycle32k()+0xA8C00000-rtctime)%(32*conn_interval));
      {
          if (ac_send_len == 0)
          {
            timeout_con++;
            if(timeout_con> 1000*30/DIS_SLEEP) //30s无RF数据，调整通信间隔为100ms
            {
                conn_interval =  EN_SLEEP;
                rtctime = RTC_GetCycle32k()+32*conn_interval;
                if(rtctime > 0xA8C00000 )
                    rtctime-=0xA8C00000;
            }
            return;
          }
          conn_interval =  DIS_SLEEP;
          rtctime = RTC_GetCycle32k()+32*conn_interval;
          if(rtctime > 0xA8C00000 )
              rtctime-=0xA8C00000;
          RF_Shut( );
          RF_Tx( send_data, ac_send_len, 0xFF, 0xFF );
          timeout_con = 0;
          ac_send_len = 0;
      }
    }
    return;
#endif
}


#if(CONFIG_RF_MODE == RFRX)
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void TMR0_IRQHandler()
{
  uint16_t timeuscount = 0;
  R32_TMR0_CNT_END = sysclk/1000*TX_ONCETIME;

  if (TMR0_GetITFlag( TMR0_3_IT_CYC_END )) {
    TMR0_ClearITFlag( TMR0_3_IT_CYC_END );      // 清除中断标志
  }

  while(1)
  {
    timeuscount++;
    if(RECVCMPSTA)
    {
      RECVCMPSTACLR;
      break;
    }
    DelayUs(1);
    if(timeuscount > (RX_TIMEOUT + 8 * BLE_BUFF_MAX_LEN))
    {
      break;
    }

  }
}
#endif



