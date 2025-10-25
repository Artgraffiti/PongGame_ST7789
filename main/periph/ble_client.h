#ifndef BLE_CLIENT_H
#define BLE_CLIENT_H

#include <stdbool.h>

#include "esp_err.h"
#include "esp_gattc_api.h"

extern bool is_scanning;

#define SCAN_DURATION_DEFAULT 30
#define MAX_SCAN_DEVICES 20

esp_err_t init_ble_client(void);
void start_ble_scan(void);
void stop_ble_scan(void);
void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

#endif  // BLE_CLIENT_H