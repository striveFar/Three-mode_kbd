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

#if(CONFIG_RF_MODE == 2)
static size_t rxBufferIndex = 0;
static size_t rxBufferLength = 0;
static uint8_t rxBuffer[BUFFER_LENGTH];
#endif

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
void read_rxcb(uint8_t *data, size_t len);
void printRSSIcb(char rssi);

static struct rf_AC_cb ac_cbs = {
   .rf_AC_Rx = read_rxcb,
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

    case RX_MODE_HOP_SHUT:
    {
      PRINT("RX_MODE_HOP_SHUT...\n");
      if(is_hopping)
          tmos_set_event( rf_process_taskID , SBP_RF_CHANNEL_HOP_RX_EVT );
      break;
    }
  }
}




void read_rxcb(uint8_t *data, size_t len)
{
#if(CONFIG_RF_MODE == 2)
    static uint16_t last_num = 0xffff;

    if(!len)
      return ;

    rf_buf_t *rcv_buf = (rf_buf_t *)data;

    if(MAGIC_WORD != rcv_buf->magicword)
      return;

    RF_PRC_DBG("read_rxcb get %d bytes=[", len);
    for(int i = 0; i < len; i++) {
      i && RF_PRC_DBG(" ");
      RF_PRC_DBG("%#x", data[i]);
    }RF_PRC_DBG("]\n");

    rf_buf_t respond_data = {0};

    if(data[0] == last_num){  //重包不处理
        respond_data.num = send_num;
        respond_data.magicword = MAGIC_WORD;

        rf_set_rxrespdata((uint8_t *)&respond_data, 0, 5);
        return;
    }

    send_num++;
    last_num = data[0];

    respond_data.num = send_num;
    respond_data.magicword = MAGIC_WORD;

    rf_set_rxrespdata((uint8_t *)&respond_data, 0, 5);

  #ifdef CONFIG_RF_PAIR
      pair_rx_process((rf_pair_buf_t *)(rcv_buf->data));
  #endif /* CONFIG_RF_PAIR */


  #ifdef CONFIG_RF_HOP
        /* 开启 CONFIG_RF_PAIR 配对成功后开始跳频率，未开启成功通信后开始跳频 */
  #ifdef CONFIG_RF_PAIR
//        if(device_pair.Pair_flag)
  #endif /* CONFIG_RF_PAIR */
//            rf_start_hop();//接收端成功收到数据后立即开始跳频
//      rf_start_hop();


  #endif /* CONFIG_RF_HOP */


    if(rf_pro_cb && device_pair.Pair_flag){
        if(rf_pro_cb->rx_read_deal_cb)
            rf_pro_cb->rx_read_deal_cb(rcv_buf->data, \
              len - (sizeof(rf_buf_t)-sizeof(rcv_buf->data)));
    }

    RF_PRC_DBG("\n");
#endif
    return;
}

void printRSSIcb(char rssi)
{
  RF_PRC_DBG("RSSI: %d dBm\n", rssi);
}


/***************** 用户api ************************/
#if(CONFIG_RF_MODE == 2)
int rf_rx_available(void)
{
    return rxBufferLength - rxBufferIndex;
}

int rf_peek(uint8_t *data, size_t len)
{
  if(!rf_status.receiving)
    return -1;

  size_t getlen = rf_rx_available();
  if(getlen > 0){
    tmos_memcpy(data, rxBuffer, getlen);
  }
  return getlen;
}

int rf_read(uint8_t *data ,size_t len)
{
  if(!rf_status.receiving)
    return -1;

  size_t getlen = rf_rx_available();
  if(getlen > 0){
    tmos_memcpy(data, rxBuffer, getlen);
    rxBufferIndex += getlen;
  }
  return getlen;
}
#endif




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
    tmos_set_event( rf_process_taskID , SBP_RF_CHANNEL_HOP_RX_EVT);
}

void rf_shut_hop(void)
{
    RF_FrequencyHoppingShut();
    is_hopping = false;
    tmos_stop_task(rf_process_taskID, SBP_RF_CHANNEL_HOP_RX_EVT);
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


  // 开启跳频接收
  if( events & SBP_RF_CHANNEL_HOP_RX_EVT ){
      PRINT("hop rx...\n");
    if( RF_FrequencyHoppingRx( 200 )) {
      tmos_start_task( rf_process_taskID , SBP_RF_CHANNEL_HOP_RX_EVT ,0 );
    }
    else
    {
        rf_rx_reset();
    }
    return events^SBP_RF_CHANNEL_HOP_RX_EVT;
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

#if (!defined (CONFIG_RF_MODE) || !((CONFIG_RF_MODE == 0) || (CONFIG_RF_MODE== 1) || (CONFIG_RF_MODE == 2)))
#error "Invalid RF_MODE !"
#endif

#ifdef CONFIG_RF_PAIR
  rf_pair_task_init();
  int ret = start_pair();
  PRINT("ret %d\n", ret);

#endif /* CONFIG_RF_PAIR */

#if (CONFIG_RF_MODE == 2)
  rf_buf_t temp ={
          .num = 0,
          .magicword = MAGIC_WORD,
          .data = {0},
  };

  rf_set_rxrespdata((uint8_t *)&temp, 0, sizeof(temp) - sizeof(temp.data));
  rf_status.receiving = 1;
#endif /* CONFIG_RF_MODE */

  rf_ACInit(&myrfConfig);
  rf_AC_RegisterCB(&ac_cbs);

}



/******************************** endfile @ main ******************************/
