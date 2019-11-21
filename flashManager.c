#include "flashManager.h"
#include <stdint.h>
#include "nrf_delay.h"
#include "ibeacon.h"

#include <string.h>

void fds_evt_handler(fds_evt_t const *const p_fds_evt) {
  switch (p_fds_evt->id) {
  case FDS_EVT_INIT:
    if (p_fds_evt->result != FDS_SUCCESS) {
    }
    break;
  case FDS_EVT_WRITE:
    if (p_fds_evt->result != FDS_SUCCESS) {
    }
    if (p_fds_evt->result == FDS_SUCCESS) {
    }

    break;
  case FDS_EVT_DEL_RECORD:
    if (p_fds_evt->result != FDS_SUCCESS) {
    }
    break;
    
  case FDS_EVT_DEL_FILE:
    if (p_fds_evt->result != FDS_SUCCESS) {
    }
    break;

  default:
    break;
  }
}

void init_fds() {
  ret_code_t ret = fds_register(fds_evt_handler);
  ret = fds_init();
}
void delete_fds(){
      int32_t err_code = fds_file_delete(BEACON_FILE_ID);
    APP_ERROR_CHECK(err_code);
    APP_ERROR_CHECK(fds_gc());

}
bool write_bytes_to_fds(uint16_t file, uint16_t key, uint32_t data_t,uint8_t byte_length) {
  ret_code_t err_code;
  fds_record_t const data_record =
      {
          .file_id = file,
          .key = key,
          .data.p_data = &data_t,
          .data.length_words = (byte_length + 3) / sizeof(uint32_t),
      };
  fds_record_desc_t desc = {0};
  fds_find_token_t tok = {0};
  fds_flash_record_t record = {0};

  if (fds_record_find(file, key, &desc, &tok) == FDS_SUCCESS) {

    err_code = fds_record_update(&desc, &data_record);
  } else {
    err_code = fds_record_write(&desc, &data_record);
  }
    nrf_delay_ms(500);

  if (err_code == FDS_SUCCESS) {
    return true;
  } else {
    printf("Error in write bytes %d\n",err_code);
    return false;
  }

}

bool read_bytes_from_fds(uint16_t file, uint16_t key, uint32_t *data_t,uint8_t byte_length) {

  uint32_t err_code;

  fds_record_desc_t desc = {0};
  fds_find_token_t tok = {0};
  fds_flash_record_t record = {0};

  if (fds_record_find(file, key, &desc, &tok) == FDS_SUCCESS) {
    fds_record_open(&desc, &record);

    memcpy(data_t, record.p_data, byte_length);
    fds_record_close(&desc);
  }

  if (err_code == FDS_SUCCESS) {
    return true;
  } else {
    return false;
  }
}

bool write_str_to_fds(uint16_t file, uint16_t key, uint8_t *data,uint8_t length) {
  uint8_t temp[length+1];
  temp[0]=length;
  memcpy(&temp[1], data, length);
  ret_code_t err_code;
  fds_record_t const data_record =
      {
          .file_id = file,
          .key = key,
          .data.p_data = &temp,
          .data.length_words =  (length + 4) / sizeof(uint32_t),
      };

  fds_record_desc_t desc = {0};
  fds_find_token_t tok = {0};
  fds_flash_record_t record = {0};

  if (fds_record_find(file, key, &desc, &tok) == FDS_SUCCESS) {

    err_code = fds_record_update(&desc, &data_record);
  } else {
    err_code = fds_record_write(&desc, &data_record);

  }
  nrf_delay_ms(500);

  if (err_code == FDS_SUCCESS) {
    return true;
  } else {
      printf("Error in write bytes %d\n",err_code);

    return false;
  }
}

bool read_str_to_fds(uint16_t file, uint16_t key, uint8_t *data_t,uint8_t length) {

  uint32_t err_code;

  fds_record_desc_t desc = {0};
  fds_find_token_t tok = {0};
  fds_flash_record_t record = {0};

  if (fds_record_find(file, key, &desc, &tok) == FDS_SUCCESS) {
    fds_record_open(&desc, &record);
    uint8_t *data=(uint8_t*) record.p_data;
    length=data[0];
    memcpy(data_t, data+1, length);

    fds_record_close(&desc);
  }
  else{
    printf("No record found\n");
  }

  if (err_code == FDS_SUCCESS) {
    return true;
  } else {
    return false;
  }
}
uint8_t get_str_parameter_length(uint16_t file, uint16_t key) {
  
  uint8_t length=0;
  uint32_t err_code;

  fds_record_desc_t desc = {0};
  fds_find_token_t tok = {0};
  fds_flash_record_t record = {0};

  if (fds_record_find(file, key, &desc, &tok) == FDS_SUCCESS) {
    fds_record_open(&desc, &record);

    uint8_t *data=(uint8_t*)record.p_data;
    length=data[0];

    fds_record_close(&desc);
  }
  else{
    printf("No record found\n");
  }
  return length;
}
