#include <stdint.h>
#include <string.h>
#include <stdbool.h>

static uint8_t   ibeacon_adv_data_length           =   0x15   ;                            /**< Length of manufacturer specific data in the advertisement. */
static uint8_t   ibeacon_device_type               =   0x02 ;                              /**< 0x02 refers to Beacon. */


static uint8_t   ibeacon_measured_rssi_at_1_meter  =   0xC5;                               /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
static uint16_t  ibeacon_company_identifier        =   0x0039;                             /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
static uint16_t  ibeacon_major                     =   100;                         /**< Major value used to identify Beacons. */
static uint16_t  ibeacon_minor                     =   200;                         /**< Minor value used to identify Beacons. */
static uint16_t   ibeacon_broadcasting_interval     =   500;                               /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
static int8_t   ibeacon_broadcasting_txpower      =   0;                               /**< The Beacon's measured RSSI at 1 meter distance in dBm. */

static uint8_t   ibeacon_uuid[]              =   {0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0};            /**< Proprietary UUID for Beacon. */
static uint8_t   *ibeacon_device_name              =   "Our123";                       /**< Name of device. Will be included in the advertising data. */
static uint8_t ibeacon_device_name_length=6;
static uint8_t   *ibeacon_auth_key                 =   "NistantriTech";
static uint8_t   ibeacon_auth_key_length                 =   13;
static uint8_t   *ibeacon_auth_key_back ;
static uint8_t   ibeacon_auth_key_back_length;

static bool       fds_write_parameter_flag=false;
void write_parameter_to_flash();