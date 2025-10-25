#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include "esp_gatts_api.h"

extern bool advertising_enabled;

esp_err_t init_ble_server(void);
void start_ble_advertising(void);
void stop_ble_advertising(void);
void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

#endif  // BLE_SERVER_H