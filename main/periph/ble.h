#ifndef BLE_H
#define BLE_H

#include "esp_err.h"
#include "esp_gap_ble_api.h"

esp_err_t init_ble();
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

#endif  // BLE_H