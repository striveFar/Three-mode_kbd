#include "test_dtm_port.h"
#include "test_dtm.h"

__attribute__((aligned(4))) uint8_t test_EP0_Databuf[64+64+64]; //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4))) uint8_t test_EP1_Databuf[64+64];    //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4))) uint8_t test_EP2_Databuf[64+64];    //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4))) uint8_t test_EP3_Databuf[64+64];    //ep3_out(64)+ep3_in(64)

UINT8 test_DevConfig;
UINT8 test_SetupReqCode;
UINT16 test_SetupReqLen;
const UINT8 *test_pDescr;

#define DevEP0SIZE  0x40
// �豸������
const UINT8 MyDevDescr[] = { 0x12,0x01,0x10,0x01,0xFF,0x00,0x00,DevEP0SIZE,
                             0x86,0x1A,0x23,0x75,0x63,0x02,0x00,0x02,
                             0x00,0x01 };
// ����������
const UINT8 MyCfgDescr[] = {     0x09,0x02,0x27,0x00,0x01,0x01,0x00,0x80,0xf0,              //�������������ӿ�������,�˵�������
                                 0x09,0x04,0x00,0x00,0x03,0xff,0x01,0x02,0x00,
                                 0x07,0x05,0x82,0x02,0x20,0x00,0x00,                        //�����ϴ��˵�
                                 0x07,0x05,0x02,0x02,0x20,0x00,0x00,                        //�����´��˵�
                                 0x07,0x05,0x81,0x03,0x08,0x00,0x01};                       //�ж��ϴ��˵�
// ����������
const UINT8 MyLangDescr[] = { 0x04, 0x03, 0x09, 0x04 };
// ������Ϣ
const UINT8 MyManuInfo[] = { 0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0 };
// ��Ʒ��Ϣ
const UINT8 MyProdInfo[] = { 0x0C, 0x03, 'C', 0, 'H', 0, '5', 0, '8', 0, 'x', 0 };
/*��Ʒ������*/
const UINT8 StrDesc[28] =
{
  0x1C,0x03,0x55,0x00,0x53,0x00,0x42,0x00,
  0x32,0x00,0x2E,0x00,0x30,0x00,0x2D,0x00,
  0x53,0x00,0x65,0x00,0x72,0x00,0x69,0x00,
  0x61,0x00,0x6C,0x00
};

const UINT8 Return1[2] = {0x31,0x00};
const UINT8 Return2[2] = {0xC3,0x00};
const UINT8 Return3[2] = {0x9F,0xEE};

__HIGH_CODE
__attribute__((interrupt("WCH-Interrupt-fast")))
void test_usb_irqhandler(void)
{
    UINT8 len, chtype;
    UINT8 intflag, errflag = 0;
    PRINT("usb irq\n");

    intflag = R8_USB_INT_FG;
    if ( intflag & RB_UIF_TRANSFER )
    {
      if ( ( R8_USB_INT_ST & MASK_UIS_TOKEN ) != MASK_UIS_TOKEN )    // �ǿ���
      {
        switch ( R8_USB_INT_ST & ( MASK_UIS_TOKEN | MASK_UIS_ENDP ) )
        // �����������ƺͶ˵��
        {
          case UIS_TOKEN_IN :
          {
            switch ( test_SetupReqCode )
            {
              case USB_GET_DESCRIPTOR :
                len = test_SetupReqLen >= DevEP0SIZE ?
                    DevEP0SIZE : test_SetupReqLen;    // ���δ��䳤��
                memcpy( pEP0_DataBuf, test_pDescr, len ); /* �����ϴ����� */
                test_SetupReqLen -= len;
                test_pDescr += len;
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL ^= RB_UEP_T_TOG;                             // ��ת
                break;
              case USB_SET_ADDRESS :
                R8_USB_DEV_AD = ( R8_USB_DEV_AD & RB_UDA_GP_BIT ) | test_SetupReqLen;
                R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
              default :
                R8_UEP0_T_LEN = 0;                                      // ״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ���
                R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
          }
            break;

          case UIS_TOKEN_OUT :
          {
            len = R8_USB_RX_LEN;
          }
            break;

          case UIS_TOKEN_OUT | 1 :
          {
            if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
            {                       // ��ͬ�������ݰ�������
              len = R8_USB_RX_LEN;
              DevEP1_OUT_Deal( len );
            }
          }
            break;

          case UIS_TOKEN_IN | 1 :
            R8_UEP1_CTRL = ( R8_UEP1_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
            break;

          case UIS_TOKEN_OUT | 2 :
          {
            if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
            {                       // ��ͬ�������ݰ�������
              len = R8_USB_RX_LEN;
              DevEP2_OUT_Deal( len );
            }
          }
            break;

          case UIS_TOKEN_IN | 2 :
            R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
            break;

          case UIS_TOKEN_OUT | 3 :
          {
            if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
            {                       // ��ͬ�������ݰ�������
              len = R8_USB_RX_LEN;
              DevEP3_OUT_Deal( len );
            }
          }
            break;

          case UIS_TOKEN_IN | 3 :
            R8_UEP3_CTRL = ( R8_UEP3_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
            break;

          case UIS_TOKEN_OUT | 4 :
          {
            if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
            {
              R8_UEP4_CTRL ^= RB_UEP_R_TOG;
              len = R8_USB_RX_LEN;
              DevEP4_OUT_Deal( len );
            }
          }
            break;

          case UIS_TOKEN_IN | 4 :
            R8_UEP4_CTRL ^= RB_UEP_T_TOG;
            R8_UEP4_CTRL = ( R8_UEP4_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
            break;

          default :
            break;
        }
        R8_USB_INT_FG = RB_UIF_TRANSFER;
      }
      if ( R8_USB_INT_ST & RB_UIS_SETUP_ACT )                  // Setup������
      {
        R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
        test_SetupReqLen = pSetupReqPak->wLength;
        test_SetupReqCode = pSetupReqPak->bRequest;
        chtype = pSetupReqPak->bRequestType;

        len = 0;
        errflag = 0;
        if ( ( pSetupReqPak->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
        {
          if( pSetupReqPak->bRequestType == 0xC0 )
          {
            if(test_SetupReqCode==0x5F)
            {
              test_pDescr = Return1;
              len = sizeof(Return1);
            }
            else if(test_SetupReqCode==0x95)
            {
              if((pSetupReqPak->wValue)==0x18)
              {
                test_pDescr = Return2;
                len = sizeof(Return2);
              }
              else if((pSetupReqPak->wValue)==0x06)
              {
                test_pDescr = Return3;
                len = sizeof(Return3);
              }
            }
            else
            {
              errflag = 0xFF;
            }
            memcpy(pEP0_DataBuf,test_pDescr,len);
          }
          else
          {
            len = 0;
          }
        }
        else /* ��׼���� */
        {
          switch ( test_SetupReqCode )
          {
            case USB_GET_DESCRIPTOR :
            {
              switch ( ( ( pSetupReqPak->wValue ) >> 8 ) )
              {
                case USB_DESCR_TYP_DEVICE :
                {
                  test_pDescr = MyDevDescr;
                  len = sizeof(MyDevDescr);
                }
                  break;

                case USB_DESCR_TYP_CONFIG :
                {
                  test_pDescr = MyCfgDescr;
                  len = sizeof(MyCfgDescr);
                }
                  break;

                case USB_DESCR_TYP_REPORT :
  //              {
  //                if ( ( ( pSetupReqPak->wIndex ) & 0xff ) == 0 )                             //�ӿ�0����������
  //                {
  //                  test_pDescr = KeyRepDesc;                                  //����׼���ϴ�
  //                  len = sizeof( KeyRepDesc );
  //                }
  //                else if ( ( ( pSetupReqPak->wIndex ) & 0xff ) == 1 )                        //�ӿ�1����������
  //                {
  //                  test_pDescr = MouseRepDesc;                                //����׼���ϴ�
  //                  len = sizeof( MouseRepDesc );
  //                  Ready = 1;                                            //����и���ӿڣ��ñ�׼λӦ�������һ���ӿ�������ɺ���Ч
  //                }
  //                else
  //                  len = 0xff;                                           //������ֻ��2���ӿڣ���仰����������ִ��
  //              }
                  break;

                case USB_DESCR_TYP_STRING :
                {
                  switch ( ( pSetupReqPak->wValue ) & 0xff )
                  {
                    case 1 :
                      test_pDescr = MyManuInfo;
                      len = MyManuInfo[0];
                      break;
                    case 2 :
                      test_pDescr = StrDesc;
                      len = StrDesc[0];
                      break;
                    case 0 :
                      test_pDescr = MyLangDescr;
                      len = MyLangDescr[0];
                      break;
                    default :
                      errflag = 0xFF;                               // ��֧�ֵ��ַ���������
                      break;
                  }
                }
                  break;

                default :
                  errflag = 0xff;
                  break;
              }
              if ( test_SetupReqLen > len )
                test_SetupReqLen = len;      //ʵ�����ϴ��ܳ���
              len = ( test_SetupReqLen >= DevEP0SIZE ) ?
                  DevEP0SIZE : test_SetupReqLen;
              memcpy( pEP0_DataBuf, test_pDescr, len );
              test_pDescr += len;
            }
              break;

            case USB_SET_ADDRESS :
              test_SetupReqLen = ( pSetupReqPak->wValue ) & 0xff;
              break;

            case USB_GET_CONFIGURATION :
              pEP0_DataBuf[0] = test_DevConfig;
              if ( test_SetupReqLen > 1 )
                test_SetupReqLen = 1;
              break;

            case USB_SET_CONFIGURATION :
              test_DevConfig = ( pSetupReqPak->wValue ) & 0xff;
              break;

            case USB_CLEAR_FEATURE :
            {
              if ( ( pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )    // �˵�
              {
                switch ( ( pSetupReqPak->wIndex ) & 0xff )
                {
                  case 0x82 :
                    R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~( RB_UEP_T_TOG | MASK_UEP_T_RES ) ) | UEP_T_RES_NAK;
                    break;
                  case 0x02 :
                    R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~( RB_UEP_R_TOG | MASK_UEP_R_RES ) ) | UEP_R_RES_ACK;
                    break;
                  case 0x81 :
                    R8_UEP1_CTRL = ( R8_UEP1_CTRL & ~( RB_UEP_T_TOG | MASK_UEP_T_RES ) ) | UEP_T_RES_NAK;
                    break;
                  case 0x01 :
                    R8_UEP1_CTRL = ( R8_UEP1_CTRL & ~( RB_UEP_R_TOG | MASK_UEP_R_RES ) ) | UEP_R_RES_ACK;
                    break;
                  default :
                    errflag = 0xFF;                                 // ��֧�ֵĶ˵�
                    break;
                }
              }
              else
                errflag = 0xFF;
            }
              break;

            case USB_GET_INTERFACE :
              pEP0_DataBuf[0] = 0x00;
              if ( test_SetupReqLen > 1 )
                test_SetupReqLen = 1;
              break;

            case USB_GET_STATUS :
              pEP0_DataBuf[0] = 0x00;
              pEP0_DataBuf[1] = 0x00;
              if ( test_SetupReqLen > 2 )
                test_SetupReqLen = 2;
              break;

            default :
              errflag = 0xff;
              break;
          }
        }
        if ( errflag == 0xff )        // �����֧��
        {
  //                  test_SetupReqCode = 0xFF;
          R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;    // STALL
        }
        else
        {
          if ( chtype & 0x80 )     // �ϴ�
          {
            len = ( test_SetupReqLen > DevEP0SIZE ) ?
                DevEP0SIZE : test_SetupReqLen;
            test_SetupReqLen -= len;
          }
          else
            len = 0;        // �´�
          R8_UEP0_T_LEN = len;
          R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;    // Ĭ�����ݰ���DATA1
        }

        R8_USB_INT_FG = RB_UIF_TRANSFER;
      }
    }
    else if ( intflag & RB_UIF_BUS_RST )
    {
      R8_USB_DEV_AD = 0;
      R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
      R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
      R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
      R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
      R8_USB_INT_FG = RB_UIF_BUS_RST;
    }
    else if ( intflag & RB_UIF_SUSPEND )
    {
      if ( R8_USB_MIS_ST & RB_UMS_SUSPEND )
      {
        ;
      }    // ����
      else
      {
        ;
      }               // ����
      R8_USB_INT_FG = RB_UIF_SUSPEND;
    }
    else
    {
      R8_USB_INT_FG = intflag;
    }
}

void test_dtm_usb_init(void)
{
    PFIC_EnableFastINT0(USB_IRQn, (uint32_t)test_usb_irqhandler);
    pEP0_RAM_Addr = test_EP0_Databuf;
    pEP1_RAM_Addr = test_EP1_Databuf;
    pEP2_RAM_Addr = test_EP2_Databuf;
    pEP3_RAM_Addr = test_EP3_Databuf;
    USB_DeviceInit();
    PFIC_EnableIRQ(USB_IRQn);
}

void DevEP1_OUT_Deal( UINT8 l )
{ /* �û����Զ��� */
}

__attribute__((section(".highcode")))
void DevEP2_OUT_Deal( UINT8 l )
{ /* �û����Զ��� */
    test_dtm_rcv_deal(pEP2_OUT_DataBuf, l);
    PRINT("test rcv[");
    for(int i = 0; i <l; i++) {
        i && PRINT(" ");
        PRINT("%#x", pEP2_OUT_DataBuf[i]);
    }PRINT("]\n");

}

void DevEP3_OUT_Deal( UINT8 l )
{ /* �û����Զ��� */
}

void DevEP4_OUT_Deal( UINT8 l )
{ /* �û����Զ��� */
}

int test_dtm_tx( uint8_t *dtm_cmd, size_t len)
{
    /*TODO: add your code here*/
    memcpy(pEP2_IN_DataBuf, dtm_cmd, len);
    DevEP2_IN_Deal( len );
    PRINT("test sent[");
    for(int i = 0; i <len; i++) {
        i && PRINT(" ");
        PRINT("%#x", pEP2_OUT_DataBuf[i]);
    }PRINT("]\n");

    return 0;
}

