#include "HAL_FLASH/include/easyflash.h"
#include "device_config.h"
#include "HAL.h"
#include "CH58x_common.h"
#include "hidkbd.h"
#include "hiddev.h"
#include "USB_task/USB_kbd.h"
#include "RF_task/rf_task.h"
#include <string.h>
#include "test_dtm_port.h"

__attribute__((aligned(4))) uint8_t device_mode;
__attribute__((aligned(4))) Device_bond_t device_bond;
__attribute__((aligned(4))) Led_info_t device_led;
__attribute__((aligned(4))) RF_Pair_Info_t device_pair;


uint8_t SaveDeviceInfo( char * device_info )
{
    uint8_t ret = SUCCESS;
    bool is_all = false;

    if(strcmp(device_info, "all") == 0){
        is_all = true;
    }

    if(is_all || (strcmp(device_info, "device_mode") == 0))
        ret |= ef_set_env_blob("device_mode", &device_mode, sizeof(device_mode));
    if(is_all || (strcmp(device_info, "device_bond") == 0))
        ret |= ef_set_env_blob("device_bond", &device_bond, sizeof(device_bond));
    if(is_all || (strcmp(device_info, "device_led") == 0))
        ret |= ef_set_env_blob("device_led", &device_led, sizeof(device_led));
    if(is_all || (strcmp(device_info, "device_pair") == 0))
        ret |= ef_set_env_blob("device_pair", &device_pair, sizeof(device_pair));

    return ret;
}


void ResetDeviceInfo(char * device_info)
{
    bool is_all = false;

    if(strcmp(device_info, "all") == 0){
        SaveDeviceInfo("all");
        return ;
    }
    if(strcmp(device_info, "device_mode") == 0){
        device_mode = MODE_BLE;
        SaveDeviceInfo(device_info);
    }

    if(strcmp(device_info, "device_bond") == 0){
        device_bond.ID_Num = 0;
        device_bond.ID[0].Direct_flag = 0;
        device_bond.ID[1].Direct_flag = 0;
        device_bond.ID[2].Direct_flag = 0;
        device_bond.ID[3].Direct_flag = 0;
        SaveDeviceInfo(device_info);
    }

    if(strcmp(device_info, "device_led") == 0){
        device_led.led_en = 0;
        device_led.led_level = 48;
        SaveDeviceInfo(device_info);
    }

    if(strcmp(device_info, "device_pair") == 0){
        device_pair.Pair_flag = 0;
        device_pair.Mac_addr = DEFAULT_MAC;
        SaveDeviceInfo(device_info);
    }
}

void ReadDeviceInfo( char * device_info )
{
    bool is_all = false;

    if(strcmp(device_info, "all") == 0){
        is_all = true;
    }

    if(is_all || (strcmp(device_info, "device_mode") == 0))
    if(ef_get_env_blob("device_mode", &device_mode, sizeof(device_mode), NULL) == 0){
        device_mode = MODE_BLE;
        SaveDeviceInfo("device_mode");
    }

    if(is_all || (strcmp(device_info, "device_bond") == 0))
    if(ef_get_env_blob("device_bond", &device_bond, sizeof(device_bond), NULL) == 0){
        device_bond.ID_Num = 0;
        device_bond.ID[0].Direct_flag = 0;
        device_bond.ID[1].Direct_flag = 0;
        device_bond.ID[2].Direct_flag = 0;
        device_bond.ID[3].Direct_flag = 0;
        SaveDeviceInfo("device_bond");
    }

    if(is_all || (strcmp(device_info, "device_led") == 0))
    if(ef_get_env_blob("device_led", &device_led, sizeof(device_led), NULL) == 0){
        device_led.led_en = 0;
        device_led.led_level = 48;
        SaveDeviceInfo("device_led");
    }

    if(is_all || (strcmp(device_info, "device_pair") == 0))
    if(ef_get_env_blob("device_pair", &device_pair, sizeof(device_pair), NULL) == 0){
        device_pair.Pair_flag = 0;
        device_pair.Mac_addr = DEFAULT_MAC;
        SaveDeviceInfo("device_pair");
    }

    LOG_INFO("my mac addr=%#x", device_pair.Mac_addr);
}


void Mode_Init(uint8_t mod)
{
    CH57X_BLEInit( );
    HAL_Init( );
    switch (mod) {
        case MODE_BLE:
            GAPRole_PeripheralInit( );
            HidDev_Init( );
            HidEmu_Init( );

            break;

        case MODE_RF24:
            RF_RoleInit( );
            RFtaskInit();
            break;

        case MODE_USB:
            USB_Task_Init();
            break;

        case MODE_TSET:
            RF_RoleInit( );
            test_dtm_usb_init();
            tmos_stop_task(halTaskID, HAL_SLEEP_EVENT);
            tmos_stop_task(halTaskID, HAL_SHUTDOWN_EVENT);
            break;

        default:
            break;

    }
}
