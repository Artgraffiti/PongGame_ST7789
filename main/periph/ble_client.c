#include "ble_client.h"

#include <string.h>

#include "esp_err.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"

static const char *TAG = "BLE_CLIENT";
const static char *GATTC_TAG = "BLE_GATTC";
bool is_scanning = false;

#define PROFILE_PONG_APP_ID 0
#define INVALID_HANDLE 0

static uint8_t scan_config_done = 0;
#define SCAN_CONFIG_FLAG (1 << 0)

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

esp_err_t init_ble_client(void) {
  esp_err_t ret;

  ret = esp_ble_gattc_app_register(PROFILE_PONG_APP_ID);
  if (ret) {
    ESP_LOGE(TAG, "GATTC app register error, error code = %x", ret);
    return ret;
  }

  ret = esp_ble_gap_set_scan_params(&ble_scan_params);
  if (ret) {
    ESP_LOGE(TAG, "Set scan params error, error code = %x", ret);
    return ret;
  } else {
    scan_config_done |= SCAN_CONFIG_FLAG;
  }

  ESP_LOGI(TAG, "BLE client initialized successfully");
  return ESP_OK;
}

void start_ble_scan(void) {
  if (!scan_config_done) {
    ESP_LOGW(TAG, "Scan parameters not set yet");
    return;
  }

  if (is_scanning) {
    return;
  }

  esp_err_t ret = esp_ble_gap_start_scanning(SCAN_DURATION_DEFAULT);
  if (ret == ESP_OK) {
    is_scanning = true;
    ESP_LOGI(TAG, "Started BLE scan for %d seconds", SCAN_DURATION_DEFAULT);
  } else {
    ESP_LOGE(TAG, "Failed to start scanning: %s", esp_err_to_name(ret));
  }
}

void stop_ble_scan(void) {
  if (!scan_config_done) {
    ESP_LOGW(TAG, "Scan parameters not set yet");
    return;
  }

  if (!is_scanning) {
    return;
  }

  esp_err_t ret = esp_ble_gap_stop_scanning();
  if (ret == ESP_OK) {
    is_scanning = false;
    ESP_LOGI(TAG, "Stopped BLE scan");
  } else {
    ESP_LOGE(TAG, "Failed to stop scanning: %s", esp_err_to_name(ret));
  }
}

void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                         esp_ble_gattc_cb_param_t *param) {
  ESP_LOGD(TAG, "GATTC event = %d", event);

  switch (event) {
    case ESP_GATTC_REG_EVT:
      ESP_LOGI(GATTC_TAG,
               "GATT client register, status %d, app_id %d, gattc_if %d",
               param->reg.status, param->reg.app_id, gattc_if);
      break;

    default:
      ESP_LOGW(GATTC_TAG, "Unhandled GATTC event: %d", event);
      break;
  }
}