#include "ibeacon.h"
#include "flashManager.h"

bool get_fds_write_parameter_flag(){
  return fds_write_parameter_flag;
}
void set_fds_write_parameter_flag(bool value){
  fds_write_parameter_flag=value;
}


uint16_t get_ibeacon_company_identifier(){
  return ibeacon_company_identifier;
}
void set_ibeacon_company_identifier(uint16_t value){
  ibeacon_company_identifier=value;
}

uint16_t get_ibeacon_major(){
  return ibeacon_major;
}
void set_ibeacon_major(uint16_t value){
  ibeacon_major=value;
}

uint16_t get_ibeacon_minor(){
  return ibeacon_minor;
}
void set_ibeacon_minor(uint16_t value){
  ibeacon_minor=value;
}

uint8_t get_ibeacon_device_name_length(){
  return ibeacon_device_name_length;
}
void set_ibeacon_device_name_length(uint8_t value){
  ibeacon_device_name_length=value;
}


uint8_t get_ibeacon_adv_data_length(){
  return ibeacon_adv_data_length;
}
void set_ibeacon_adv_data_length(uint8_t value){
  ibeacon_adv_data_length=value;
}

uint8_t get_ibeacon_device_type(){
  return ibeacon_device_type;
}
void set_ibeacon_device_type(uint8_t value){
  ibeacon_device_type=value;
}

uint8_t get_ibeacon_measured_rssi_at_1_meter(){
  return ibeacon_measured_rssi_at_1_meter;
}
void set_ibeacon_measured_rssi_at_1_meter(uint8_t value){
  ibeacon_measured_rssi_at_1_meter=value;
}

uint16_t get_ibeacon_broadcasting_interval(){
  return ibeacon_broadcasting_interval;
}
void set_ibeacon_broadcasting_interval(uint16_t value){
  ibeacon_broadcasting_interval=value;
}

int8_t get_ibeacon_broadcasting_txpower(){
  return ibeacon_broadcasting_txpower;
}
void set_ibeacon_broadcasting_txpower(int8_t value){
  ibeacon_broadcasting_txpower=value;
}

void set_ibeacon_name(int8_t *value,int8_t length){
  free(ibeacon_device_name);
  ibeacon_device_name=(uint8_t*) malloc(length*sizeof(uint8_t));
  memcpy(ibeacon_device_name,value,length);
  ibeacon_device_name_length=length;

}
void set_ibeacon_key(int8_t *value,int8_t length){
  free(ibeacon_auth_key);
  ibeacon_auth_key=(uint8_t*) malloc(length*sizeof(uint8_t));
  memcpy(ibeacon_auth_key,value,length);
  ibeacon_auth_key_length=length;

}

void get_ibeacon_key_back(int8_t **value,int8_t *length){
  *length=ibeacon_auth_key_back_length;
  *value=ibeacon_auth_key_back;
}
void set_ibeacon_key_back(int8_t *value,int8_t length){
  free(ibeacon_auth_key_back);
  ibeacon_auth_key_back=(uint8_t*) malloc(length*sizeof(uint8_t));
  memcpy(ibeacon_auth_key_back,value,length);
  ibeacon_auth_key_back_length=length;
}

void set_ibeacon_uuid(int8_t *value){
  memcpy(ibeacon_uuid,value,16);
}

void write_parameter_to_flash(){
    write_bytes_to_fds(BEACON_FILE_ID,BEACON_COMPANY_ID,get_ibeacon_company_identifier(),2);
    write_bytes_to_fds(BEACON_FILE_ID,BEACON_MAJOR,get_ibeacon_major(),2);
    write_bytes_to_fds(BEACON_FILE_ID,BEACON_MINOR,get_ibeacon_minor(),2);
    write_bytes_to_fds(BEACON_FILE_ID,BEACON_TX_POWER_AT_1_METER,get_ibeacon_measured_rssi_at_1_meter(),1);
    write_bytes_to_fds(BEACON_FILE_ID,BEACON_BROADCASTING_INTERVAL,get_ibeacon_broadcasting_interval(),2);
    write_bytes_to_fds(BEACON_FILE_ID,BEACON_BROADCASTING_POWER,get_ibeacon_broadcasting_txpower(),1);
    write_str_to_fds(BEACON_FILE_ID,BEACON_NAME,ibeacon_device_name,ibeacon_device_name_length);
    write_str_to_fds(BEACON_FILE_ID,BEACON_UUID,ibeacon_uuid,16);
    write_str_to_fds(BEACON_FILE_ID,BEACON_AUTHENTICATION_KEY,ibeacon_auth_key,ibeacon_auth_key_length);
    NVIC_SystemReset();

}
