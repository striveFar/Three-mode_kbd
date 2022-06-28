/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : RF_PHY.c
* Version            : V1.0
* Date               : 2020/08/06
* Description        : 
*******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "CH58x_common.h"
#include "HAL.h"

#include "rf_process.h"
#include "rf_AC.h"
#include "rf_pair.h"
#include "device_config.h"

#include "backlight/backlight.h"


#ifdef CONFIG_RF_PRC_DEBUG
#define RF_PRC_DBG  PRINT
#else
#define RF_PRC_DBG
#endif
/*********************************************************************
 * GLOBAL TYPEDEFS
 */
uint8_t rf_process_taskID = INVALID_TASK_ID;
static uint8_t registeredTaskID = TASK_NO_TASK;
rfConfig_t myrfConfig;

struct rf_status_t rf_status = {
        .idle = 1,
};

static uint8_t send_num = 0;

#if(CONFIG_RF_MODE == 1)
static uint8_t send_len = 0;
#endif

#ifdef CONFIG_RF_RESEND
static uint8_t resend_count = 0;
#endif

#ifdef CONFIG_RF_HOP
static bool is_hopping = false;
#endif

/*********************************************************************
 * CALLBACKS
 */
void tx_respondcb(uint8_t *data, size_t len);
void printRSSIcb(char rssi);

static struct rf_AC_cb ac_cbs = {
   .rf_AC_TxResp = tx_respondcb,
   .read_rssi = printRSSIcb,
};

struct rf_process_cb * rf_pro_cb;

void rf_pro_RegisterCB( struct rf_process_cb *cb )
{
    rf_pro_cb = cb;
}

/*******************************************************************************
* Function Name  : RF_2G4StatusCallBack
* Description    : RF 状态回调，注意：不可在此函数中直接调用RF接收或者发送API，需要使用事件的方式调用
* Input          : sta - 状态类型
*                   crc - crc校验结果
*                   rxBuf - 数据buf指针
* Output         : None
* Return         : None
*******************************************************************************/
void RF_2G4StatusCallBack( uint8 sta , uint8 crc, uint8 *rxBuf )
{
  switch( sta )
	{
    case TX_MODE_TX_FINISH:
    {
      rf_AC(NULL, crc, 0);
      break;
    }
    case TX_MODE_TX_FAIL:
    {
      break;
    }		
    case TX_MODE_RX_DATA:
    {
      rf_AC(rxBuf, crc, 1);
      break;
    }
    case TX_MODE_RX_TIMEOUT:		// Default timeout is 3ms
    {
      rf_AC(NULL, crc, 1);
      break;
    }		
    case RX_MODE_RX_DATA:  //成功接收发送应答，否则不应答
    {
      AC_timesync(rxBuf[1]);
      rf_AC(rxBuf, crc, 0);
      break;
    }
    case RX_MODE_TX_FINISH:
    {
      rf_AC(NULL, crc, 1);
      break;
    }
    case RX_MODE_TX_FAIL:
    {
      rf_AC(NULL, crc, 1);
      break;
    }

    case TX_MODE_HOP_SHUT:
    {
      PRINT("TX_MODE_HOP_SHUT...\n");
      if(is_hopping)
          tmos_set_event( rf_process_taskID , SBP_RF_CHANNEL_HOP_TX_EVT );
      break;
    }

  }
}


// TODO: 接收数据成功与否的判断需要更为明确
void tx_respondcb(uint8_t *data, size_t len)
{
#if(CONFIG_RF_MODE == 1)
    uint16_t last_num = 0xffff;

  /* 数据长度为0， 传输失败  */
   if(!len)
      return ;

#ifdef CONFIG_RF_RESEND
  resend_count = 0;
#endif /* CONFIG_RF_RESEND */

  rf_buf_t *rsp_buf = (rf_buf_t *)data;

  if(rsp_buf->num == last_num)
      return;

  if(rsp_buf->magicword != MAGIC_WORD)
    return;

  /* update status */
  rf_status.idle = 1;
  rf_status.sending = 0;
  last_num = rsp_buf->num;
  send_num++;

  tmos_stop_task(rf_process_taskID, SBP_RF_TX_TIMEOUT_EVT);


  /* get respond */
  RF_PRC_DBG("tx_respondcb get %d bytes=[", len);
  for(int i = 0; i < len; i++) {
    i && RF_PRC_DBG(" ");
    RF_PRC_DBG("%#x", data[i]);
  }RF_PRC_DBG("]\n\n");


#ifdef CONFIG_RF_PAIR
    int ret =  pair_tx_process((rf_pair_buf_t *)(rsp_buf->data));

    if(ret == 0)
        rf_status.paired = 1;
#endif

#ifdef CONFIG_RF_HOP
/* 开启 CONFIG_RF_PAIR 配对成功后开始跳频率，未开启成功通信后开始跳频 */
#ifdef CONFIG_RF_PAIR
//    if(likely(device_pair.Pair_flag))
#endif /* CONFIG_RF_PAIR */
        //发送端成功收到应答后延时100ms开始跳频
//        rf_start_hop();
//    rf_start_hop();


#endif /* CONFIG_RF_HOP */

    if(rf_pro_cb){
        if(rf_pro_cb->tx_respond_deal_cb)
            rf_pro_cb->tx_respond_deal_cb(rsp_buf->data, \
              len - (sizeof(rf_buf_t)-sizeof(rsp_buf->data)));
    }
#endif
  return;
}



void printRSSIcb(char rssi)
{
  RF_PRC_DBG("RSSI: %d dBm\n", rssi);
}




/* FIXME： 如果接收方成功接收到数据，但是发送方丢失应答数据，导致发送方发送重复数据
 * 虽然通过包序号过滤重复数据，但重复数据发送失败会导致重发，造成带宽的浪费
 */
bool rf_send(uint8_t *data, size_t len, bool force)
{
#if(CONFIG_RF_MODE == 1)
  /*
   * 没法送完成无法再发送
   * 发送完成的定义为： 成功填入数据，tx_respond接收到数据，或者重发次数>5
   * */
  if(!rf_status.idle && !force){
    return false;
  }

  rf_status.idle = 0;
  rf_status.sending = 1;

  uint32_t magic_word = MAGIC_WORD;

  if(force)
      rf_clear_txdata();

  rf_set_txdata(&send_num, 0, sizeof(send_num));
  rf_set_txdata((uint8_t *)&magic_word, 1, sizeof(magic_word));
  rf_set_txdata(data, 5, len);
  send_len = len;  //重发机制基于send_len， 所以send_len即数据长度

#ifdef DEBUG
  if(data){
    RF_PRC_DBG("rf send data=[");
    RF_PRC_DBG("%#x", send_num);
    uint8_t *p = (uint8_t *)&magic_word;
    for(int i = 0; i < 4; i++){
        RF_PRC_DBG(" ");
        RF_PRC_DBG("%#x", *p++);
    }
    for (int i = 0; i < len; i++) {
        RF_PRC_DBG(" ");
        RF_PRC_DBG("%#x", data[i]);
    }
    RF_PRC_DBG("]\n");
  }
#endif

  extern uint32_t conn_interval;

  tmos_start_task(rf_process_taskID, SBP_RF_TX_TIMEOUT_EVT,\
                    MS1_TO_SYSTEM_TIME(conn_interval));
#endif
  return true;
}

void rf_readtask_register(uint8_t taksid)
{
  registeredTaskID = taksid;
}

struct rf_status_t *get_rf_status(void)
{
    return &rf_status;
}

void set_rf_status(struct rf_status_t *sta)
{
    tmos_memcpy(&rf_status, sta, sizeof(struct rf_status_t));
    RF_PRC_DBG("set rf status: %#x\n", rf_status);
}

#ifdef CONFIG_RF_HOP
void rf_start_hop(void)
{
    if(is_hopping)
        return;

    is_hopping = true;
    tmos_set_event( rf_process_taskID , SBP_RF_CHANNEL_HOP_TX_EVT);
}

void rf_shut_hop(void)
{
    PRINT("rf shut hop\n");
    tmos_stop_task(rf_process_taskID, SBP_RF_CHANNEL_HOP_TX_EVT);
    RF_FrequencyHoppingShut();
    is_hopping = false;
}
#endif
/*******************************************************************************
* Function Name  : RF_ProcessEvent
* Description    : RF 事件处理
* Input          : task_id - 任务ID
*                   events - 事件标志
* Output         : None
* Return         : None
*******************************************************************************/
uint16 RF_ProcessEvent( uint8 task_id, uint16 events )
{
  static uint8_t rf_timeout_count = 0;
  static bool is_timeou_set = false;

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = tmos_msg_receive( task_id )) != NULL )
    {
      // Release the TMOS message
      tmos_msg_deallocate( pMsg );
    }
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
/* 开启后，重发5此以上判断为包丢失，并丢弃此包
  在2s内如果丢失3包及以上判定为dongle loss
  dongle loss后开始low power sleep
  重新按下按键可清除dongle loss的标志 */
#if(CONFIG_RF_MODE == 1)
  if(events & SBP_RF_TX_TIMEOUT_EVT) {
#ifdef CONFIG_RF_RESEND
      if(resend_count >= RF_RESEND_NUM_MAX)
#endif /* CONFIG_RF_RESEND */
      {
          rf_timeout_count++;
#ifdef CONFIG_RF_RESEND
          resend_count = 0;
#endif /* CONFIG_RF_RESEND */
          rf_status.idle = 1;
          rf_status.sending = 0;

          if(!is_timeou_set){
              is_timeou_set = true;
              tmos_start_task(rf_process_taskID, SBP_RF_DGLOSS_TIMEOUT_EVT, MS1_TO_SYSTEM_TIME(2000));
          }

          if(rf_pro_cb){
              if(rf_pro_cb->tx_send_failed_cb)
                  rf_pro_cb->tx_send_failed_cb();
          }
      }
#ifdef CONFIG_RF_RESEND
      else {
          resend_count++;
          RF_PRC_DBG("resend %d\n", resend_count);

          rf_send(NULL, send_len, true);
      }
#endif /* CONFIG_RF_RESEND */

    return (events ^ SBP_RF_TX_TIMEOUT_EVT);
  }
#endif /* CONFIG_RF_MODE == 1 */

#if(CONFIG_RF_MODE == 1)
  if( events & SBP_RF_DGLOSS_TIMEOUT_EVT ){

      RF_PRC_DBG("dongle lost timeout!\n");
      is_timeou_set = false;

      if(rf_timeout_count >= 3){
          rf_timeout_count = 0;
          rf_status.dongle_lost = 1;
          //TODO: send dongle lost msg to other task
          tmos_set_event(halTaskID, HAL_SLEEP_EVENT);
      }
      return events^SBP_RF_DGLOSS_TIMEOUT_EVT;
  }
#endif /* CONFIG_RF_MODE == 1 */

  // 开启跳频发送
  if( events & SBP_RF_CHANNEL_HOP_TX_EVT ){
      PRINT("\n------------- hop tx...\n");
    if( RF_FrequencyHoppingTx( 16 ) ){
      tmos_start_task( rf_process_taskID , SBP_RF_CHANNEL_HOP_TX_EVT ,0 );
    }

    return events^SBP_RF_CHANNEL_HOP_TX_EVT;
  }


    
  return 0;
}

#define CONFIG_RF_HOP_INTERVAL      2
/*******************************************************************************
* Function Name  : rf_process_init
* Description    : RF 初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void rf_process_init( void )
{
  uint8 state;

  rf_pro_cb = NULL;

  rf_process_taskID = TMOS_ProcessEventRegister( RF_ProcessEvent );
  tmos_memset( &myrfConfig, 0, sizeof(rfConfig_t) );
  myrfConfig.accessAddress = device_pair.Mac_addr;	// 禁止使用0x55555555以及0xAAAAAAAA ( 建议不超过24次位反转，且不超过连续的6个0或1 )
  myrfConfig.CRCInit = 0x555555;
  myrfConfig.Channel = 17;
  myrfConfig.Frequency = 2480000;
  myrfConfig.LLEMode = LLE_MODE_AUTO | LLE_MODE_PHY_2M; // 使能 LLE_MODE_EX_CHANNEL 表示 选择 rfConfig.Frequency 作为通信频点
  myrfConfig.rfStatusCB = RF_2G4StatusCallBack;
#ifdef CONFIG_RF_HOP
  myrfConfig.HopPeriod = 16;
  myrfConfig.HeartPeriod = CONFIG_RF_HOP_INTERVAL;   //CONFIG_RF_HOP_INTERVAL * 100ms 发送一个心跳包
#if ((CONFIG_RF_HOP_INTERVAL>2) && CLK_OSC32K)
#warning "Use internal 32KHz crystal, CONFIG_RF_HOP_INTERVAL cannot be too large!"
#endif
#endif /* CONFIG_RF_HOP */
  state = RF_Config( &myrfConfig );
  RF_PRC_DBG("rf 2.4g init: %x\n",state);

  PRINT("access addr %#x\n", myrfConfig.accessAddress);

#if (!defined (CONFIG_RF_MODE) || !((CONFIG_RF_MODE == 0) || (CONFIG_RF_MODE== 1) || (CONFIG_RF_MODE == 2)))
#error "Invalid RF_MODE !"
#endif

#ifdef CONFIG_RF_PAIR
  rf_pair_task_init();
  if(device_pair.Pair_flag)
      rf_status.paired = 1;
#endif /* CONFIG_RF_PAIR */

  rf_ACInit(&myrfConfig);
  rf_AC_RegisterCB(&ac_cbs);

#ifdef CONFIG_RF_HOP
#ifdef CONFIG_RF_PAIR
    if(device_pair.Pair_flag)
#endif /* CONFIG_RF_PAIR */
        rf_start_hop();
#endif /* CONFIG_RF_HOP */

}



/******************************** endfile @ main ******************************/
