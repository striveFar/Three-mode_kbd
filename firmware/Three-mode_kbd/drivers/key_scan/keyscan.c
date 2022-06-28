#include "keyscan.h"
#include "CH58x_common.h"
#include "RingBuffer/lwrb.h"
#include "config.h"
#include "device_config.h"
#include "RF_task/rf_task.h"
#include "HAL/HAL.h"
#include "USB/usbuser.h"

#include "backlight/backlight.h"

const uint8_t keytale_8b[] =                    //默认的键码表
{
//        R0   R1   R2   R3   R4   R5
        0x00, 0x29, 0x35, 0x2b, 0x39, 0x02, 0x01, //c0                 //esc           ~`              tab         caps-lock  shift-L      ctr-l
        0x00, 0x3a, 0x1e, 0x14, 0x04, 0x1d, 0x08, //c1                 //F1            1!              Q           A          Z            win-l
        0x00, 0x3b, 0x1f, 0x1a, 0x16, 0x1b, 0x04, //c2                 //f2            2@              W           S          X            alt-l
        0x00, 0x3c, 0x20, 0x08, 0x07, 0x06, 0x2c, //c3                 //F3            3#              E           D          C            Space
        0x00, 0x3d, 0x21, 0x15, 0x09, 0x19, 0x40, //c4                 //F4            4$              R           F          V            alt-r
        0x00, 0x3e, 0x22, 0x17, 0x0a, 0x05, 0xfe, //c5                 //F5            5%              T           G          B            Fn

        0x00, 0x3f, 0x23, 0x1c, 0x0b, 0x11, 0x10, //c6                 //F6            6^              Y           H          N
        0x00, 0x40, 0x24, 0x18, 0x0d, 0x10, 0x80, //c7                 //F7            7&              U           J          M            win-r
        0x00, 0x41, 0x25, 0x0c, 0x0e, 0x36, 0x10, //c8                 //F8            8*              I           K          <,           ctr-r
        0x00, 0x42, 0x26, 0x12, 0x0f, 0x37, 0x00, //c9                 //F9            9               O           L          >.
        0x00, 0x43, 0x27, 0x13, 0x33, 0x38, 0x00, //c10                //F10           0               P           ;:         /?
        0x00, 0x44, 0x2d, 0x2f, 0x34, 0x20, 0x00, //c11                //F11           -_              [{          '"         shift-r
        0x00, 0x45, 0x2e, 0x30, 0x31, 0x00, 0x00, //c12                //F12           =+              ]}          \|

        0x00, 0x00, 0x2a, 0x00, 0x28, 0x00, 0x00, //c13                //0x00          Backspace                   Enter-R
        0x00, 0x46, 0x49, 0x4c, 0x00, 0x00, 0x50, //c14                //Print-screen  Insert          Delete      0x00,       0x00,       左
        0x00, 0x47, 0x4a, 0x4d, 0x00, 0x52, 0x51, //c15                //Scroll-Lock   Home            End         0x00,       上                           下
        0x00, 0x48, 0x4b, 0x4e, 0x00, 0x00, 0x4f, //c16                //Pause         Page-Up         Page-Down   0x00        0x00        右
        0x00, 0x00, 0x53, 0x5f, 0x5c, 0x59, 0x00, //c17                //Backlight     Num-lock        7HOME       4(小键盘)   1End       0x00
        0x00, 0x00, 0x54, 0x60, 0x5d, 0x5a, 0x62, //c18                //Locking       /               8(小键盘)   5(小键盘)   2(小键盘)   0Ins
        0x00, 0x00, 0x55, 0x61, 0x5e, 0x5b, 0x63, //c19                //0x00          *               9Pgup       6(小键盘)   3PgDn       =del
        0x00, 0x00, 0x56, 0x57, 0x00, 0x00, 0x58 //c20                 //0x00          -               +           0x00        0x00        Enter-R2
};


void index2keyVal_8(uint8_t *index, uint8_t *keyVal, uint8_t len);
void index2keyVal_16(uint8_t *index, uint8_t *keyVal, uint8_t len);

void keyInit(void)
{
    GPIOA_ModeCfg(
    GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5,
            GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeOut_PP_5mA);
    GPIOB_ModeCfg(
             GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 |
            GPIO_Pin_14 | GPIO_Pin_15 | GPIO_Pin_16 | GPIO_Pin_17 | GPIO_Pin_18 |
            GPIO_Pin_19 | GPIO_Pin_20 | GPIO_Pin_21 | GPIO_Pin_22 | GPIO_Pin_23
            , GPIO_ModeOut_PP_5mA);
}

const uint32_t IOmap[] = {1<<4, 1<<5, 1<<6, 1<<7, 1<<14, 1<<15, 1<<16, 1<<17, 1<<8, 1<<9, 1<<8, 1<<18, 1<<19, 1<<20, 1<<21, 1<<22, 1<<23};


void RstAllPins(void)
{
    for(uint8_t i = 0; i < 17; i++){
          i == 10 ? GPIOA_ResetBits(IOmap[i]): GPIOB_ResetBits(IOmap[i]);
    }
}

__HIGH_CODE
void keyScan(uint8_t *pbuf, uint8_t *key_num)
{
    uint8_t KeyNum;
    static uint8_t secbuf[120];

    uint8_t firstbuf[120] = { 0 };  //每一次扫描 firstbuf复位为0

    KeyNum = 0;

    for(uint8_t i = 0; i < 17; i++){
        i == 10 ? GPIOA_ResetBits(IOmap[i]): GPIOB_ResetBits(IOmap[i]);
        __nop();__nop();  //由于上拉输入拉低需要一定的时间，所以必须延时一段时间再读IO
        {
            if (Key_S0 == 0) {
                firstbuf[KeyNum++] = i * 7 + 1;
            }
            if (Key_S1 == 0) {
                firstbuf[KeyNum++] = i * 7 + 2;
            }
            if (Key_S2 == 0) {
                firstbuf[KeyNum++] = i * 7 + 3;
            }
            if (Key_S3 == 0) {
                firstbuf[KeyNum++] = i * 7 + 4;
            }
            if (Key_S4 == 0) {
                firstbuf[KeyNum++] = i * 7 + 5;
            }
            if (Key_S5 == 0) {
                firstbuf[KeyNum++] = i * 7 + 6;
            }
        }
        i == 10 ? GPIOA_SetBits(IOmap[i]): GPIOB_SetBits(IOmap[i]);

        while(!(Key_S0 && Key_S1 && Key_S2 && Key_S3 && Key_S4 && Key_S5)) {
            continue;
        }
    }

    //这一次与上一次键值相等 去抖动作用
    if (tmos_memcmp(firstbuf, secbuf, sizeof(firstbuf)) == true)
            {
        tmos_memcpy(pbuf, secbuf, sizeof(firstbuf));
        *key_num = KeyNum;
    }

    tmos_memcpy(secbuf, firstbuf, sizeof(firstbuf));

}

void index2keyVal_8(uint8_t *index, uint8_t *keyVal, uint8_t len)
{

    for (int b = 0, idx = 0; b < len; b++) {
        if(!keytale_8b[index[b]]) continue;
        keyVal[2 + idx++] = keytale_8b[index[b]];
    }
}

void index2keyVal_16(uint8_t *index, uint8_t *keyVal, uint8_t len)
{

    for (int b = 0; b < len; b++) {
        if (!(index[b] && keytale_8b[index[b]]))
            continue;

        uint8_t general_code = keytale_8b[index[b]] - 4;
        uint8_t code_num = general_code / 8 + 1;
        uint8_t keyval = 1 << (general_code % 8);
//         LOG_INFO("index=%d gc=%#x, cn=%d, kv=%#x ",index[b], general_code, code_num, keyval);

        keyVal[code_num] |= keyval;
    }
}


bool hotkeydeal(uint8_t *index, uint8_t *keys, uint8_t nums)
{
    bool ret = false;
    for (int i = 0; i < nums; i++) {
        if (index[i] == 5)             //shift-l
                {
            keys[0] |= keytale_8b[index[i]];
            index[i] = 0;
            ret = true;
        }
        if (index[i] == 82)             //shift-r
                {
            keys[0] |= keytale_8b[index[i]];
            index[i] = 0;
            ret = true;
        }
        if (index[i] == 6)             //ctrl-l
                {
            keys[0] |= keytale_8b[index[i]];
            index[i] = 0;
            ret = true;
        }
        if (index[i] == 13)             //winl
                {
            keys[0] |= keytale_8b[index[i]];
            index[i] = 0;
            ret = true;
        }
        if (index[i] == 20)             //altl
                {
            keys[0] |= keytale_8b[index[i]];
            index[i] = 0;
            ret = true;
        }
        if (index[i] == 34)             //altr
                {
            keys[0] |= keytale_8b[index[i]];
            index[i] = 0;
            ret = true;
        }
        if (index[i] == 55)             //winr
                {
            keys[0] |= keytale_8b[index[i]];
            index[i] = 0;
            ret = true;
        }
        if (index[i] == 62)             //ctrr
                {
            keys[0] |= keytale_8b[index[i]];
            index[i] = 0;
            ret = true;
        }

        if (index[i] == 41)             //Fn
                {
            keys[0] = keytale_8b[index[i]];
            index[i] = 0;
        }
    }

    return ret;

}

uint8_t Volume_Decre_Flag;
uint8_t Volume_Incre_Flag;
uint8_t Volume_Mute_Flag;

uint8_t LED_Enhance_flag;
uint8_t LED_Weaken_flag;
uint8_t LED_Switch_flag;

uint8_t Fn_1_Flag;
uint8_t Fn_2_Flag;
uint8_t Fn_3_Flag;
uint8_t Fn_4_Flag;

bool SpecialKey(uint8_t *keyval)             //Fn键
{
    if ( keyval[0] == 0xfe ) {
        switch ( keyval[2] ) {
        case 0x22:      //FN+5
            PRINT("FN+5\n");
            if (device_mode == MODE_RF24) {
                PRINT("Pair function enable.\n");
                OnBoard_SendMsg(RFtaskID, RF_PAIR_MESSAGE, 1, NULL);
            }
            break;
        case 0x27:      //FN+0
            PRINT("FN+0\n");
            if (device_mode == MODE_TSET) {  //just for test, should not use hotkey
                break;
            }
            device_mode = MODE_TSET;
            SaveDeviceInfo("device_mode");

            SYS_ResetExecute();   //软复位
            break;

        case 0x46:      //FN+Insert
            PRINT("FN+Print-screen\n");
            if (device_mode == MODE_BLE) {
                break;
            }
            device_mode = MODE_BLE;
            SaveDeviceInfo("device_mode");

            SYS_ResetExecute();   //软复位

            break;

        case 0x47:      //FN+Scroll-Lock
            PRINT("FN+Scroll-Lock\n");
            if (device_mode == MODE_RF24) {
                break;
            }

            device_mode = MODE_RF24;
            SaveDeviceInfo("device_mode");

            SYS_ResetExecute();   //软复位

            break;

        case 0x48:      //FN+Pause
            PRINT("FN+Pause\n");

            if (device_mode == MODE_USB) {
                break;
            }

            device_mode = MODE_USB;
            SaveDeviceInfo("device_mode");

            SYS_ResetExecute();   //软复位

            break;

        case 0x3a:
            PRINT("Fn+F1\n");
            Volume_Mute_Flag = 1;

            break;

        case 0x3b:
            PRINT("Fn+F2\n");
            Volume_Decre_Flag = 1;

            break;

        case 0x3c:
            PRINT("Fn+F3\n");
            Volume_Incre_Flag = 1;
            break;

        case 0x3d:
            PRINT("Fn+F4\n");

            break;
        case 0x3e:      //Fn+F5
            PRINT("Fn+F5\n");

            break;

        case 0x3f:      //Fn+F6
            PRINT("Fn+F6\n");
            LED_Enhance_flag = 1;
            break;

        case 0x40:      //Fn+F7
            PRINT("Fn+F7\n");
            LED_Weaken_flag = 1;
            break;
        case 0x41:
            PRINT("Fn+F8\n");
            LED_Switch_flag = 1;
            break;

        case 0x1e:
            PRINT("Fn+1\n");
            Fn_1_Flag = 1;
            break;
        case 0x1f:
            PRINT("Fn+2\n");
            Fn_2_Flag = 1;
            break;

        case 0x20:
            PRINT("Fn+3\n");
            Fn_3_Flag = 1;
            break;

        case 0x21:
            PRINT("Fn+4\n");
            Fn_4_Flag = 1;
            break;

        case 0:
            PRINT("clear\n");
            Volume_Decre_Flag = 0;
            Volume_Mute_Flag = 0;
            Volume_Incre_Flag = 0;
            LED_Enhance_flag = 0;
            LED_Weaken_flag = 0;
            LED_Switch_flag = 0;
            Fn_1_Flag = 0;
            Fn_2_Flag = 0;
            Fn_3_Flag = 0;
            Fn_4_Flag = 0;
            break;
        default:
            break;

        }
        return true;
    }
    return false;
}

bool SpecialKey_Deal(void) {
    static uint32_t Fn_1_time = 0;
    static uint32_t Fn_2_time = 0;
    static uint32_t Fn_3_time = 0;
    static uint32_t Fn_4_time = 0;

    static uint8_t flag = 0;
    bool ret = false;
    if (device_mode == MODE_BLE) {
        static UINT8 state = 0;

        if (Fn_1_Flag == 1) {
            state = 1;
            flag = 1;
        }
        if (Fn_2_Flag == 1) {
            state = 2;
            flag = 2;
        }
        if (Fn_3_Flag == 1) {
            state = 3;
            flag = 3;
        }
        if (Fn_4_Flag == 1) {
            state = 4;
            flag = 4;
        }

        if (flag == 1 && Fn_1_Flag == 0) {
            state = 5;
            flag = 0;
        }
        if (flag == 2 && Fn_2_Flag == 0) {
            state = 6;
            flag = 0;
        }
        if (flag == 3 && Fn_3_Flag == 0) {
            state = 7;
            flag = 0;
        }
        if (flag == 4 && Fn_4_Flag == 0) {
            state = 8;
            flag = 0;
        }

        LOG_INFO("special state:%d", state);

        switch (state) {
        case 0: {

        }
            break;

        case 1: {
            Fn_1_time = get_current_time();
        }
            break;

        case 2: {
            Fn_2_time = get_current_time();
        }
            break;

        case 3: {
            Fn_3_time = get_current_time();
        }
            break;

        case 4: {
            Fn_4_time = get_current_time();
        }
            break;

        case 5: {
                state = 0;
                uint32_t current_time = get_current_time() - Fn_1_time;
                if (current_time >= LONGKEY_TIME) {
                    Fn_1_time = 0;
                    PRINT("Long\n");
                    device_bond.ID_Num = 0;
                    device_bond.ID[0].Direct_flag = 0;
                    SaveDeviceInfo("device_bond");

                    mDelaymS(10);
                    SYS_ResetExecute();   //软复位

                } else if (current_time > SHORTKEY_TIME) {
                    Fn_1_time = 0;
                    PRINT(" Short\n");
                    if (device_bond.ID_Num == 0) {
                        break;
                    } else {
                        device_bond.ID_Num = 0;
                        device_bond.ID[0].Direct_flag = 1;
                        SaveDeviceInfo("device_bond");

                        mDelaymS(10);
                        SYS_ResetExecute();   //软复位

                    }
                } else{
                    PRINT("Fn+1 time error: %d\n", current_time);
                }
        }
            break;

        case 6: {
                state = 0;
                uint32_t current_time = get_current_time() - Fn_2_time;
                if (current_time >= LONGKEY_TIME) {
                    Fn_2_time = 0;
                    PRINT("Long\n");
                    device_bond.ID_Num = 1;
                    device_bond.ID[1].Direct_flag = 0;
                    SaveDeviceInfo("device_bond");

                    mDelaymS(10);
                    SYS_ResetExecute();   //软复位

                } else if (current_time > SHORTKEY_TIME) {
                    Fn_2_time = 0;
                    PRINT(" Short\n");
                    if (device_bond.ID_Num == 1) {
                        break;
                    } else {
                        device_bond.ID_Num = 1;
                        device_bond.ID[1].Direct_flag = 1;
                        SaveDeviceInfo("device_bond");

                        mDelaymS(10);
                        SYS_ResetExecute();   //软复位

                    }
                } else {
                    PRINT("Fn+2 time error: %d\n", current_time);
                }
        }
            break;

        case 7: {
                state = 0;
                uint32_t current_time = get_current_time() - Fn_3_time;
                if (current_time >= LONGKEY_TIME) {
                    Fn_3_time = 0;
                    PRINT("Long\n");
                    device_bond.ID_Num = 2;
                    device_bond.ID[2].Direct_flag = 0;
                    SaveDeviceInfo("device_bond");

                    mDelaymS(10);
                    SYS_ResetExecute();   //软复位

                } else if (current_time > SHORTKEY_TIME) {
                    Fn_3_time = 0;
                    PRINT(" Short\n");
                    if (device_bond.ID_Num == 2) {
                        break;
                    } else {
                        device_bond.ID_Num = 2;
                        device_bond.ID[2].Direct_flag = 1;
                        SaveDeviceInfo("device_bond");

                        mDelaymS(10);
                        SYS_ResetExecute();   //软复位

                    }
                } else {
                    PRINT("Fn+3 time error: %d\n", current_time);
                }
        }
            break;

        case 8: {
                state = 0;
                uint32_t current_time = get_current_time() - Fn_4_time;
                if (current_time >= LONGKEY_TIME) {
                    Fn_4_time = 0;
                    PRINT("Long\n");
                    device_bond.ID_Num = 3;
                    device_bond.ID[3].Direct_flag = 0;
                    SaveDeviceInfo("device_bond");

                    mDelaymS(10);
                    SYS_ResetExecute();   //软复位

                } else if (current_time > SHORTKEY_TIME) {
                    Fn_4_time = 0;
                    PRINT(" Short\n");
                    if (device_bond.ID_Num == 3) {
                        break;
                    } else {
                        device_bond.ID_Num = 3;
                        device_bond.ID[3].Direct_flag = 1;
                        SaveDeviceInfo("device_bond");

                        mDelaymS(10);
                        SYS_ResetExecute();   //软复位
                    }
                } else {
                    PRINT("Fn+4 time error: %d\n", current_time);
                }
        }
            break;

        default:
            break;

        }

    }


    {
        if(LED_Enhance_flag){
            LED_Enhance_flag = 0;
            if(enhance_bk(BK_LINEALL))
                SaveDeviceInfo("device_led");
        }

        if(LED_Weaken_flag){
            LED_Weaken_flag = 0;
            if(weaken_bk(BK_LINEALL))
                SaveDeviceInfo("device_led");
        }

        if(LED_Switch_flag){
            LED_Switch_flag = 0;
            if(device_led.led_en){
                device_led.led_en = false;
                disbale_bk(BK_LINEALL);
                SaveDeviceInfo("device_led");
            } else{
                device_led.led_en = true;
                set_bk(BK_LINEALL, device_led.led_level);
                SaveDeviceInfo("device_led");
            }
        }
    }


    {
        static UINT8 state;
        uint8_t BUF[2];
        uint8_t report_id = CONSUME_ID;
        switch (state) {
        case 0: {

            if (Volume_Incre_Flag == 1) {
                state = 1;

                PRINT("Volume_Incre\n");

                BUF[0] = 0xe9;
                BUF[1] = 0;
                lwrb_write(&KEY_buff, &report_id, 1);
                size_t wl = lwrb_write(&KEY_buff, BUF, 2);
                if(wl != 2)
                    lwrb_skip(&KEY_buff, wl);
                ret = true;
            }

            if (Volume_Decre_Flag == 1) {
                state = 2;
                PRINT("Volume_Decre\n");
                BUF[0] = 0xea;
                BUF[1] = 0;
                lwrb_write(&KEY_buff, &report_id, 1);
                size_t wl = lwrb_write(&KEY_buff, BUF, 2);
                if(wl != 2)
                    lwrb_skip(&KEY_buff, wl);
                ret = true;
            }

            if (Volume_Mute_Flag == 1) {
                state = 3;
                PRINT("Volume_Mute\n");

                BUF[0] = 0xe2;
                BUF[1] = 0;
                lwrb_write(&KEY_buff, &report_id, 1);
                size_t wl = lwrb_write(&KEY_buff, BUF, 2);
                if(wl != 2)
                    lwrb_skip(&KEY_buff, wl);
                ret = true;
            }

        }
            break;

        case 1: {
            if (Volume_Incre_Flag == 0) {
                state = 0;
                BUF[0] = 0;
                BUF[1] = 0;
                lwrb_write(&KEY_buff, &report_id, 1);
                size_t wl = lwrb_write(&KEY_buff, BUF, 2);
                if(wl != 2)
                    lwrb_skip(&KEY_buff, wl);
                ret = true;
            }

        }
            break;

        case 2: {
            if (Volume_Decre_Flag == 0) {
                state = 0;
                BUF[0] = 0;
                BUF[1] = 0;
                lwrb_write(&KEY_buff, &report_id, 1);
                size_t wl = lwrb_write(&KEY_buff, BUF, 2);
                if(wl != 2)
                    lwrb_skip(&KEY_buff, wl);
                ret = true;
            }

        }
            break;

        case 3: {
            if (Volume_Mute_Flag == 0) {
                state = 0;
                BUF[0] = 0;
                BUF[1] = 0;
                lwrb_write(&KEY_buff, &report_id, 1);
                size_t wl = lwrb_write(&KEY_buff, BUF, 2);
                if(wl != 2)
                    lwrb_skip(&KEY_buff, wl);
                ret = true;
            }

        }
            break;

        default: {
            state = 0;
        }
            break;
        }

    }
    return ret;
}

bool readKeyVal(void) {

    static uint8_t current_key_index[120] = { 0 };
    static uint8_t last_key_index[120] = { 0 };
    uint8_t key_num = 0;
    uint8_t save_key16[16] = { 0 };
    uint8_t save_key8[8] = { 0 };

    keyScan(current_key_index, &key_num);

    if (tmos_memcmp(current_key_index, last_key_index,
            sizeof(current_key_index)) == true) { //无键值变化
        return false;
    }
    tmos_memcpy(last_key_index, current_key_index, sizeof(current_key_index));

#define  KEY_MODE    8

#if KEY_MODE==8
    //hotkey deal
    hotkeydeal(current_key_index, save_key8, key_num);

    index2keyVal_8(current_key_index, save_key8, key_num);

    PRINT("key=[");
    for(int i = 0; i < 8; i++){
        if(i) PRINT(" ");
        PRINT("%#x", save_key8[i]);
    }PRINT("]\n");

    static bool isFnpress = false;
    if(SpecialKey(save_key8)){  //FN键后一个normal键一定是释放键
        isFnpress = true;
        return SpecialKey_Deal();
    } else{
        if(isFnpress) {
            isFnpress = false;
            return false;  // no keys change
        }
    }

    uint8_t report_id = KEYNORMAL_ID;
    lwrb_write(&KEY_buff, &report_id, 1);
    size_t wl = lwrb_write(&KEY_buff, save_key8, 8);
    if(wl != 8)
        lwrb_skip(&KEY_buff, wl);


#elif KEY_MODE==16
    index2keyVal_16(current_key_index, save_key16, key_num);
    uint8_t report_id = KEYBIT_ID;
    lwrb_write(&KEY_buff, &report_id, 1);
    size_t wl = lwrb_write(&KEY_buff, save_key16, 16);
    if(wl != 16)
        lwrb_skip(&KEY_buff, wl);
#endif

    return true;
}

