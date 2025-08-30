#ifndef BLE_H
#define BLE_H

#include "esp_err.h"
#include "esp_gap_ble_api.h"

#define DEVICE_NAME "PongGame BLE"

esp_err_t init_ble();
esp_err_t start_ble_advertising(void);
esp_err_t stop_ble_advertising(void);
void gap_event_handler(esp_gap_ble_cb_event_t event,
                       esp_ble_gap_cb_param_t *param);

#endif  // BLE_H