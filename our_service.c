
#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "our_service.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "ibeacon.h"
#include "flashManager.h"
#include "nrf_delay.h"

uint8_t set_success[]            = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t set_failed[]            = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


static void on_ble_write(ble_os_t * p_our_service, ble_evt_t * p_ble_evt)
{
    uint32_t         err_code;

    ble_gatts_value_t rx_data;
    rx_data.len = sizeof(20);
    rx_data.offset = 0;

    if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_company_id_char_handle.value_handle)
    {
        sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_company_id_char_handle.value_handle, &rx_data);
        set_ibeacon_company_identifier(p_ble_evt->evt.gatts_evt.params.write.data[1] | (p_ble_evt->evt.gatts_evt.params.write.data[0] << 8));
        printf("\nRecieved Ibeacon Company Identifier :%x \n",get_ibeacon_company_identifier());
    }

    else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_uuid_char_handle.value_handle)
    {
        sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_uuid_char_handle.value_handle, &rx_data);
        printf("\nRecieved UUID : \n");
        set_ibeacon_uuid(p_ble_evt->evt.gatts_evt.params.write.data);

    }

    else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_major_char_handle.value_handle)
    {
        sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_major_char_handle.value_handle, &rx_data);
        set_ibeacon_major(p_ble_evt->evt.gatts_evt.params.write.data[1] | (p_ble_evt->evt.gatts_evt.params.write.data[0] << 8));
        printf("\nRecieved Ibeacon Major :%x \n",get_ibeacon_major());


    }

    else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_minor_char_handle.value_handle)
    {
        sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_minor_char_handle.value_handle, &rx_data);
        set_ibeacon_minor(p_ble_evt->evt.gatts_evt.params.write.data[1] | (p_ble_evt->evt.gatts_evt.params.write.data[0] << 8));
        printf("\nRecieved Ibeacon Minor :%x \n",get_ibeacon_minor());
    }

    else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_measured_tx_power_char_handle.value_handle)
    {
        sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_measured_tx_power_char_handle.value_handle, &rx_data);
        set_ibeacon_measured_rssi_at_1_meter(p_ble_evt->evt.gatts_evt.params.write.data[0]);
        printf("\nRecieved Ibeacon Measured Tx Power :%x \n",get_ibeacon_measured_rssi_at_1_meter());

    }
    else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_interval_char_handle.value_handle)
    {
        sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_interval_char_handle.value_handle, &rx_data);
        set_ibeacon_broadcasting_interval(p_ble_evt->evt.gatts_evt.params.write.data[1] | (p_ble_evt->evt.gatts_evt.params.write.data[0] << 8));
        printf("\nRecieved Ibeacon Interval :%x \n",get_ibeacon_broadcasting_interval());

    }
    else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_txpower_char_handle.value_handle)
    {
        sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_txpower_char_handle.value_handle, &rx_data);
        set_ibeacon_broadcasting_txpower(p_ble_evt->evt.gatts_evt.params.write.data[0]);
        printf("\nRecieved Ibeacon Broadcasting Tx power :%02x \n",get_ibeacon_broadcasting_txpower());

    }
    else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_name_char_handle.value_handle)
    {
        err_code=sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_name_char_handle.value_handle, &rx_data);
        APP_ERROR_CHECK(err_code);
        printf("\nRecieved Ibeacon Name %d : %.*s \n",p_ble_evt->evt.gatts_evt.params.write.data[0],p_ble_evt->evt.gatts_evt.params.write.data[0],p_ble_evt->evt.gatts_evt.params.write.data);
        set_ibeacon_name(p_ble_evt->evt.gatts_evt.params.write.data+1,p_ble_evt->evt.gatts_evt.params.write.data[0]);
    }
    else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_auth_key_char_handle.value_handle)
    {
        sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_auth_key_char_handle.value_handle, &rx_data);
        printf("\nRecieved Ibeacon Key %d : %.*s \n",p_ble_evt->evt.gatts_evt.params.write.data[0],p_ble_evt->evt.gatts_evt.params.write.data[0],p_ble_evt->evt.gatts_evt.params.write.data);
        set_ibeacon_key(p_ble_evt->evt.gatts_evt.params.write.data+1,p_ble_evt->evt.gatts_evt.params.write.data[0]);
    }
    else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->beacon_save_char_handle.value_handle)
    {
        sd_ble_gatts_value_get(p_our_service->conn_handle, p_our_service->beacon_save_char_handle.value_handle, &rx_data);
        printf("\nRecieved Save And Reboot :%s \n",p_ble_evt->evt.gatts_evt.params.write.data);
        int8_t *key_back;
        int8_t length;
        get_ibeacon_key_back(&key_back,&length);
        if(memcmp(key_back,p_ble_evt->evt.gatts_evt.params.write.data+1,length)==0){
          update_save_copnfirmation_characteristic_value(p_our_service,set_success);
          set_fds_write_parameter_flag(true);
  
        }
        else{
          printf("\nwrong password\n");
          update_save_copnfirmation_characteristic_value(p_our_service,set_failed);
          nrf_delay_ms(3000);
          NVIC_SystemReset();
        }
    }
    else{
  
        printf("something is wrong");
    }

}


void ble_our_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
		ble_os_t * p_our_service =(ble_os_t *) p_context;
		switch (p_ble_evt->header.evt_id)
{
    case BLE_GATTS_EVT_WRITE:
          on_ble_write(p_our_service, p_ble_evt);
        break;

    case BLE_GAP_EVT_CONNECTED:
        p_our_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;
        break;
    default:
        // No implementation needed.
        break;
}
		
}

/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial. 
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
 //base uuid , charac uuid, read_flag, write_flag, max value, init value, value
uint32_t add_char(ble_os_t * p_our_service,ble_gatts_char_handles_t *handle,ble_uuid128_t _base_uuid,uint16_t _char_uuid,bool read_flag,bool write_flag,bool notify_flag,bool cccd_flag,uint8_t max_value_len,uint8_t init_value_len)
{
    uint8_t value[max_value_len];
    memset(value,0,max_value_len);
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = _base_uuid;
    char_uuid.uuid      = _char_uuid;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);  
    
    
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    if(write_flag==true){
      char_md.char_props.write = 1;
    }
    else{
      char_md.char_props.write = 0;
    }
    if(read_flag==true){
    char_md.char_props.read = 1;
    }
    else{
    char_md.char_props.read = 0;
    }

      
    ble_gatts_attr_md_t cccd_md;
    
    if(cccd_flag==true){
      memset(&cccd_md, 0, sizeof(cccd_md));     
    if(read_flag==true){
      BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    }
    if(write_flag==true){

      BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    }
      cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    if(read_flag==true || write_flag==true){

      char_md.p_cccd_md           = &cccd_md;
    }
    }
    if(notify_flag==true){
      char_md.char_props.notify   = 1;
    }
    else{
      char_md.char_props.notify   = 0;
    }
    
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));  
		attr_md.vloc        = BLE_GATTS_VLOC_STACK;
    
    if(read_flag==true){
      BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    }
    if(write_flag==true){
      BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    }
    
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
		attr_char_value.p_uuid      = &char_uuid;
		attr_char_value.p_attr_md   = &attr_md;
    
		attr_char_value.max_len     = max_value_len;
		attr_char_value.init_len    = init_value_len;
		attr_char_value.p_value     = value;

    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   handle);
    APP_ERROR_CHECK(err_code);
    return NRF_SUCCESS;
}


/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void our_service_init(ble_os_t * p_our_service)
{
    uint32_t   err_code;

    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid     =   BLE_UUID_OUR_BASE_UUID;
    service_uuid.uuid               =   BLE_UUID_OUR_SERVICE_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);    
    
		p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;

		err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_our_service->service_handle);
    
    APP_ERROR_CHECK(err_code);

    
}

void update_save_copnfirmation_characteristic_value(ble_os_t *p_our_service, int value[])
{
    if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len = 20;
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_our_service->beacon_save_char_handle.value_handle;

        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)value;
        sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    }
}