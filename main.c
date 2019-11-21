#include "our_service.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "our_service.h"
#include "flashManager.h"
#include "ibeacon.h"

#ifdef BSP_BUTTON_0 //GPIO 17
#define PIN_IN BSP_BUTTON_0
#endif
#ifndef PIN_IN
#error "Please indicate input pin"
#endif

bool button_pressed=false;
bool flash_remove=false;

#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                       /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                      /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                             /**< Advertising module instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

static ble_os_t m_our_service;


APP_TIMER_DEF(m_autodisconnect_erase_timer_id);
#define AUTODISCONNECT_TIMER_INTERVAL APP_TIMER_TICKS(30000) 
#define ERASE_TIMER_INTERVAL APP_TIMER_TICKS(5000) 

#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(200, UNIT_0_625_MS)  /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define APP_BEACON_INFO_LENGTH          0x17                               /**< Total length of information advertised by the Beacon. */

#define DEAD_BEEF                       0xDEADBEEF                         /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

static ble_gap_adv_params_t m_adv_params;                                  /**< Parameters to be passed to the stack when starting advertising. */
static uint8_t              m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< Advertising handle used to identify an advertising set. */
static uint8_t              m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  /**< Buffer for storing an encoded advertising set. */
static uint8_t              m_enc_scanrespdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  /**< Buffer for storing an encoded advertising set. */

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data =
{
    .adv_data =
    {
        .p_data = m_enc_advdata,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = m_enc_scanrespdata,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX

    }
};


struct m_beacon_info                    /**< Information advertised by the Beacon. */
{
    uint8_t ibeacon_device_type;     // Manufacturer specific information. Specifies the device type in this
                        // implementation.
    uint8_t ibeacon_adv_data_length; // Manufacturer specific information. Specifies the length of the
                         // manufacturer specific data in this implementation.
    uint8_t ibeacon_uuid[16];     // 128 bit UUID value.
    uint8_t ibeacon_major[2];     // Major arbitrary value that can be used to distinguish between Beacons.
    uint8_t ibeacon_minor[2];     // Minor arbitrary value that can be used to distinguish between Beacons.
    uint8_t ibeacon_measured_rssi_at_1_meter;    // Manufacturer specific information. The Beacon's measured TX power in
                         // this implementation.
};




static void advertising_start(bool erase_bonds);


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}




/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            NRF_LOG_INFO("Connected to a previously bonded device.");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            advertising_start(false);
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
            // This can happen when the local DB has changed.
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}


static void timer_timeout_handler(void * p_context)
{
  if(button_pressed==true){
    flash_remove=true;
  }
  else{
    printf("User Auto Disconnected");
    sd_ble_gap_disconnect(m_conn_handle,BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
  }
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    app_timer_create(&m_autodisconnect_erase_timer_id, APP_TIMER_MODE_SINGLE_SHOT, timer_timeout_handler);


}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)ibeacon_device_name,
                                          ibeacon_device_name_length);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}



/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t         err_code;
    nrf_ble_qwr_init_t qwr_init = {0};

    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    our_service_init(&m_our_service);

    add_char(&m_our_service,&m_our_service.beacon_company_id_char_handle,BLE_UUID_OUR_BASE_UUID,BEACON_COMPANY_CHARACTERISTC_UUID,true,true,true,true,2,2);
    add_char(&m_our_service,&m_our_service.beacon_uuid_char_handle ,BLE_UUID_OUR_BASE_UUID,BEACON_UUID_CHARACTERISTC_UUID,true,true,true,true,16,16);
    add_char(&m_our_service,&m_our_service.beacon_major_char_handle,BLE_UUID_OUR_BASE_UUID,BEACON_MAJOR_CHARACTERISTC_UUID,true,true,true,true,2,2);
    add_char(&m_our_service,&m_our_service.beacon_minor_char_handle,BLE_UUID_OUR_BASE_UUID,BEACON_MINOR_CHARACTERISTC_UUID,true,true,true,true,2,2);
    add_char(&m_our_service,&m_our_service.beacon_measured_tx_power_char_handle,BLE_UUID_OUR_BASE_UUID,BEACON_MEASURED_POWER_CHARACTERISTC_UUID,true,true,true,true,sizeof(ibeacon_measured_rssi_at_1_meter),sizeof(ibeacon_measured_rssi_at_1_meter));
    add_char(&m_our_service,&m_our_service.beacon_interval_char_handle,BLE_UUID_OUR_BASE_UUID,BEACON_INTERVAL_CHARACTERISTC_UUID,true,true,true,true,sizeof(ibeacon_broadcasting_interval),sizeof(ibeacon_broadcasting_interval));
    add_char(&m_our_service,&m_our_service.beacon_txpower_char_handle,BLE_UUID_OUR_BASE_UUID,BEACON_TXPOWER_CHARACTERISTC_UUID,true,true,true,true,2,2);
    add_char(&m_our_service,&m_our_service.beacon_name_char_handle,BLE_UUID_OUR_BASE_UUID,BEACON_NAME_CHARACTERISTC_UUID,true,true,true,true,20,20);
    add_char(&m_our_service,&m_our_service.beacon_auth_key_char_handle,BLE_UUID_OUR_BASE_UUID,BEACON_AUTHKEY_CHARACTERISTC_UUID,true,true,true,true,20,20);
    add_char(&m_our_service,&m_our_service.beacon_save_char_handle,BLE_UUID_OUR_BASE_UUID,BEACON_SAVE_REBOOT_CHARACTERISTC_UUID,true,true,true,true,20,20);
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        default:
            break;
    }
}




/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;
    ble_os_t * p_our_service;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected.");
            advertising_start(1);

            // LED indication will be changed when advertising starts.
 

            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);

            //When connected; start our timer to start regular temperature measurements
            app_timer_start(m_autodisconnect_erase_timer_id, AUTODISCONNECT_TIMER_INTERVAL, NULL);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }

		
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

    //OUR_JOB: Step 3.C Set up a BLE event observer to call ble_our_service_on_ble_evt() to do housekeeping of ble connections related to our service and characteristics.
    NRF_SDH_BLE_OBSERVER(m_our_service_observer, APP_BLE_OBSERVER_PRIO, ble_our_service_on_ble_evt, (void*) &m_our_service);
    sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);

	
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    printf("%d",err_code);
    APP_ERROR_CHECK(err_code);
}


/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated when button is pressed.
 */
static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break; // BSP_EVENT_SLEEP

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break; // BSP_EVENT_DISCONNECT

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break; // BSP_EVENT_KEY_0

        default:
            break;
    }
}

void uint16_t_to_uint8_t_array(uint16_t _16bitvalue,uint8_t *_8bit_array){
  _8bit_array[1]=_16bitvalue& 0x00FF;
  _8bit_array[0]= _16bitvalue>>8;
}
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrespdata;

    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_manuf_data_t manuf_specific_data;

    memset(&advdata, 0, sizeof(advdata));
    memset(&scanrespdata, 0, sizeof(scanrespdata));

    manuf_specific_data.company_identifier = ibeacon_company_identifier;
    struct m_beacon_info ibeacon={
    .ibeacon_device_type=ibeacon_device_type,
    .ibeacon_adv_data_length=ibeacon_adv_data_length,
    .ibeacon_major=ibeacon_major,
    .ibeacon_minor=ibeacon_minor,
    .ibeacon_measured_rssi_at_1_meter=ibeacon_measured_rssi_at_1_meter
    };
    memcpy(ibeacon.ibeacon_uuid,ibeacon_uuid,sizeof(ibeacon_uuid));
    uint8_t temp[2];
    uint16_t_to_uint8_t_array(ibeacon_major,&temp);
    memcpy(ibeacon.ibeacon_major,temp,sizeof(ibeacon_major));
    uint16_t_to_uint8_t_array(ibeacon_minor,&temp);
    memcpy(ibeacon.ibeacon_minor,temp,sizeof(ibeacon_minor));

    manuf_specific_data.data.p_data = &ibeacon;
    manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;

    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr     = NULL;
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval        = MSEC_TO_UNITS(ibeacon_broadcasting_interval, UNIT_0_625_MS);
    m_adv_params.duration        = 0;       // Never time out.

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    scanrespdata.name_type=BLE_ADVDATA_FULL_NAME;
    scanrespdata.p_tx_power_level=NULL;  

    err_code = ble_advdata_encode(&scanrespdata, m_adv_data.scan_rsp_data.p_data, &m_adv_data.scan_rsp_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
    APP_ERROR_CHECK(err_code);
}

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	if (nrf_drv_gpiote_in_is_set(PIN_IN) ==0) {
            printf("pressed\n");
            app_timer_start(m_autodisconnect_erase_timer_id, ERASE_TIMER_INTERVAL, NULL);
            button_pressed=true;
	} else {
            printf("Released\n");
            app_timer_stop(m_autodisconnect_erase_timer_id);
            button_pressed=false;
	}

}

/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
	ret_code_t err_code;
	err_code = nrf_drv_gpiote_init();
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
	in_config.pull = NRF_GPIO_PIN_PULLUP;
	err_code = nrf_drv_gpiote_in_init(PIN_IN, &in_config, in_pin_handler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_in_event_enable(PIN_IN, true);

}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for starting advertising.
 */
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED event
    }
    else
    {
    ret_code_t err_code;

    err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
    err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_adv_handle, ibeacon_broadcasting_txpower); 
    APP_ERROR_CHECK(err_code); 


    }
}

void load_parameter_from_flash(){
    read_bytes_from_fds(BEACON_FILE_ID,BEACON_COMPANY_ID,&ibeacon_company_identifier,sizeof(ibeacon_company_identifier));
    set_ibeacon_company_identifier(ibeacon_company_identifier);
    printf("Ibeacon Company Identifier : %lu\n",ibeacon_company_identifier);
    read_bytes_from_fds(BEACON_FILE_ID,BEACON_MAJOR,&ibeacon_major,sizeof(ibeacon_major));
    printf("Ibeacon Major : %lu\n",ibeacon_major);
    read_bytes_from_fds(BEACON_FILE_ID,BEACON_MINOR,&ibeacon_minor,sizeof(ibeacon_minor));
    set_ibeacon_minor(ibeacon_minor);
    printf("Ibeacon Minor : %lu\n",ibeacon_minor);
    read_bytes_from_fds(BEACON_FILE_ID,BEACON_TX_POWER_AT_1_METER,&ibeacon_measured_rssi_at_1_meter,sizeof(ibeacon_measured_rssi_at_1_meter));
    set_ibeacon_measured_rssi_at_1_meter(ibeacon_measured_rssi_at_1_meter);
    printf("Ibeacon Measured RSSI : %lu\n",ibeacon_measured_rssi_at_1_meter);
    read_bytes_from_fds(BEACON_FILE_ID,BEACON_BROADCASTING_INTERVAL,&ibeacon_broadcasting_interval,sizeof(ibeacon_broadcasting_interval));
    set_ibeacon_broadcasting_interval(ibeacon_broadcasting_interval);
    printf("Ibeacon Broadcasting Interval : %lu \n",ibeacon_broadcasting_interval);
    read_bytes_from_fds(BEACON_FILE_ID,BEACON_BROADCASTING_POWER,&ibeacon_broadcasting_txpower,sizeof(ibeacon_broadcasting_txpower));
    set_ibeacon_broadcasting_txpower(ibeacon_broadcasting_txpower);
    printf("Ibeacon Broadcasting Power : %d\n",ibeacon_broadcasting_txpower);


    uint8_t temp_length;
    temp_length=get_str_parameter_length(BEACON_FILE_ID,BEACON_NAME);
    if(temp_length>=1){
      free(ibeacon_device_name);
      ibeacon_device_name=(uint8_t*) malloc(temp_length*sizeof(uint8_t));
      read_str_to_fds(BEACON_FILE_ID,BEACON_NAME,ibeacon_device_name,&temp_length);
      ibeacon_device_name_length=temp_length;
    }
    set_ibeacon_name(ibeacon_device_name,ibeacon_device_name_length);
    printf("Device Name : %.*s\n",temp_length,ibeacon_device_name);

    temp_length=get_str_parameter_length(BEACON_FILE_ID,BEACON_UUID);
    if(temp_length>=1){
      read_str_to_fds(BEACON_FILE_ID,BEACON_UUID,ibeacon_uuid,&temp_length);
    }
    set_ibeacon_uuid(ibeacon_uuid);

    printf("UUID : ");
    for (int i = 0; i < temp_length; i++)
   {
      printf("%02X ", ibeacon_uuid[i]);
    }

    temp_length=get_str_parameter_length(BEACON_FILE_ID,BEACON_AUTHENTICATION_KEY);
    if(temp_length>=1){
      ibeacon_auth_key=(uint8_t*) malloc(temp_length*sizeof(uint8_t));
      read_str_to_fds(BEACON_FILE_ID,BEACON_AUTHENTICATION_KEY,ibeacon_auth_key,&temp_length);
      ibeacon_auth_key_length=temp_length;
    }
    set_ibeacon_key(ibeacon_auth_key,ibeacon_auth_key_length);
    printf("\nAuthentication Key : %.*s",ibeacon_auth_key_length,ibeacon_auth_key);

    set_ibeacon_key_back(ibeacon_auth_key,ibeacon_auth_key_length);

    get_ibeacon_key_back(&ibeacon_auth_key_back,&ibeacon_auth_key_back_length);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;

    // Initialize.
    log_init();
    timers_init();
    buttons_leds_init(&erase_bonds);
    printf("started..\n");
    init_fds();
    load_parameter_from_flash();
    power_management_init();
    ble_stack_init();

    gap_params_init();

    gatt_init();

    services_init();

    advertising_init();

    conn_params_init();

    advertising_start(erase_bonds);


    // Enter main loop.
    for (;;)
    {
        if(get_fds_write_parameter_flag()==true){
          printf("writing to flash...\n");
          write_parameter_to_flash();
        }
        if(flash_remove==true){
            delete_fds();
            printf("flash removed");
            nrf_delay_ms(2000);
            NVIC_SystemReset();
        }

        
        idle_state_handle();
    }
}


/**
 * @}
 */