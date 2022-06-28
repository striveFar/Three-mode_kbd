/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbdservice.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : ¼üÅÌ·þÎñ
 
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include <stddef.h>
#include "CONFIG.h"
#include "hidkbdservice.h"
#include "hiddev.h"
#include "battservice.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// HID service
const uint8 hidServUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(HID_SERV_UUID),
        HI_UINT16(HID_SERV_UUID) };

// HID Boot Keyboard Input Report characteristic
const uint8 hidBootKeyInputUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(
        BOOT_KEY_INPUT_UUID), HI_UINT16(BOOT_KEY_INPUT_UUID) };

// HID Boot Keyboard Output Report characteristic
const uint8 hidBootKeyOutputUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(
        BOOT_KEY_OUTPUT_UUID), HI_UINT16(BOOT_KEY_OUTPUT_UUID) };

// HID Information characteristic
const uint8 hidInfoUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(HID_INFORMATION_UUID),
        HI_UINT16(HID_INFORMATION_UUID) };

// HID Report Map characteristic
const uint8 hidReportMapUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(REPORT_MAP_UUID),
        HI_UINT16(REPORT_MAP_UUID) };

// HID Control Point characteristic
const uint8 hidControlPointUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(
        HID_CTRL_PT_UUID), HI_UINT16(HID_CTRL_PT_UUID) };

// HID Report characteristic
const uint8 hidReportUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(REPORT_UUID),
        HI_UINT16(REPORT_UUID) };

// HID Protocol Mode characteristic
const uint8 hidProtocolModeUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(
        PROTOCOL_MODE_UUID), HI_UINT16(PROTOCOL_MODE_UUID) };

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// HID Information characteristic value
static const uint8 hidInfo[HID_INFORMATION_LEN] = {
//  LO_UINT16(0x0111), HI_UINT16(0x0111),              // bcdHID (USB HID version)
        LO_UINT16(0x0101), HI_UINT16(0x0101),        // bcdHID (USB HID version)
        0x00,                                              // bCountryCode
        HID_FEATURE_FLAGS                                  // Flags
        };

// HID Report Map characteristic value
static const uint8 hidReportMap[] = {

        0x05,
        0x01,     // Usage Pg (Generic Desktop)
        0x09,
        0x06,     // Usage (Keyboard)
        0xA1,
        0x01,     // Collection: (Application)
        0x85,
        0x01,     //   ReportID................ (3)   jackson
                  //
        0x05,
        0x07,     // Usage Pg (Key Codes)
        0x19,
        0xE0,     // Usage Min (224)
        0x29,
        0xE7,     // Usage Max (231)
        0x15,
        0x00,     // Log Min (0)
        0x25,
        0x01,     // Log Max (1)
                  //
                  // Modifier byte
        0x75,
        0x01,     // Report Size (1)
        0x95,
        0x08,     // Report Count (8)
        0x81,
        0x02,     // Input: (Data, Variable, Absolute)
                  //
                  // Reserved byte
        0x95,
        0x01,     // Report Count (1)
        0x75,
        0x08,     // Report Size (8)
        0x81,
        0x01,     // Input: (Constant)
                  //
                  // LED report
        0x95,
        0x05,     // Report Count (5)
        0x75,
        0x01,     // Report Size (1)
        0x05,
        0x08,     // Usage Pg (LEDs)
        0x19,
        0x01,     // Usage Min (1)
        0x29,
        0x05,     // Usage Max (5)
        0x91,
        0x02,     // Output: (Data, Variable, Absolute)
                  //
                  // LED report padding
        0x95,
        0x01,     // Report Count (1)
        0x75,
        0x03,     // Report Size (3)
        0x91,
        0x01,     // Output: (Constant)
                  //
                  // Key arrays (6 bytes)
        0x95,
        0x06,     // Report Count (6)
        0x75,
        0x08,     // Report Size (8)
        0x15,
        0x00,     // Log Min (0)
        0x25,
        0x65,     // Log Max (101)
        0x05,
        0x07,     // Usage Pg (Key Codes)
        0x19,
        0x00,     // Usage Min (0)
        0x29,
        0x65,     // Usage Max (101)
        0x81,
        0x00,     // Input: (Data, Array)
                  //
        0xC0,           // End Collection

//  /********************* bit?¨¹?¨¬¡À¡§¡À¨ª ******************************/
        0x05,
        0x01,     // Usage Pg ()
        0x09,
        0x06,     // Usage (Keyboard)
        0xA1,
        0x01,     // Collection: (Application)
        0x85,
        0x03,     //   ReportID................ (3)

        0x05,
        0x07,     // Usage Pg (Key Codes)
        0x19,
        0x04,     // Usage Min
        0x29,
        0x70,     // Usage Max
        0x15,
        0x00,     // Log Min (0)
        0x25,
        0x01,     // Log Max (1)
                  // Modifier byte
        0x75,
        0x01,     // Report Size (1)
        0x95,
        0x78,     // Report Count
        0x81,
        0x02,     // Input: (Data, Variable, Absolute)
                  //
        0xC0,            // End Collection

//  /********************* consume ******************************/
        0x05, 0x0c, 0x09, 0x01, 0xA1, 0x01, 0x85, 0x04, 0x15, 0x00, 0x26, 0xff,
        0x1f, 0x19, 0x00, 0x2A, 0xff, 0x1f, 0x75, 0x10, 0x95, 0x01, 0x81, 0x00,
        0xc0

};

// HID report map length
uint16 hidReportMapLen = sizeof(hidReportMap);

//uint16 hidReportMapLen1 = sizeof(hidReportMap1);

// HID report mapping table
static hidRptMap_t hidRptMap[HID_NUM_REPORTS];

/*********************************************************************
 * Profile Attributes - variables
 */

// HID Service attribute
static const gattAttrType_t hidService = { ATT_BT_UUID_SIZE, hidServUUID };

// Include attribute (Battery service)
static uint16 include = GATT_INVALID_HANDLE;

// HID Information characteristic
static uint8 hidInfoProps = GATT_PROP_READ;

// HID Report Map characteristic
static uint8 hidReportMapProps = GATT_PROP_READ;

// HID External Report Reference Descriptor
static uint8 hidExtReportRefDesc[ATT_BT_UUID_SIZE] = { LO_UINT16(
        BATT_LEVEL_UUID), HI_UINT16(BATT_LEVEL_UUID) };

// HID Control Point characteristic
static uint8 hidControlPointProps = GATT_PROP_WRITE_NO_RSP;
static uint8 hidControlPoint;

// HID Protocol Mode characteristic
static uint8 hidProtocolModeProps = GATT_PROP_READ | GATT_PROP_WRITE_NO_RSP;
uint8 hidProtocolMode = HID_PROTOCOL_MODE_REPORT;

// HID Report characteristic, key input
static uint8 hidReportKeyInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportKeyIn;
static gattCharCfg_t hidReportKeyInClientCharCfg[GATT_MAX_NUM_CONN];
// HID Report Reference characteristic descriptor, key input
static uint8 hidReportRefKeyIn[HID_REPORT_REF_LEN] = { HID_RPT_ID_KEY_IN,
        HID_REPORT_TYPE_INPUT };

// HID Report characteristic, HIDx input
static uint8 hidReportHIDxInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportHIDxIn;
static gattCharCfg_t hidReportHIDxInClientCharCfg[GATT_MAX_NUM_CONN];
// HID Report Reference characteristic descriptor, HIDx input
static uint8 hidReportRefHIDxIn[HID_REPORT_REF_LEN] = { HID_RPT_ID_HIDx_IN,
        HID_REPORT_TYPE_INPUT };

//// HID Report characteristic, Consumer input
//static uint8 hidReportConsumerInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
//static uint8 hidReportConsumerIn;
//static gattCharCfg_t hidReportConsumerInClientCharCfg[GATT_MAX_NUM_CONN];
//// HID Report Reference characteristic descriptor, Consumer input
//static uint8 hidReportRefConsumerIn[HID_REPORT_REF_LEN] =
//             { HID_RPT_ID_CONSUMER_IN, HID_REPORT_TYPE_INPUT };

// HID Report characteristic, KEYbit input
static uint8 hidReportkeybitInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportkeybitIn;
static gattCharCfg_t hidReportkeybitInClientCharCfg[GATT_MAX_NUM_CONN];
// HID Report Reference characteristic descriptor, keybit input
static uint8 hidReportRefkeybitIn[HID_REPORT_REF_LEN] = { HID_RPT_ID_KEYBIT_IN,
        HID_REPORT_TYPE_INPUT };

// HID Report characteristic, KEYConsume input
static uint8 hidReportkeyConsumeInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportkeyConsumeIn;
static gattCharCfg_t hidReportkeyConsumeInClientCharCfg[GATT_MAX_NUM_CONN];
// HID Report Reference characteristic descriptor, keyConsume input
static uint8 hidReportRefkeyConsumeIn[HID_REPORT_REF_LEN] = {
        HID_RPT_ID_KEYCONSUME_IN, HID_REPORT_TYPE_INPUT };

// HID Report characteristic, LED output
static uint8 hidReportLedOutProps = GATT_PROP_READ | GATT_PROP_WRITE
        | GATT_PROP_WRITE_NO_RSP;
static uint8 hidReportLedOut;

// HID Report Reference characteristic descriptor, LED output
static uint8 hidReportRefLedOut[HID_REPORT_REF_LEN] = { HID_RPT_ID_LED_OUT,
        HID_REPORT_TYPE_OUTPUT };

// HID Boot Keyboard Input Report
static uint8 hidReportBootKeyInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportBootKeyIn;
static gattCharCfg_t hidReportBootKeyInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Boot Keyboard Output Report
static uint8 hidReportBootKeyOutProps = GATT_PROP_READ | GATT_PROP_WRITE
        | GATT_PROP_WRITE_NO_RSP;
static uint8 hidReportBootKeyOut;

// Feature Report
static uint8 hidReportFeatureProps = GATT_PROP_READ | GATT_PROP_WRITE;
static uint8 hidReportFeature;

// HID Report Reference characteristic descriptor, Feature
static uint8 hidReportRefFeature[HID_REPORT_REF_LEN] = { HID_RPT_ID_FEATURE,
        HID_REPORT_TYPE_FEATURE };

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t hidAttrTbl[] = {
// HID Service
        { { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ, /* permissions */
        0, /* handle */
        (uint8 *) &hidService /* pValue */
        },

        // Included service (battery)
        { { ATT_BT_UUID_SIZE, includeUUID },
        GATT_PERMIT_READ, 0, (uint8 *) &include },

        // HID Information characteristic declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidInfoProps },

        // HID Information characteristic
        { { ATT_BT_UUID_SIZE, hidInfoUUID },
        GATT_PERMIT_ENCRYPT_READ, 0, (uint8 *) hidInfo },

        // HID Control Point characteristic declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidControlPointProps },

        // HID Control Point characteristic
        { { ATT_BT_UUID_SIZE, hidControlPointUUID },
        GATT_PERMIT_ENCRYPT_WRITE, 0, &hidControlPoint },

        // HID Protocol Mode characteristic declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidProtocolModeProps },

        // HID Protocol Mode characteristic
        { { ATT_BT_UUID_SIZE, hidProtocolModeUUID },
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE, 0,
                &hidProtocolMode },

        // HID Report Map characteristic declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidReportMapProps },

        // HID Report Map characteristic
        { { ATT_BT_UUID_SIZE, hidReportMapUUID },
        GATT_PERMIT_ENCRYPT_READ, 0, (uint8 *) hidReportMap },

        // HID External Report Reference Descriptor
        { { ATT_BT_UUID_SIZE, extReportRefUUID },
        GATT_PERMIT_READ, 0, hidExtReportRefDesc

        },

        /******************  hid key **************************/

        // HID Report characteristic, key input declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidReportKeyInProps },

        // HID Report characteristic, key input
        { { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ, 0, &hidReportKeyIn },

        // HID Report characteristic client characteristic configuration
        { { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE, 0,
                (uint8 *) &hidReportKeyInClientCharCfg },

        // HID Report Reference characteristic descriptor, key input
        { { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ, 0, hidReportRefKeyIn },

        /******************  hid HIDx **************************/

        // HID Report characteristic, HIDx input declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidReportHIDxInProps },

        // HID Report characteristic, Consumer input
        { { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ, 0, &hidReportHIDxIn },

        // HID Report characteristic client characteristic configuration
        { { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE, 0,
                (uint8 *) &hidReportHIDxInClientCharCfg },

        // HID Report Reference characteristic descriptor, Consumer input
        { { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ, 0, hidReportRefHIDxIn },

        /******************  hid bit **************************/
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidReportkeybitInProps },

        // HID Report characteristic, bit input
        { { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ, 0, &hidReportkeybitIn },

        // HID Report characteristic client characteristic configuration
        { { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE, 0,
                (uint8 *) &hidReportkeybitInClientCharCfg },

        // HID Report Reference characteristic descriptor, bit input
        { { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ, 0, hidReportRefkeybitIn },

        /******************  hid consume **************************/
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidReportkeyConsumeInProps },

        // HID Report characteristic, Consumer input
        { { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ, 0, &hidReportkeyConsumeIn },

        // HID Report characteristic client characteristic configuration
        { { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE, 0,
                (uint8 *) &hidReportkeyConsumeInClientCharCfg },

        // HID Report Reference characteristic descriptor, Consumer input
        { { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ, 0, hidReportRefkeyConsumeIn },

        /***************************led*********************************/

        // HID Report characteristic, LED output declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidReportLedOutProps },

        // HID Report characteristic, LED output
        { { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE, 0,
                &hidReportLedOut },

        // HID Report Reference characteristic descriptor, LED output
        { { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ, 0, hidReportRefLedOut },

        /**********************boot key*******************************/
        // HID Boot Keyboard Input Report declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidReportBootKeyInProps },

        // HID Boot Keyboard Input Report
        { { ATT_BT_UUID_SIZE, hidBootKeyInputUUID },
        GATT_PERMIT_ENCRYPT_READ, 0, &hidReportBootKeyIn },

        // HID Boot Keyboard Input Report characteristic client characteristic configuration
        { { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE, 0,
                (uint8 *) &hidReportBootKeyInClientCharCfg },

        // HID Boot Keyboard Output Report declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidReportBootKeyOutProps },

        // HID Boot Keyboard Output Report
        { { ATT_BT_UUID_SIZE, hidBootKeyOutputUUID },
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE, 0,
                &hidReportBootKeyOut },

        /**************************feature****/
        // Feature Report declaration
        { { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ, 0, &hidReportFeatureProps },

        // Feature Report
        { { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE, 0,
                &hidReportFeature },

        // HID Report Reference characteristic descriptor, feature
        { { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ, 0, hidReportRefFeature }, };

// Attribute index enumeration-- these indexes match array elements above
enum {
    HID_SERVICE_IDX,                // HID Service
    HID_INCLUDED_SERVICE_IDX,       // Included Service
    HID_INFO_DECL_IDX,             // HID Information characteristic declaration
    HID_INFO_IDX,                   // HID Information characteristic
    HID_CONTROL_POINT_DECL_IDX,  // HID Control Point characteristic declaration
    HID_CONTROL_POINT_IDX,          // HID Control Point characteristic
    HID_PROTOCOL_MODE_DECL_IDX,  // HID Protocol Mode characteristic declaration
    HID_PROTOCOL_MODE_IDX,          // HID Protocol Mode characteristic
    HID_REPORT_MAP_DECL_IDX,        // HID Report Map characteristic declaration
    HID_REPORT_MAP_IDX,             // HID Report Map characteristic
    HID_EXT_REPORT_REF_DESC_IDX,    // HID External Report Reference Descriptor

    HID_REPORT_KEY_IN_DECL_IDX, // HID Report characteristic, key input declaration
    HID_REPORT_KEY_IN_IDX,          // HID Report characteristic, key input
    HID_REPORT_KEY_IN_CCCD_IDX, // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_KEY_IN_IDX, // HID Report Reference characteristic descriptor, key input

    HID_REPORT_HIDx_IN_DECL_IDX, // HID Report characteristic, key input declaration
    HID_REPORT_HIDx_IN_IDX,          // HID Report characteristic, key input
    HID_REPORT_HIDx_IN_CCCD_IDX, // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_HIDx_IN_IDX, // HID Report Reference characteristic descriptor, key input

    HID_REPORT_KEYBIT_IN_DECL_IDX, // HID Report characteristic, key input declaration
    HID_REPORT_KEYBIT_IN_IDX,      // HID Report characteristic, key input
    HID_REPORT_KEYBIT_IN_CCCD_IDX, // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_Keybit_IN_IDX, // HID Report Reference characteristic descriptor, key input

    HID_REPORT_CONSUME_IN_DECL_IDX, // HID Report characteristic, key input declaration
    HID_REPORT_CONSUME_IN_IDX,      // HID Report characteristic, key input
    HID_REPORT_CONSUME_IN_CCCD_IDX, // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_Consume_IN_IDX, // HID Report Reference characteristic descriptor, key input

    HID_REPORT_LED_OUT_DECL_IDX, // HID Report characteristic, LED output declaration
    HID_REPORT_LED_OUT_IDX,         // HID Report characteristic, LED output
    HID_REPORT_REF_LED_OUT_IDX, // HID Report Reference characteristic descriptor, LED output

    HID_BOOT_KEY_IN_DECL_IDX,      // HID Boot Keyboard Input Report declaration
    HID_BOOT_KEY_IN_IDX,            // HID Boot Keyboard Input Report
    HID_BOOT_KEY_IN_CCCD_IDX, // HID Boot Keyboard Input Report characteristic client characteristic configuration
    HID_BOOT_KEY_OUT_DECL_IDX,    // HID Boot Keyboard Output Report declaration
    HID_BOOT_KEY_OUT_IDX,           // HID Boot Keyboard Output Report

    HID_FEATURE_DECL_IDX,           // Feature Report declaration
    HID_FEATURE_IDX,                // Feature Report
    HID_REPORT_REF_FEATURE_IDX // HID Report Reference characteristic descriptor, feature
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PROFILE CALLBACKS
 */

// Service Callbacks
gattServiceCBs_t hidKbdCBs = { HidDev_ReadAttrCB, // Read callback function pointer
        HidDev_WriteAttrCB, // Write callback function pointer
        NULL                // Authorization callback function pointer
        };

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Hid_AddService
 *
 * @brief   Initializes the HID Service by registering
 *          GATT attributes with the GATT server.
 *
 * @return  Success or Failure
 */
bStatus_t Hid_AddService(void) {
    uint8 status = SUCCESS;

    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, hidReportKeyInClientCharCfg);

    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, hidReportHIDxInClientCharCfg);

    GATTServApp_InitCharCfg( INVALID_CONNHANDLE,
            hidReportkeybitInClientCharCfg);

    GATTServApp_InitCharCfg( INVALID_CONNHANDLE,
            hidReportkeyConsumeInClientCharCfg);

    GATTServApp_InitCharCfg( INVALID_CONNHANDLE,
            hidReportBootKeyInClientCharCfg);

    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService(hidAttrTbl, GATT_NUM_ATTRS(hidAttrTbl),
            GATT_MAX_ENCRYPT_KEY_SIZE, &hidKbdCBs);

    // Set up included service
    Batt_GetParameter( BATT_PARAM_SERVICE_HANDLE,
            &GATT_INCLUDED_HANDLE(hidAttrTbl, HID_INCLUDED_SERVICE_IDX));

//    PRINT("handle %d %d %d \r\n", hidAttrTbl[HID_REPORT_KEY_IN_CCCD_IDX].handle,
//            hidAttrTbl[HID_REPORT_HIDx_IN_CCCD_IDX].handle,
//            hidAttrTbl[HID_REPORT_KEYBIT_IN_CCCD_IDX].handle);

    // Construct map of reports to characteristic handles
    // Each report is uniquely identified via its ID and type

    // Key input report
    hidRptMap[0].id = hidReportRefKeyIn[0];
    hidRptMap[0].type = hidReportRefKeyIn[1];
    hidRptMap[0].handle = hidAttrTbl[HID_REPORT_KEY_IN_IDX].handle;
    hidRptMap[0].cccdHandle = hidAttrTbl[HID_REPORT_KEY_IN_CCCD_IDX].handle;
    hidRptMap[0].mode = HID_PROTOCOL_MODE_REPORT;

    // HIDx input report
    hidRptMap[1].id = hidReportRefHIDxIn[0];
    hidRptMap[1].type = hidReportRefHIDxIn[1];
    hidRptMap[1].handle = hidAttrTbl[HID_REPORT_HIDx_IN_IDX].handle;
    hidRptMap[1].cccdHandle = hidAttrTbl[HID_REPORT_HIDx_IN_CCCD_IDX].handle;
    hidRptMap[1].mode = HID_PROTOCOL_MODE_REPORT;

    // bit input report
    hidRptMap[2].id = hidReportRefkeybitIn[0];
    hidRptMap[2].type = hidReportRefkeybitIn[1];
    hidRptMap[2].handle = hidAttrTbl[HID_REPORT_KEYBIT_IN_IDX].handle;
    hidRptMap[2].cccdHandle = hidAttrTbl[HID_REPORT_KEYBIT_IN_CCCD_IDX].handle;
    hidRptMap[2].mode = HID_PROTOCOL_MODE_REPORT;

    // Consume input report
    hidRptMap[3].id = hidReportRefkeyConsumeIn[0];
    hidRptMap[3].type = hidReportRefkeyConsumeIn[1];
    hidRptMap[3].handle = hidAttrTbl[HID_REPORT_CONSUME_IN_IDX].handle;
    hidRptMap[3].cccdHandle = hidAttrTbl[HID_REPORT_CONSUME_IN_CCCD_IDX].handle;
    hidRptMap[3].mode = HID_PROTOCOL_MODE_REPORT;

    // LED output report
    hidRptMap[4].id = hidReportRefLedOut[0];
    hidRptMap[4].type = hidReportRefLedOut[1];
    hidRptMap[4].handle = hidAttrTbl[HID_REPORT_LED_OUT_IDX].handle;
    hidRptMap[4].cccdHandle = 0;
    hidRptMap[4].mode = HID_PROTOCOL_MODE_REPORT;

    // Boot keyboard input report
    // Use same ID and type as key input report
    hidRptMap[5].id = hidReportRefKeyIn[0];
    hidRptMap[5].type = hidReportRefKeyIn[1];
    hidRptMap[5].handle = hidAttrTbl[HID_BOOT_KEY_IN_IDX].handle;
    hidRptMap[5].cccdHandle = hidAttrTbl[HID_BOOT_KEY_IN_CCCD_IDX].handle;
    hidRptMap[5].mode = HID_PROTOCOL_MODE_BOOT;

    // Boot keyboard output report
    // Use same ID and type as LED output report
    hidRptMap[6].id = hidReportRefLedOut[0];
    hidRptMap[6].type = hidReportRefLedOut[1];
    hidRptMap[6].handle = hidAttrTbl[HID_BOOT_KEY_OUT_IDX].handle;
    hidRptMap[6].cccdHandle = 0;
    hidRptMap[6].mode = HID_PROTOCOL_MODE_BOOT;

    // Feature report
    hidRptMap[7].id = hidReportRefFeature[0];
    hidRptMap[7].type = hidReportRefFeature[1];
    hidRptMap[7].handle = hidAttrTbl[HID_FEATURE_IDX].handle;
    hidRptMap[7].cccdHandle = 0;
    hidRptMap[7].mode = HID_PROTOCOL_MODE_REPORT;

    // Battery level input report
    Batt_GetParameter( BATT_PARAM_BATT_LEVEL_IN_REPORT, &(hidRptMap[8]));

    // Setup report ID map
    HidDev_RegisterReports( HID_NUM_REPORTS, hidRptMap);

    return (status);
}

/*********************************************************************
 * @fn      Hid_SetParameter
 *
 * @brief   Set a HID Kbd parameter.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   len - length of data to right.
 * @param   pValue - pointer to data to write.  This is dependent on
 *          the input parameters and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  GATT status code.
 */
uint8 Hid_SetParameter(uint8 id, uint8 type, uint16 uuid, uint8 len,
        void *pValue) {
    bStatus_t ret = SUCCESS;

    switch (uuid) {
    case REPORT_UUID:
        if (type == HID_REPORT_TYPE_OUTPUT) {
            if (len == 1) {
                hidReportLedOut = *((uint8 *) pValue);
            } else {
                ret = ATT_ERR_INVALID_VALUE_SIZE;
            }
        } else if (type == HID_REPORT_TYPE_FEATURE) {
            if (len == 1) {
                hidReportFeature = *((uint8 *) pValue);
            } else {
                ret = ATT_ERR_INVALID_VALUE_SIZE;
            }
        } else {
            ret = ATT_ERR_ATTR_NOT_FOUND;
        }
        break;

    case BOOT_KEY_OUTPUT_UUID:
        if (len == 1) {
            hidReportBootKeyOut = *((uint8 *) pValue);
        } else {
            ret = ATT_ERR_INVALID_VALUE_SIZE;
        }
        break;

    default:
        // ignore the request
        break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      Hid_GetParameter
 *
 * @brief   Get a HID Kbd parameter.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   pLen - length of data to be read
 * @param   pValue - pointer to data to get.  This is dependent on
 *          the input parameters and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  GATT status code.
 */
uint8 Hid_GetParameter(uint8 id, uint8 type, uint16 uuid, uint16 *pLen,
        void *pValue) {
    switch (uuid) {
    case REPORT_UUID:
        if (type == HID_REPORT_TYPE_OUTPUT) {
            *((uint8 *) pValue) = hidReportLedOut;
            *pLen = 1;
        } else if (type == HID_REPORT_TYPE_FEATURE) {
            *((uint8 *) pValue) = hidReportFeature;
            *pLen = 1;
        } else {
            *pLen = 0;
        }
        break;

    case BOOT_KEY_OUTPUT_UUID:
        *((uint8 *) pValue) = hidReportBootKeyOut;
        *pLen = 1;
        break;

    default:
        *pLen = 0;
        break;
    }

    return ( SUCCESS);
}

/*********************************************************************
 *********************************************************************/
