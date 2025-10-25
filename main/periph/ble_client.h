#ifndef BLE_CLIENT_H
#define BLE_CLIENT_H

#include <stdbool.h>

#include "esp_err.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"

#define SCAN_DURATION_DEFAULT 5
#define MAX_SCAN_DEVICES 20

esp_err_t init_ble_client(void);
void start_ble_scan(void);
void stop_ble_scan(void);
void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
void handle_device_discovery(esp_ble_gap_cb_param_t *scan_result, uint8_t *adv_name, uint8_t adv_name_len);

#endif  // BLE_CLIENT_H