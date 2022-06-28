#include "test_dtm.h"
#include "test_dtm_port.h"
#include "CH58xBLE_LIB.h"

uint8_t dtm_test_recv_len = 0;  //����
uint8_t dtm_test_recv_cmd[16];  //dtm���Խ��յ�buf
uint8_t dtm_test_send_len = 0;  //����
uint8_t dtm_test_send_cmd[16];  //dtm���Է��͵�buf

const uint8_t endTestCmd[4]       = {0x01, 0x1F, 0x20, 0x00};//cmd:0x201f len:0 parm:NULL
const uint8_t resetCmd[4]         = {0x01, 0x03, 0x0C, 0x00};//cmd:0x0c03 len:0 parm:NULL
const uint8_t receiverCmd[4]      = {0x01, 0x1D, 0x20, 0x01};//cmd:0x201d len:1 parm:RX_Channel
const uint8_t transmitterCmd[4]   = {0x01, 0x1E, 0x20, 0x03};//cmd:0x201e len:3 parm:TX_Channel,Test_Data_Length,Packet_Payload
//Enhanced
const uint8_t receiver2MCmd[4]    = {0x01, 0x33, 0x20, 0x03};//cmd:0x2033 len:3 parm:RX_Channel,PHY,Modulation_Index
const uint8_t transmitter2MCmd[4] = {0x01, 0x34, 0x20, 0x04};//cmd:0x2034 len:4 parm:TX_Channel,Test_Data_Length,Packet_Payload,PHY
const uint8_t setPowerCmd[4]      = {0x01, 0x01, 0xFF, 0x01};//cmd:0xff01 len:1 parm:tpl (WCH vendor-specific debug commands)
const uint8_t pFCmd[4]            = {0x01, 0x02, 0xFF, 0x01};//cmd:0xff02 len:1 parm:    (WCH vendor-specific debug commands)
const uint8_t SingleCarrierCmd[4] = {0x01, 0x03, 0xFF, 0x01};//cdm:0xff03 len:1 parm:Channel (WCH vendor-specific debug commands)


/* DTM���� �������� */
void DTMTestEndDeal(void)
{
  uint8_t pktNum[2] = {0};

  LL_TestEnd( pktNum );
  API_LE_TestEndCmd();
  dtm_test_send_cmd[0] = 0x04;
  dtm_test_send_cmd[1] = 0x0E;
  dtm_test_send_cmd[2] = 0x06;
  dtm_test_send_cmd[3] = 0x01;
  dtm_test_send_cmd[4] = dtm_test_recv_cmd[1];
  dtm_test_send_cmd[5] = dtm_test_recv_cmd[2];
  dtm_test_send_cmd[6] = 0x00;
  dtm_test_send_cmd[7] = pktNum[0];
  dtm_test_send_cmd[8] = pktNum[1];
  dtm_test_send_len = 9;

  test_dtm_tx((u8 *)&dtm_test_send_cmd[0],dtm_test_send_len);
}

/* DTM���� ��λ���� */
void DTMTestResetDeal(void)
{
  uint8_t pktNum[2] = {0};

  LL_TestEnd( pktNum );
  API_LE_TestEndCmd();
  dtm_test_send_cmd[0] = 0x04;
  dtm_test_send_cmd[1] = 0x0E;
  dtm_test_send_cmd[2] = 0x04;
  dtm_test_send_cmd[3] = 0x01;
  dtm_test_send_cmd[4] = dtm_test_recv_cmd[1];
  dtm_test_send_cmd[5] = dtm_test_recv_cmd[2];
  dtm_test_send_cmd[6] = 0x00;
  dtm_test_send_len = 7;

  test_dtm_tx((u8 *)&dtm_test_send_cmd[0],dtm_test_send_len);
}

/* DTM���� �������մ��� */
void DTMTestRecvDeal(void)
{
  API_LE_ReceiverTestCmd( &dtm_test_recv_cmd[4],0x201D);
  dtm_test_send_cmd[0] = 0x04;
  dtm_test_send_cmd[1] = 0x0E;
  dtm_test_send_cmd[2] = 0x04;
  dtm_test_send_cmd[3] = 0x01;
  dtm_test_send_cmd[4] = dtm_test_recv_cmd[1];
  dtm_test_send_cmd[5] = dtm_test_recv_cmd[2];
  dtm_test_send_cmd[6] = 0x00;
  dtm_test_send_len = 7;

  test_dtm_tx((u8 *)&dtm_test_send_cmd[0],dtm_test_send_len);
}

/* DTM���� �������ʹ��� */
void DTMTestSendDeal(void)
{
  API_LE_TransmitterTestCmd(&dtm_test_recv_cmd[4],0x201E);

  dtm_test_send_cmd[0] = 0x04;
  dtm_test_send_cmd[1] = 0x0E;
  dtm_test_send_cmd[2] = 0x04;
  dtm_test_send_cmd[3] = 0x01;
  dtm_test_send_cmd[4] = dtm_test_recv_cmd[1];
  dtm_test_send_cmd[5] = dtm_test_recv_cmd[2];
  dtm_test_send_cmd[6] = 0x00;
  dtm_test_send_len = 7;

  test_dtm_tx((u8 *)&dtm_test_send_cmd[0],dtm_test_send_len);
}

/* DTM���� ��չ�������� */
void DTMTestEnhancedRecvDeal(void)
{
  API_LE_ReceiverTestCmd( &dtm_test_recv_cmd[4],0x2033);
  dtm_test_send_cmd[0] = 0x04;
  dtm_test_send_cmd[1] = 0x0E;
  dtm_test_send_cmd[2] = 0x04;
  dtm_test_send_cmd[3] = 0x01;
  dtm_test_send_cmd[4] = dtm_test_recv_cmd[1];
  dtm_test_send_cmd[5] = dtm_test_recv_cmd[2];
  dtm_test_send_cmd[6] = 0x00;
  dtm_test_send_len = 7;

  test_dtm_tx((u8 *)&dtm_test_send_cmd[0],dtm_test_send_len);
}

/* DTM���� ��չ�������� */
void DTMTestEnhancedSendDeal(void)
{
  API_LE_TransmitterTestCmd(&dtm_test_recv_cmd[4],0x2034);

  dtm_test_send_cmd[0] = 0x04;
  dtm_test_send_cmd[1] = 0x0E;
  dtm_test_send_cmd[2] = 0x04;
  dtm_test_send_cmd[3] = 0x01;
  dtm_test_send_cmd[4] = dtm_test_recv_cmd[1];
  dtm_test_send_cmd[5] = dtm_test_recv_cmd[2];
  dtm_test_send_cmd[6] = 0x00;
  dtm_test_send_len = 7;

  test_dtm_tx((u8 *)&dtm_test_send_cmd[0],dtm_test_send_len);
}

/* �������ô��� */
void DTMTestPowerSet(void)
{
  LL_SetTxPowerLevel( dtm_test_recv_cmd[4] );

  dtm_test_send_cmd[0] = 0x04;
  dtm_test_send_cmd[1] = 0x0E;
  dtm_test_send_cmd[2] = 0x04;
  dtm_test_send_cmd[3] = 0x01;
  dtm_test_send_cmd[4] = dtm_test_recv_cmd[1];
  dtm_test_send_cmd[5] = dtm_test_recv_cmd[2];
  dtm_test_send_cmd[6] = 0x00;
  dtm_test_send_len = 7;
  test_dtm_tx((u8 *)&dtm_test_send_cmd[0],dtm_test_send_len);
}

/* ���ݲ������� */
void DTMTest32MTuneSet(void)
{
  R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG1;
  R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG2;
  SAFEOPERATE;
  R8_XT32M_TUNE &= ~RB_XT32M_C_LOAD;
  R8_XT32M_TUNE |= dtm_test_recv_cmd[4]<<4;
  R8_SAFE_ACCESS_SIG = 0;

  dtm_test_send_cmd[0] = 0x04;
  dtm_test_send_cmd[1] = 0x0E;
  dtm_test_send_cmd[2] = 0x04;
  dtm_test_send_cmd[3] = 0x01;
  dtm_test_send_cmd[4] = dtm_test_recv_cmd[1];
  dtm_test_send_cmd[5] = dtm_test_recv_cmd[2];
  dtm_test_send_cmd[6] = 0x00;

  dtm_test_send_len = 7;
  test_dtm_tx((u8 *)&dtm_test_send_cmd[0],dtm_test_send_len);
}

/* ���ز����Դ��� */
void DTMTestSingleCarrier(void)
{
  LL_SingleChannel(dtm_test_recv_cmd[4]);

  dtm_test_send_cmd[0] = 0x04;
  dtm_test_send_cmd[1] = 0x0E;
  dtm_test_send_cmd[2] = 0x04;
  dtm_test_send_cmd[3] = 0x01;
  dtm_test_send_cmd[4] = dtm_test_recv_cmd[1];
  dtm_test_send_cmd[5] = dtm_test_recv_cmd[2];
  dtm_test_send_cmd[6] = 0x00;
  dtm_test_send_len = 7;
  test_dtm_tx((u8 *)&dtm_test_send_cmd[0],dtm_test_send_len);
}


/* DTM���ݽ��մ��� */
void test_dtm_rcv_deal(uint8_t *p_dat,uint8_t len)
{
  uint8_t i;
  uint8_t *p_rec_data;

  p_rec_data = p_dat;
  dtm_test_recv_len = len;
  for(i=0; i<dtm_test_recv_len; i++)
  {
    dtm_test_recv_cmd[i] = p_rec_data[i];
  }

  // ��������������ط���/�յ������ֽ�
  if( tmos_memcmp( dtm_test_recv_cmd, endTestCmd, 4 ))
  {
    DTMTestEndDeal();
  }
  // ��λ���ֹͣ���в���
  else if( tmos_memcmp( dtm_test_recv_cmd, resetCmd, 4 ))
  {
    DTMTestResetDeal();
  }
  // ���ղ�������
  else if( tmos_memcmp( dtm_test_recv_cmd, receiverCmd, 4 ))
  {
    //У������֪����ȷ
    if(dtm_test_recv_len == 5)
    {
      if(dtm_test_recv_cmd[4] < 40)
      {
        DTMTestRecvDeal();
      }
    }
  }
  // ���Ͳ�������
  else if( tmos_memcmp( dtm_test_recv_cmd, transmitterCmd, 4 ))
  {
    if(dtm_test_recv_len == 7)
    {
      if((dtm_test_recv_cmd[4] < 40)   //����channelֵ
       &&(dtm_test_recv_cmd[5])        //���͵�����֡
       &&(dtm_test_recv_cmd[6] < 8))   //������������
      {
        DTMTestSendDeal();
      }
    }
  }
  // Enhanced ��������
  else if( tmos_memcmp( dtm_test_recv_cmd, receiver2MCmd, 4 ))
  {
    DTMTestEnhancedRecvDeal();
  }
  // Enhanced ��������
  else if( tmos_memcmp( dtm_test_recv_cmd, transmitter2MCmd, 4 ))
  {
    DTMTestEnhancedSendDeal();
  }
  //��������
  else if( tmos_memcmp( dtm_test_recv_cmd, setPowerCmd, 4 ))
  {
    DTMTestPowerSet();
  }
  // ������������
  else if( tmos_memcmp( dtm_test_recv_cmd, pFCmd, 4 ))
  {
    DTMTest32MTuneSet();
  }
  //���ز�����
  else if( tmos_memcmp( dtm_test_recv_cmd, SingleCarrierCmd, 4 ))
  {
    DTMTestSingleCarrier();
  }
}



