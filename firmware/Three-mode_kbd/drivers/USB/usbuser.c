/******************************************************************************/
/* 头文件包含 */
#include "CH58x_common.h"
#include "usbuser.h"
#include "RingBuffer/lwrb.h"
#include "HAL/HAL.h"
#include "backlight/backlight.h"
#include "../../../drivers/tmk_core/common/report.h"
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
 * Description    : CH582USB中断处理函数
 *******************************************************************************/
__HIGH_CODE void USB_IRQHandler(void)                   //USB中断服务程序
{
    UINT8 len, chtype;
    UINT8 intflag, errflag = 0;

    intflag = R8_USB_INT_FG;
    if (intflag & RB_UIF_TRANSFER) {
        if (( R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN)    // 非空闲
        {
            switch ( R8_USB_INT_ST & ( MASK_UIS_TOKEN | MASK_UIS_ENDP)) {
            case UIS_TOKEN_IN:
                switch (SetupReqCode) {
                case USB_GET_DESCRIPTOR:
                    len = SetupReqLen >= USB_EP0_PACKET_SIZE ? USB_EP0_PACKET_SIZE : SetupReqLen; // 本次传输长度
                    memcpy( pEP0_DataBuf, pDescr, len); /* 加载上传数据 */
                    SetupReqLen -= len;
                    pDescr += len;
                    R8_UEP0_T_LEN = len;
                    R8_UEP0_CTRL ^= RB_UEP_T_TOG;                          // 翻转
                    break;
                case USB_SET_ADDRESS:
                    R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT)
                            | SetupReqLen;
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                default:
                    R8_UEP0_T_LEN = 0;            // 状态阶段完成中断或者是强制上传0长度数据包结束控制传输
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

                /*****端口1上传*****/
            case UIS_TOKEN_IN | 1:
                EpnInFlag |= 1 << 0;
                R8_UEP1_T_LEN = 0;
                //          R8_UEP1_CTRL ^= RB_UEP_T_TOG;                                          //如果不设置自动翻转则需要手动翻转
                R8_UEP1_CTRL =
                        (R8_UEP1_CTRL & ~ MASK_UEP_T_RES) | UEP_T_RES_NAK; //默认应答NAK
                break;
            case UIS_TOKEN_IN | 2:
                EpnInFlag |= 1 << 1;
                R8_UEP2_T_LEN = 0;
                R8_UEP2_CTRL =
                        (R8_UEP2_CTRL & ~ MASK_UEP_T_RES) | UEP_T_RES_NAK; //默认应答NAK
                break;
            case UIS_TOKEN_IN | 3:
                EpnInFlag |= 1 << 2;
                R8_UEP3_T_LEN = 0;
                R8_UEP3_CTRL =
                        (R8_UEP3_CTRL & ~ MASK_UEP_T_RES) | UEP_T_RES_NAK; //默认应答NAK
                break;

                /*****端口1下传*****/
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
        if ( R8_USB_INT_ST & RB_UIS_SETUP_ACT)                  // Setup包处理
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
            else /* 标准请求 */
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
                            pDescr = HID_ReportDescriptor;              //数据准备上传
                            len = HID_ReportDescSize;
                        } else if (((pSetupReqPak->wIndex) & 0xff) == 1) {
                            pDescr = HID_ReportDescriptor2;             //数据准备上传
                            len = HID_ReportDescSize2;
                        } else
                            len = 0xff;                   //本程序只有2个接口，这句话正常不可能执行
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
                            errflag = 0xFF;                        // 不支持的字符串描述符
                            break;
                        }
                        break;

                    default:
                        errflag = 0xff;
                        break;
                    }
                    if (SetupReqLen > len)
                        SetupReqLen = len;      //实际需上传总长度
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
                    if ((chtype & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE) //设备
                    {
                        if ((pSetupReqPak->wValue & 0xff) == USB_FEATURE_REMOTE_WAKEUP)   //远程唤醒
                        {
//                            wUSB_Status |= (mskREMOTE_WAKEUP | mskDEV_FEATURE_CMD | mskSETUP_IN);
                            LOG_INFO("USB host suspend\n");
                            USB_sus_status = true;
                        }
                    } else if ((chtype & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) {
                        /* 端点 */
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
                    if (( pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)    // 端点
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
                            errflag = 0xFF;                            // 不支持的端点
                            break;
                        }
                    } else if ((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE){
                        if (((pSetupReqPak->wValue) & 0xff) == USB_FEATURE_REMOTE_WAKEUP){
                            /* 远程唤醒 */
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
            if (errflag == 0xff)        // 错误或不支持
            {
//             SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;    // STALL
            } else {
                if (chtype & 0x80) {        //上传
                    len = (SetupReqLen > USB_EP0_PACKET_SIZE) ? USB_EP0_PACKET_SIZE :SetupReqLen;
                    SetupReqLen -= len;
                } else
                    len = 0;        // 下传
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;    // 默认数据包是DATA1
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
        if ( R8_USB_MIS_ST & RB_UMS_SUSPEND) {      // 挂起
//            wUSB_Status |= mskBUSSUSPEND;
        } else {
            ;
        }                             // 唤醒
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

    uint8_t report_id, key_len;

    lwrb_peek(&KEY_buff, 0, &report_id, 1);

    if (report_id == KEYNORMAL_ID || report_id == KEYBIT_ID) {
        key_len = KEYBOARD_REPORT_SIZE;
    } else if (report_id == SYS_ID) {
        key_len = sizeof(uint16_t);
    } else if (report_id == CONSUME_ID) {
        key_len = sizeof(uint16_t);
    } else if (report_id == VENDOR_ID) {
        //        key_len = 2;
    } else if (report_id == MOUSE_ID) {
        key_len = sizeof(report_mouse_t);
    } else {
        /* should not be here */
        LOG_DEBUG("report id error!");
        lwrb_reset(&KEY_buff);
    }

    if (report_id == KEYNORMAL_ID) {
        if (!(EpnInFlag & 1 << 0))
            return true;
    } else {
        if (!(EpnInFlag & 1 << 1))
            return true;
    }

    lwrb_skip(&KEY_buff, 1);
    lwrb_read(&KEY_buff, EP1_Databuf + 64, key_len);

#ifdef DEBUG
    switch (report_id) {
    case KEYNORMAL_ID:
        PRINT("USB keyboard IN:[");
        break;
    case KEYBIT_ID:
        PRINT("USB KEYBIT_ID IN:[");
        break;
    case SYS_ID:
        PRINT("USB SYS ID IN:[");
        break;
    case CONSUME_ID:
        PRINT("USB CONSUME_ID IN:[");
        break;
    case VENDOR_ID:
        PRINT("USB VENDOR_ID IN:[");
        break;
    case MOUSE_ID:
        PRINT("USB MOUSE_ID IN:[");
        break;
    default:
        LOG_DEBUG("USB :report id does not exist!");
        /* 唯一的限制是 goto 只能跳转到同一个函数中的某个标号处。 */
        goto err_out;

    }
    for (int i = 0; i < key_len; i++) {
        if (i)
            PRINT(" ");
        PRINT("%#x", EP2_Databuf[64 + i]);
    }
    PRINT("]\n");
    err_out:
#endif

    PFIC_DisableIRQ(USB_IRQn);
    if (report_id == KEYNORMAL_ID) {
        EpnInFlag &= ~(1 << 0);
        R8_UEP1_T_LEN = key_len;
        R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
    } else {
        EpnInFlag &= ~(1 << 1);
        R8_UEP2_T_LEN = key_len;
        R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
    }
    PFIC_EnableIRQ(USB_IRQn);

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
