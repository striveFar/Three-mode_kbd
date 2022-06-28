#ifndef __KEY_TABLE_H__
#define __KEY_TABLE_H__


#include <stdint.h>
//==========================================================
//KEY_TYPE DEFINE
//==========================================================
#define     MOUSE_PAGE          1
#define     KB_TYPE             2
#define     CON_TYPE            3
#define     SYS_TYPE            4
#define     FUNC_TYPE           5
#define     WEB_TYPE            6
#define     EXE_TYPE            7
#define     COM_TYPE            8

#define     KB_HOT              1
#define     KB_GENERAL          2


//==========================================================
//KEY_CODE_TAB
//==========================================================
#define     KEY_DEFAULT     0x00
#define     KEY_A           0x04
#define     KEY_B           0x05
#define     KEY_C           0x06
#define     KEY_D           0x07
#define     KEY_E           0x08
#define     KEY_F           0x09
#define     KEY_G           0x0A
#define     KEY_H           0x0B
#define     KEY_I           0x0C
#define     KEY_J           0x0d
#define     KEY_K           0x0e
#define     KEY_L           0x0f
#define     KEY_M           0x10
#define     KEY_N           0x11
#define     KEY_O           0x12
#define     KEY_P           0x13
#define     KEY_Q           0x14
#define     KEY_R           0x15
#define     KEY_S           0x16
#define     KEY_T           0x17
#define     KEY_U           0x18
#define     KEY_V           0x19
#define     KEY_W           0x1a
#define     KEY_X           0x1b
#define     KEY_Y           0x1c
#define     KEY_Z           0x1d

#define     KEY_1           0x1e
#define     KEY_2           0x1f
#define     KEY_3           0x20
#define     KEY_4           0x21
#define     KEY_5           0x22
#define     KEY_6           0x23
#define     KEY_7           0x24
#define     KEY_8           0x25
#define     KEY_9           0x26
#define     KEY_0           0x27
#define     KEY_ENTER       0x28
#define     KEY_ESC         0x29

#define     KEY_BACKSPACE   0x2a
#define     KEY_TAB         0x2b
#define     KEY_SPACE       0x2c
#define     KEY_DEC         0x2d
#define     KEY_ADD         0x2e
#define     KEY_BRACKET_L   0x2f    // [{
#define     KEY_BRACKET_R   0x30    // ]}
#define     KEY_SPRIT       0x31    // \|
#define     KEY_EUROPE      0x32
#define     KEY_SEMICOLON   0x33    // ;:
#define     KEY_QUOTES      0x34    // '"
#define     KEY_OVERDOT     0x35    // `~
#define     KEY_COMMA       0x36    // ,<
#define     KEY_PERIOD      0x37    // .>
#define     KEY_QUESTION    0x38    // /?
#define     KEY_CAP         0x39

#define     KEY_F1          0x3a
#define     KEY_F2          0x3b
#define     KEY_F3          0x3c
#define     KEY_F4          0x3d
#define     KEY_F5          0x3e
#define     KEY_F6          0x3f
#define     KEY_F7          0x40
#define     KEY_F8          0x41
#define     KEY_F9          0x42
#define     KEY_F10         0x43
#define     KEY_F11         0x44
#define     KEY_F12         0x45

#define     KEY_PRINT       0x46
#define     KEY_SCROLL      0x47
#define     KEY_PAUSE       0x48

#define     KEY_INSERT      0x49
#define     KEY_HOME        0x4a
#define     KEY_PGUP        0x4b
#define     KEY_DELETE      0x4c
#define     KEY_END         0x4d
#define     KEY_PGDN        0x4e

#define     KEY_RIGHT       0x4f
#define     KEY_LEFT        0x50
#define     KEY_DOWN        0x51
#define     KEY_UP          0x52

#define     KEY_NUM_LOCK    0x53
#define     KEY_NUM_SPRIT   0x54
#define     KEY_NUM_STAR    0x55
#define     KEY_NUM_DEC     0x56
#define     KEY_NUM_ADD     0x57
#define     KEY_NUM_ENTER   0x58
#define     KEY_NUM_1       0x59
#define     KEY_NUM_2       0x5a
#define     KEY_NUM_3       0x5b
#define     KEY_NUM_4       0x5c
#define     KEY_NUM_5       0x5d
#define     KEY_NUM_6       0x5e
#define     KEY_NUM_7       0x5f
#define     KEY_NUM_8       0x60
#define     KEY_NUM_9       0x61
#define     KEY_NUM_0       0x62
#define     KEY_NUM_DEL     0x63
#define     KEY_APP         0x65

#define     KEY_F13         0x68
#define     KEY_F14         0x69
#define     KEY_F15         0x6a
#define     KEY_F16         0x6b
#define     KEY_F17         0x6c
#define     KEY_F18         0x6d
#define     KEY_F19         0x6e
#define     KEY_F20         0x6f
#define     KEY_F21         0x70
#define     KEY_F22         0x71
#define     KEY_F23         0x72
#define     KEY_F24         0x73

#define     KEY_K29         0x31
#define     KEY_K42         0x32
#define     KEY_K45         0x64
#define     KEY_K107        0x85
#define     KEY_K56         0x87
#define     KEY_K14         0x89
#define     KEY_K132        0x8a
#define     KEY_K131        0x8b
#define     KEY_K151        0x90
#define     KEY_K150        0x91

//====================================================
//热键
//====================================================
#define     KEY_CTRL_L      0x01
#define     KEY_SHIFT_L     0x02
#define     KEY_ALT_L       0x04
#define     KEY_WIN_L       0x08
#define     KEY_CTRL_R      0x10
#define     KEY_SHIFT_R     0x20
#define     KEY_ALT_R       0x40
#define     KEY_WIN_R       0x80

//====================================================
//系统按键
//====================================================
#define     KEY_POWER       0x01
#define     KEY_SLEEP       0x02
#define     KEY_WAKEUP      0x04

//====================================================
//消费类
//====================================================
#define     CONSUME_NEXT        0X00B5
#define     CON_NEXT_L          0XB5
#define     CON_NEXT_H          0X00

#define     CONSUME_PREV        0X00B6
#define     CON_PREV_L          0XB6
#define     CON_PREV_H          0X00

#define     CONSUME_STOP        0X00B7
#define     CON_STOP_L          0XB7
#define     CON_STOP_H          0X00

#define     CONSUME_MUTE        0X00E2
#define     CON_MUTE_L          0XE2
#define     CON_MUTE_H          0X00

#define     CONSUME_VOL_UP      0X00E9
#define     CON_VOL_UP_L        0XE9
#define     CON_VOL_UP_H        0X00

#define     CONSUME_VOL_DOWN    0X00EA
#define     CON_VOL_DOWN_L      0XEA
#define     CON_VOL_DOWN_H      0X00

#define     CONSUME_PLAY        0X00CD
#define     CON_PLAY_L          0XCD
#define     CON_PLAY_H          0X00

#define     CONSUME_PLAY_STOP   0X00B7
#define     CON_PLAY_STOP_L     0XB7
#define     CON_PLAY_STOP_H     0X00

#define     CONSUME_MEDIA       0X0183
#define     CON_MEDIA_L         0X83
#define     CON_MEDIA_H         0X01

#define     CONSUME_MAIL        0X018A
#define     CON_MAIL_L          0X8A
#define     CON_MAIL_H          0X01

#define     CONSUME_CALCULATOR  0X0192
#define     CON_CALCULATOR_L    0X92
#define     CON_CALCULATOR_H    0X01

#define     CONSUME_COMPUTER    0X0194
#define     CON_COMPUTER_L      0X94
#define     CON_COMPUTER_H      0X01

#define     CONSUME_SEARCH      0X0221
#define     CON_SEARCH_L        0X21
#define     CON_SEARCH_H        0X02

#define     CONSUME_WWW_HOME    0X0223
#define     CON_WWW_HOME_L      0X23
#define     CON_WWW_HOME_H      0X02

#define     CONSUME_WWW_BACK    0X0224
#define     CON_WWW_BACK_L      0X24
#define     CON_WWW_BACK_H      0X02

#define     CONSUME_WWW_FORWARD 0X0225
#define     CON_WWW_FORWARD_L   0X25
#define     CON_WWW_FORWARD_H   0X02

#define     CONSUME_WWW_STOP    0X0226
#define     CON_WWW_STOP_L      0X26
#define     CON_WWW_STOP_H      0X02

#define     CONSUME_WWW_REFRESH 0X0227
#define     CON_WWW_REFRESH_L   0X27
#define     CON_WWW_REFRESH_H   0X02

#define     CONSUME_WWW_COLLECT 0X022A
#define     CON_WWW_COLLECT_L   0X2A
#define     CON_WWW_COLLECT_H   0X02
//====================================================
//自定义功能 0xe0 -- 0xff
//====================================================
#define     KEY_MACRO           0X01
#define     KEY_FN              0x02

#define     KEY_FUNCTION        0X05
#define     KEY_WIN_LOCK        0x06
#define     KEY_LED             0x07

#define     KEY_PAD_LOCK        0X08
#define     KEY_DISPLAY         0X09

#define     KEY_LIGHT_UP        0X0A
#define     KEY_LIGHT_DOWN      0X0B

#define     KEY_SPEED_DEC       0X0C
#define     KEY_SPEED_ADD       0X0D

#define     KEY_LED_SET_1       0x11
#define     KEY_LED_SET_2       0x12
#define     KEY_LED_SET_3       0x13
#define     KEY_LED_SET_4       0x14
#define     KEY_LED_SET_5       0x15
#define     KEY_LED_SET_6       0x16

#define     KEY_LED_DEFINE      0x17
#define     KEY_COLOUR_SET      0X18
#define     KEY_TREN_SET        0x19
#define     KEY_RESET           0x1a
#define     KEY_LOGO_SET        0x1B
#define     KEY_COLOUR          0x1C
#define     KEY_LED_OFF         0x1D

#define     LED_DEFINE_SET_0        0x1E
#define     LED_DEFINE_SET_1        0x1F
#define     LED_DEFINE_SET_2        0x20
#define     LED_DEFINE_SET_3        0x21
#define     LED_DEFINE_SET_4        0x22





#define     KEY_G_CLASS     0XA2
#define     KEY_G1          0X01
#define     KEY_G2          0X02
#define     KEY_G3          0X03
#define     KEY_G4          0X04
#define     KEY_G5          0X05
#define     KEY_G6          0X06
#define     KEY_G7          0X07
#define     KEY_G8          0X08
#define     KEY_G9          0X09
#define     KEY_REC         0X0A


#define     KEY_TEST            0X10


#define     KEY_DEFINE          0X0F



#endif
