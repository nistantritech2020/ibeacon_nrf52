#include "fds.h"

#define BEACON_FILE_ID                0x1111

#define BEACON_COMPANY_ID             0x2221
#define BEACON_UUID                   0x2222
#define BEACON_MAJOR                  0x2223
#define BEACON_MINOR                  0x2224
#define BEACON_TX_POWER_AT_1_METER    0x2225
#define BEACON_NAME                   0x2226

#define BEACON_BROADCASTING_INTERVAL  0x2227
#define BEACON_BROADCASTING_POWER     0x2228

#define BEACON_AUTHENTICATION_KEY     0x2229
#define BEACON_SAVE_AND_REBOOT        0x2230


void fds_evt_handler(fds_evt_t const * const p_fds_evt);
void init_fds() ;
bool write_bytes_to_fds(uint16_t file, uint16_t key, uint32_t data_t,uint8_t byte_length);
bool read_bytes_from_fds(uint16_t file, uint16_t key, uint32_t *data_t,uint8_t byte_length) ;
bool write_str_to_fds(uint16_t file, uint16_t key, uint8_t *data,uint8_t length);
bool read_str_to_fds(uint16_t file, uint16_t key, uint8_t *data_t,uint8_t length) ;
void delete_fds();