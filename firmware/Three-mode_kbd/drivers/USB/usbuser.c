/******************************************************************************/
/* ͷ�ļ����� */
#include "CH58x_common.h"
#include "usbuser.h"
#include "RingBuffer/lwrb.h"
#include "HAL/HAL.h"
#include "backlight/backlight.h"
/*********************************************************************
 * GLOBAL TYPEDEFS
 */
uint8_t DevConfig;
uint8_t SetupReqCode;
uint16_t SetupReqLen;

uint8_t USB_sus_status = false;

uint8_t EpnInFlag = 1<<0|1<<1|1<<2;

const uint8_t *pDescr;

S_HID_DATA sHID_Data;

UINT8  f_hot_led_get;
__attribute__((aligned(4))) uint8_t EP0_Databuf[64+64+64]; //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4))) uint8_t EP1_Databuf[64+64];    //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4))) uint8_t EP2_Databuf[64+64];    //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4))) uint8_t EP3_Databuf[64+64];    //ep3_out(64)+ep3_in(64)

void USBDevWakeup(void);

/*******************************************************************************
 * Function Name  : USB_IRQHandler()
 * Description    : CH582USB�жϴ�����
 *******************************************************************************/
__HIGH_CODE void USB_IRQHandler(void)                   //USB�жϷ������
{
    UINT8 len, chtype;
    UINT8 intflag, errflag = 0;

    intflag = R8_USB_INT_FG;
    if (intflag & RB_UIF_TRANSFER) {
        if (( R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN)    // �ǿ���
        {
            switch ( R8_USB_INT_ST & ( MASK_UIS_TOKEN | MASK_UIS_ENDP)) {
            case UIS_TOKEN_IN:
                switch (SetupReqCode) {
                case USB_GET_DESCRIPTOR:
                    len = SetupReqLen >= USB_EP0_PACKET_SIZE ? USB_EP0_PACKET_SIZE : SetupReqLen; // ���δ��䳤��
                    memcpy( pEP0_DataBuf, pDescr, len); /* �����ϴ����� */
                    SetupReqLen -= len;
                    pDescr += len;
                    R8_UEP0_T_LEN = len;
                    R8_UEP0_CTRL ^= RB_UEP_T_TOG;                          // ��ת
                    break;
                case USB_SET_ADDRESS:
                    R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT)
                            | SetupReqLen;
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                default:
                    R8_UEP0_T_LEN = 0;            // ״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ���
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                }
                break;

            case UIS_TOKEN_OUT:
                len = R8_USB_RX_LEN;
                (pEP0_DataBuf[0] & 0x01 )?set_led_num(1):set_led_num(0);
                (pEP0_DataBuf[0] & 0x02 )?set_led_cap(1):set_led_cap(0);
                (pEP0_DataBuf[0] & 0x04 )?set_led_scr(1):set_led_scr(0);

                break;

                /*****�˿�1�ϴ�*****/
            case UIS_TOKEN_IN | 1:
                EpnInFlag |= 1 << 0;
                R8_UEP1_T_LEN = 0;
                //          R8_UEP1_CTRL ^= RB_UEP_T_TOG;                                          //����������Զ���ת����Ҫ�ֶ���ת
                R8_UEP1_CTRL =
                        (R8_UEP1_CTRL & ~ MASK_UEP_T_RES) | UEP_T_RES_NAK; //Ĭ��Ӧ��NAK
                break;
            case UIS_TOKEN_IN | 2:
                EpnInFlag |= 1 << 1;
                R8_UEP2_T_LEN = 0;
                R8_UEP2_CTRL =
                        (R8_UEP2_CTRL & ~ MASK_UEP_T_RES) | UEP_T_RES_NAK; //Ĭ��Ӧ��NAK
                break;
            case UIS_TOKEN_IN | 3:
                EpnInFlag |= 1 << 2;
                R8_UEP3_T_LEN = 0;
                R8_UEP3_CTRL =
                        (R8_UEP3_CTRL & ~ MASK_UEP_T_RES) | UEP_T_RES_NAK; //Ĭ��Ӧ��NAK
                break;

                /*****�˿�1�´�*****/
            case UIS_TOKEN_OUT | 1:
                break;
            case UIS_TOKEN_OUT | 2:
                break;
            case UIS_TOKEN_OUT | 3:
                break;

            default:
                break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }
        if ( R8_USB_INT_ST & RB_UIS_SETUP_ACT)                  // Setup������
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
            SetupReqLen = pSetupReqPak->wLength;
            SetupReqCode = pSetupReqPak->bRequest;
            chtype = pSetupReqPak->bRequestType;

            len = 0;
            errflag = 0;
            if (( pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD) {
                if ((chtype & USB_REQ_TYP_MASK) == USB_REQ_TYP_CLASS) {
                    switch (SetupReqCode) {
                    case HID_GET_REPORT:
                        if ((DevConfig == 0x01)
                                && (chtype == HID_REQUEST_GET)) {
                            if ((pSetupReqPak->wIndex & 0xff) == USB_INTERFACE_0) // Interface 0
                            {
                                if ((pSetupReqPak->wValue & 0xff00) >> 8== HID_REPORT_INPUT) {
                                } else if ((pSetupReqPak->wValue & 0xff00) >> 8== HID_REPORT_OUTPUT) {
                                } else if ((pSetupReqPak->wValue & 0xff00) >> 8== HID_REPORT_FEATURE) {
                                }
                            } else if ((pSetupReqPak->wIndex & 0Xff) == USB_INTERFACE_1)   // Interface 1
                            {
                                if (sHID_Data.wHID_Protocol == USB_REPORT_PROTOCOL) {
                                    if ((pSetupReqPak->wValue & 0xff00) >> 8== HID_REPORT_INPUT) {
                                    }
                                }
                            }
                        }
                        break;
                    case HID_SET_REPORT:
                        if (chtype == HID_REQUEST_SET) {
                            if ((pSetupReqPak->wIndex & 0xff) == USB_INTERFACE_0) // Interface 0
                            {
                                if ((pSetupReqPak->wValue & 0xff00) >> 8 == HID_REPORT_OUTPUT) {
                                    f_hot_led_get = 1;
                                } else if ((pSetupReqPak->wValue & 0xff00) >> 8 == HID_REPORT_FEATURE) {
                                }
                            } else if ((pSetupReqPak->wIndex & 0Xff) == USB_INTERFACE_1)   // Interface 1
                            {
                                if ((pSetupReqPak->wValue & 0xff00) >> 8 == HID_REPORT_OUTPUT) {
                                    f_hot_led_get = 1;
                                }
                            }
                        }
                        break;
                    case HID_GET_IDLE:
                    case HID_GET_PROTOCOL:
                    case HID_SET_IDLE:
                        break;
                    case HID_SET_PROTOCOL:
                    default:
                        errflag = 0xFF;
                        break;
                    }
                }
            }
            else /* ��׼���� */
            {

                switch (SetupReqCode) {
                case USB_GET_DESCRIPTOR:
                    switch (((pSetupReqPak->wValue) >> 8)) {
                    case USB_DESCR_TYP_DEVICE:
                        pDescr = USB_DeviceDescriptor;
                        len = USB_DeviceDescriptor[0];
                        break;

                    case USB_DESCR_TYP_CONFIG:
                        pDescr = USB_ConfigDescriptor;
                        len = USB_ConfigDescriptor[2];
                        break;

                    case USB_DESCR_TYP_REPORT:
                        if (((pSetupReqPak->wIndex) & 0xff) == 0) {
                            pDescr = HID_ReportDescriptor;              //����׼���ϴ�
                            len = HID_ReportDescSize;
                        } else if (((pSetupReqPak->wIndex) & 0xff) == 1) {
                            pDescr = HID_ReportDescriptor2;             //����׼���ϴ�
                            len = HID_ReportDescSize2;
                        } else
                            len = 0xff;                   //������ֻ��2���ӿڣ���仰����������ִ��
                        break;

                    case USB_DESCR_TYP_STRING:
                        switch ((pSetupReqPak->wValue) & 0xff) {
                        case 1:
                            pDescr = USB_ManufacturerStringDescriptor;
                            len = USB_ManufacturerStringDescriptor[0];
                            break;
                        case 2:
                            pDescr = USB_ProductStringDescriptor;
                            len = USB_ProductStringDescriptor[0];
                            break;
                        case 0:
                            pDescr = USB_SerialNumberStringDescriptor;
                            len = USB_SerialNumberStringDescriptor[0];
                            break;
                        default:
                            errflag = 0xFF;                        // ��֧�ֵ��ַ���������
                            break;
                        }
                        break;

                    default:
                        errflag = 0xff;
                        break;
                    }
                    if (SetupReqLen > len)
                        SetupReqLen = len;      //ʵ�����ϴ��ܳ���
                    len = (SetupReqLen >= USB_EP0_PACKET_SIZE) ? USB_EP0_PACKET_SIZE : SetupReqLen;
                    memcpy( pEP0_DataBuf, pDescr, len);
                    pDescr += len;
                    break;

                case USB_SET_ADDRESS:
                    SetupReqLen = ( pSetupReqPak->wValue) & 0xff;
                    break;

                case USB_GET_CONFIGURATION:
                    pEP0_DataBuf[0] = DevConfig;
                    if (SetupReqLen > 1)
                        SetupReqLen = 1;
                    break;

                case USB_SET_CONFIGURATION:
                    DevConfig = ( pSetupReqPak->wValue) & 0xff;
                    break;

                case USB_SET_FEATURE:
                    if ((chtype & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE) //�豸
                    {
                        if ((pSetupReqPak->wValue & 0xff) == USB_FEATURE_REMOTE_WAKEUP)   //Զ�̻���
                        {
//                            wUSB_Status |= (mskREMOTE_WAKEUP | mskDEV_FEATURE_CMD | mskSETUP_IN);
                            LOG_INFO("USB host suspend\n");
                            USB_sus_status = true;
                        }
                    } else if ((chtype & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) {
                        /* �˵� */
                        if ((pSetupReqPak->wValue & 0xff) == USB_FEATURE_ENDPOINT_STALL)
                        {
                                switch (pSetupReqPak->wIndex & 0xff) {
                                case 0x83:
                                    R8_UEP3_CTRL = RB_UEP_T_TOG | UEP_T_RES_STALL | UEP_R_RES_STALL;
                                    break;
                                case 0x03:
                                    R8_UEP3_CTRL = RB_UEP_R_TOG | UEP_T_RES_STALL | UEP_R_RES_STALL;
                                    break;
                                case 0x82:
                                    R8_UEP2_CTRL = RB_UEP_T_TOG | UEP_T_RES_STALL | UEP_R_RES_STALL;
                                   break;
                                case 0x02:
                                    R8_UEP2_T_LEN = 0;
                                    R8_UEP2_CTRL = RB_UEP_R_TOG | UEP_T_RES_STALL | UEP_R_RES_STALL;
                                   break;
                                case 0x81:
                                    R8_UEP1_T_LEN = 0;
                                    R8_UEP1_CTRL = RB_UEP_T_TOG | UEP_T_RES_STALL | UEP_R_RES_STALL;
                                   break;
                                case 0x01:
                                    R8_UEP1_T_LEN = 0;
                                    R8_UEP1_CTRL = RB_UEP_R_TOG | UEP_T_RES_STALL | UEP_R_RES_STALL;
                                   break;

                                default:
                                    errflag = 0xFF;
                                    break;

                                }
                        }
                    } else
                        errflag = 0xFF;
                    break;

                case USB_CLEAR_FEATURE: {
                    if (( pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)    // �˵�
                    {
                        switch (pSetupReqPak->wIndex & 0xff) {
                        case 0x83:
                            R8_UEP3_CTRL = ( R8_UEP3_CTRL & ~( RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                            break;
                        case 0x03:
                            R8_UEP3_CTRL = ( R8_UEP3_CTRL & ~( RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                            break;
                        case 0x82:
                            R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~( RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                            break;
                        case 0x02:
                            R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~( RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                            break;
                        case 0x81:
                            R8_UEP1_CTRL = ( R8_UEP1_CTRL & ~( RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                            break;
                        case 0x01:
                            R8_UEP1_CTRL = ( R8_UEP1_CTRL & ~( RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                            break;
                        default:
                            errflag = 0xFF;                            // ��֧�ֵĶ˵�
                            break;
                        }
                    } else if ((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE){
                        if (((pSetupReqPak->wValue) & 0xff) == USB_FEATURE_REMOTE_WAKEUP){
                            /* Զ�̻��� */
                            LOG_INFO("USB host wake\n");
                            USB_sus_status = false;
                        }
                    } else
                        errflag = 0xFF;
                }
                    break;

                case USB_GET_INTERFACE:
                    pEP0_DataBuf[0] = 0x00;
                    if (SetupReqLen > 1)
                        SetupReqLen = 1;
                    break;

                case USB_GET_STATUS:
                    if (( pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) {
                        pEP0_DataBuf[0] = 0x00;
                        switch (( pSetupReqPak->wIndex) & 0xff) {
                        case 0x83:
                            if((R8_UEP3_CTRL & MASK_UEP_T_RES) == UEP_T_RES_STALL)
                                pEP0_DataBuf[0] = 0x01;
                            break;
                        case 0x03:
                            if((R8_UEP3_CTRL & MASK_UEP_R_RES) == UEP_R_RES_STALL)
                                pEP0_DataBuf[0] = 0x01;
                            break;
                        case 0x82:
                            if((R8_UEP2_CTRL & MASK_UEP_T_RES) == UEP_T_RES_STALL)
                                pEP0_DataBuf[0] = 0x01;
                            break;
                        case 0x02:
                            if((R8_UEP2_CTRL & MASK_UEP_R_RES) == UEP_R_RES_STALL)
                                pEP0_DataBuf[0] = 0x01;
                            break;
                        case 0x81:
                            if((R8_UEP1_CTRL & MASK_UEP_T_RES) == UEP_T_RES_STALL)
                                pEP0_DataBuf[0] = 0x01;
                            break;
                        case 0x01:
                            if((R8_UEP1_CTRL & MASK_UEP_R_RES) == UEP_R_RES_STALL)
                                pEP0_DataBuf[0] = 0x01; 
                            break;
                        }
                    } else if(( pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE) {
                        pEP0_DataBuf[ 0 ] = 0x00;
                        if( USB_sus_status )
                        {
                            pEP0_DataBuf[ 0 ] = 0x02;
                        }
                        else
                        {
                            pEP0_DataBuf[0] = 0x00;
                        }
                    }
                    pEP0_DataBuf[1] = 0x00;
                    if (SetupReqLen > 2)
                        SetupReqLen = 2;
                    break;

                default:
                    errflag = 0xff;
                    break;
                }
            }
            if (errflag == 0xff)        // �����֧��
            {
//             SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;    // STALL
            } else {
                if (chtype & 0x80) {        //�ϴ�
                    len = (SetupReqLen > USB_EP0_PACKET_SIZE) ? USB_EP0_PACKET_SIZE :SetupReqLen;
                    SetupReqLen -= len;
                } else
                    len = 0;        // �´�
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;    // Ĭ�����ݰ���DATA1
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }
    } else if (intflag & RB_UIF_BUS_RST) {
        R8_USB_DEV_AD = 0;
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
        R8_USB_INT_FG |= RB_UIF_BUS_RST;
    } else if (intflag & RB_UIF_SUSPEND) {
        if ( R8_USB_MIS_ST & RB_UMS_SUSPEND) {      // ����
//            wUSB_Status |= mskBUSSUSPEND;
        } else {
            ;
        }                             // ����
        R8_USB_INT_FG = RB_UIF_SUSPEND;
    } else {
        R8_USB_INT_FG = intflag;
    }
}


bool DevEPn_IN_Deal(void) {
    if(unlikely(USB_sus_status == true)){
        USB_sus_status = false;
        USBDevWakeup();
    }

    uint8_t report_id;
    lwrb_peek(&KEY_buff, 0, &report_id, 1);

    if(report_id == KEYNORMAL_ID) {
        if(!(EpnInFlag & 1<<0)) return true;
        lwrb_skip(&KEY_buff, 1);
        lwrb_read(&KEY_buff, EP1_Databuf+64, 8);

        PRINT("key normal IN:[");
        for(int i = 0; i < 8; i++){
            if(i) PRINT(" ");
            PRINT("%#x", EP2_Databuf[64+i]);
        }PRINT("]\n");

        PFIC_DisableIRQ(USB_IRQn);
        EpnInFlag &= ~(1 << 0);
        R8_UEP1_T_LEN = 8;
        R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
        PFIC_EnableIRQ(USB_IRQn);
    } else if( report_id == KEYBIT_ID ){        //bit mdoe
        if(!(EpnInFlag & 1<<1)) return true;
        lwrb_skip(&KEY_buff, 1);
        lwrb_read(&KEY_buff, EP2_Databuf+64, 16);

        EP2_Databuf[64] = KEYBIT_ID;
        PFIC_DisableIRQ(USB_IRQn);
        EpnInFlag &= ~(1 << 1);
        R8_UEP2_T_LEN = 16;
        R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
        PFIC_EnableIRQ(USB_IRQn);
    } else if (report_id == SYS_ID) {
        if(!(EpnInFlag & 1<<1)) return true;
        lwrb_skip(&KEY_buff, 1);
        lwrb_read(&KEY_buff, EP2_Databuf+64+1, 2);
        EP2_Databuf[64] = SYS_ID;

        PRINT("SYS ID IN:[");
        for(int i = 0; i < 3; i++){
            if(i) PRINT(" ");
            PRINT("%#x", EP2_Databuf[64+i]);
        }PRINT("]\n");

        PFIC_DisableIRQ(USB_IRQn);
        EpnInFlag &= ~(1 << 1);
        R8_UEP2_T_LEN = 3;
        R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
        PFIC_EnableIRQ(USB_IRQn);
    } else if(report_id == CONSUME_ID){
        if(!(EpnInFlag & 1<<1)) return true;
        lwrb_skip(&KEY_buff, 1);
        lwrb_read(&KEY_buff, EP2_Databuf+64+1, 2);
        EP2_Databuf[64] = CONSUME_ID;

        PRINT("CONSUME_ID IN:[");
        for(int i = 0; i < 3; i++){
            if(i) PRINT(" ");
            PRINT("%#x", EP2_Databuf[64+i]);
        }PRINT("]\n");

        PFIC_DisableIRQ(USB_IRQn);
        EpnInFlag &= ~(1 << 1);
        R8_UEP2_T_LEN = 3;
        R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
        PFIC_EnableIRQ(USB_IRQn);
    }else if (report_id == VENDOR_ID) {

    }else if (report_id == MOUSE_ID) {
        if(!(EpnInFlag & 1<<1)) return true;
        lwrb_skip(&KEY_buff, 1);
        lwrb_read(&KEY_buff, EP2_Databuf+64+1, 4);
        EP2_Databuf[64] = MOUSE_ID;

        PRINT("MOUSE_ID IN:[");
        for(int i = 0; i < 4; i++){
            if(i) PRINT(" ");
            PRINT("%#x", EP2_Databuf[64+i]);
        }PRINT("]\n");

        PFIC_DisableIRQ(USB_IRQn);
        EpnInFlag &= ~(1 << 1);
        R8_UEP2_T_LEN = 5;
        R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
        PFIC_EnableIRQ(USB_IRQn);
    } else {
        /* should not be here */
        LOG_DEBUG("report id error!");
    }
    return !!lwrb_get_full(&KEY_buff);
}

void USBDevWakeup(void)
{
    LOG_INFO("USB wake host\n");
    R16_PIN_ANALOG_IE &= ~RB_PIN_USB_DP_PU;
    R8_UDEV_CTRL |= RB_UD_LOW_SPEED;
    mDelaymS(2);
    R16_PIN_ANALOG_IE |= RB_PIN_USB_DP_PU;
    R8_UDEV_CTRL &= ~RB_UD_LOW_SPEED;

}

void usbtest(void){
    uint8_t testbuf[16] = {0};
#if 0
    static bool flag = true;
    flag?(testbuf[0] = 1,flag = false):(testbuf[0]=0,flag = true);
    if(!(EpnInFlag & 1<<1)) return ;

    memcpy(EP2_Databuf+64, testbuf, 16);
    EpnInFlag &= ~(1 << 0);
    PFIC_DisableIRQ(USB_IRQn);
    R8_UEP2_T_LEN = 16;
    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
    PFIC_EnableIRQ(USB_IRQn);
#endif


    static bool flag = true;
    if(!(EpnInFlag & 1<<1)) return ;
    flag?(testbuf[1] = 1,flag = false):(testbuf[0]=0,flag = true);



    memcpy(EP2_Databuf+64, testbuf, 4);
    EP2_Databuf[64] = SYS_ID;

    PRINT("SYS ID IN:[");
    for(int i = 0; i < 3; i++){
        if(i) PRINT(" ");
        PRINT("%#x", EP2_Databuf[64+i]);
    }PRINT("]\n");

    PFIC_DisableIRQ(USB_IRQn);
    EpnInFlag &= ~(1 << 1);
    R8_UEP2_T_LEN = 3;
    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
    PFIC_EnableIRQ(USB_IRQn);

}

void usb_disable(void)
{
    R16_PIN_ANALOG_IE &= ~RB_PIN_USB_DP_PU;
    R8_UDEV_CTRL &= ~(RB_UD_PD_DIS | RB_UD_PORT_EN);
    GPIOB_ModeCfg( GPIO_Pin_10 | GPIO_Pin_11, GPIO_ModeIN_PD);
    PFIC_DisableIRQ(USB_IRQn);
}


void USB_Init(void){

    pEP0_RAM_Addr = EP0_Databuf;
    pEP1_RAM_Addr = EP1_Databuf;
    pEP2_RAM_Addr = EP2_Databuf;
    pEP3_RAM_Addr = EP3_Databuf;
    USB_DeviceInit();

    PFIC_EnableIRQ(USB_IRQn);

}
