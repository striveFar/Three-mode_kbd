#include <rf_task/rf_task.h>
#include "HAL_FLASH/include/easyflash.h"
#include "device_config.h"
#include "HAL.h"
#include "CH58x_common.h"
#include "hidkbd.h"
#include "hiddev.h"
#include "USB_task/USB_kbd.h"
#include "test_dtm_port.h"

__attribute__((aligned(4))) RF_Pair_Info_t device_pair;
__attribute__((aligned(4))) uint8_t device_mode;


uint8_t SaveDeviceInfo(char * device_info)
{
    uint8_t ret = SUCCESS;
    bool is_all = false;

    if(strcmp(device_info, "all") == 0){
        is_all = true;
    }

    if(is_all || (strcmp(device_info, "device_pair") == 0))
        ret |= ef_set_env_blob("device_pair", &device_pair, sizeof(device_pair));
    if(is_all || (strcmp(device_info, "device_mode") == 0))
        ret |= ef_set_env_blob("device_mode", &device_mode, sizeof(device_mode));

    return ret;
}

void ResetDeviceInfo(char * device_info)
{
    if(strcmp(device_info, "all") == 0){
        SaveDeviceInfo("all");
    }
    if(strcmp(device_info, "device_pair") == 0){
        device_pair.Pair_flag = 0;
        device_pair.Mac_addr = DEFAULT_MAC;
        SaveDeviceInfo(device_info);
    }
    if(strcmp(device_info, "device_mode") == 0){
        device_mode = MODE_RF24;
        SaveDeviceInfo(device_info);
    }
}

void ReadDeviceInfo(char * device_info)
{
    bool is_all = false;

    if(strcmp(device_info, "all") == 0){
        is_all = true;
    }

    if(is_all || (strcmp(device_info, "device_pair") == 0))
    if(ef_get_env_blob("device_pair", &device_pair, sizeof(device_pair), NULL) == 0){
        device_pair.Pair_flag = 0;
        device_pair.Mac_addr = DEFAULT_MAC;
        SaveDeviceInfo("device_pair");
    }

    if(is_all || (strcmp(device_info, "device_mode") == 0))
    if(ef_get_env_blob("device_mode", &device_mode, sizeof(device_mode), NULL) == 0){
        device_mode = MODE_RF24;
        SaveDeviceInfo("device_mode");
    }

    LOG_INFO("my mac addr=%#x", device_pair.Mac_addr);
}

void Mode_Init(uint8_t mod)
{
    CH57X_BLEInit();
    HAL_Init();

    switch (mod) {
        case MODE_RF24:
            RF_RoleInit();
            rf_task_init();
            USB_Task_Init();
            break;

        case MODE_TSET:
            RF_RoleInit( );
            test_dtm_usb_init();
            break;

        default:
            RF_RoleInit();
            rf_task_init();
            USB_Task_Init();
            break;
    }
}
