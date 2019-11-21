
#ifndef OUR_SERVICE_H__
#define OUR_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"


static const ble_uuid128_t BLE_UUID_OUR_BASE_UUID={.uuid128={0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}}; // 128-bit base UUID
static const uint32_t BLE_UUID_OUR_SERVICE_UUID=0xF00D; // Just a random, but recognizable value

static const uint32_t BEACON_COMPANY_CHARACTERISTC_UUID=0x0001; // Just a random, but recognizable value
static const uint32_t BEACON_UUID_CHARACTERISTC_UUID=0x0002; // Just a random, but recognizable value
static const uint32_t BEACON_MAJOR_CHARACTERISTC_UUID=0x0003; // Just a random, but recognizable value
static const uint32_t BEACON_MINOR_CHARACTERISTC_UUID=0x0004; // Just a random, but recognizable value
static const uint32_t BEACON_MEASURED_POWER_CHARACTERISTC_UUID=0x0005; // Just a random, but recognizable value
static const uint32_t BEACON_INTERVAL_CHARACTERISTC_UUID=0x0006; // Just a random, but recognizable value
static const uint32_t BEACON_TXPOWER_CHARACTERISTC_UUID=0x0007; // Just a random, but recognizable value
static const uint32_t BEACON_NAME_CHARACTERISTC_UUID=0x0008; // Just a random, but recognizable value
static const uint32_t BEACON_AUTHKEY_CHARACTERISTC_UUID=0x0009; // Just a random, but recognizable value
static const uint32_t BEACON_SAVE_REBOOT_CHARACTERISTC_UUID=0x0010; // Just a random, but recognizable value

// This structure contains various status information for our service. 
// The name is based on the naming convention used in Nordics SDKs. 
// 'ble’ indicates that it is a Bluetooth Low Energy relevant structure and 
// ‘os’ is short for Our Service). 
typedef struct
{
    uint16_t                    conn_handle;    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t                    service_handle; /**< Handle of Our Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t beacon_company_id_char_handle;
    ble_gatts_char_handles_t beacon_uuid_char_handle;
    ble_gatts_char_handles_t beacon_major_char_handle;
    ble_gatts_char_handles_t beacon_minor_char_handle;
    ble_gatts_char_handles_t beacon_measured_tx_power_char_handle;
    ble_gatts_char_handles_t beacon_interval_char_handle;
    ble_gatts_char_handles_t beacon_txpower_char_handle;
    ble_gatts_char_handles_t beacon_name_char_handle;
    ble_gatts_char_handles_t beacon_auth_key_char_handle;
    ble_gatts_char_handles_t beacon_save_char_handle;
}ble_os_t;

/**@brief Function for handling BLE Stack events related to our service and characteristic.
 *
 * @details Handles all events from the BLE stack of interest to Our Service.
 *
 * @param[in]   p_our_service       Our Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_our_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for initializing our new service.
 *
 * @param[in]   p_our_service       Pointer to Our Service structure.
 */
void our_service_init(ble_os_t * p_our_service);

/**@brief Function for updating and sending new characteristic values
 *
 * @details The application calls this function whenever our timer_timeout_handler triggers
 *
 * @param[in]   p_our_service                     Our Service structure.
 * @param[in]   characteristic_value     New characteristic value.
 */
void our_temperature_characteristic_update(ble_os_t *p_our_service, int32_t *temperature_value);
void write_parameter_to_flash();
uint32_t add_char(ble_os_t * p_our_service,ble_gatts_char_handles_t *handle,ble_uuid128_t _base_uuid,uint16_t _char_uuid,bool read_flag,bool write_flag,bool notify_flag,bool cccd_flag,uint8_t max_value_len,uint8_t init_value_len);
#endif  /* _ OUR_SERVICE_H__ */
