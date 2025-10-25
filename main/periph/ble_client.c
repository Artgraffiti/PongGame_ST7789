#include "ble_client.h"

#include <string.h>

#include "esp_bt_defs.h"
#include "esp_err.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "BLE_CLIENT";
const static char *GATTC_TAG = "BLE_GATTC";
const static char *GATTC_PROFILE_TAG = "BLE_GATTC";

#define PROFILE_NUM 1
#define PROFILE_PONG_APP_ID 0
#define INVALID_HANDLE 0

extern char device_name[ESP_BLE_ADV_NAME_LEN_MAX];
bool connect = false;

static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

struct gattc_profile_inst {
  esp_gattc_cb_t gattc_cb;
  uint16_t gattc_if;
  uint16_t app_id;
  uint16_t conn_id;
  uint16_t service_start_handle;
  uint16_t service_end_handle;
  uint16_t char_handle;
  esp_bd_addr_t remote_bda;
};

static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_PONG_APP_ID] =
        {
            .gattc_cb = gattc_profile_event_handler,
            .gattc_if = ESP_GATT_IF_NONE,
        },
};

typedef enum {
  SCAN_CONFIG_FLAG = (1 << 0),
} scan_config_flags_t;

bool is_scanning = false;
static uint8_t scan_config_done = 0;

void ble_scan_timer_cb(void *arg);
static esp_timer_handle_t ble_scan_timer;
const static esp_timer_create_args_t ble_timer_args = {.callback = &ble_scan_timer_cb, .name = "ble_scan_timer"};

static esp_ble_scan_params_t ble_scan_params = {.scan_type = BLE_SCAN_TYPE_ACTIVE,
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
  }
  scan_config_done |= SCAN_CONFIG_FLAG;
  ESP_ERROR_CHECK(esp_timer_create(&ble_timer_args, &ble_scan_timer));

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
  ESP_ERROR_CHECK(esp_timer_start_once(ble_scan_timer, SCAN_DURATION_DEFAULT * 1000 * 1000));
  if (ret == ESP_OK) {
    is_scanning = true;
    ESP_LOGI(TAG, "Started BLE scan for %d seconds", SCAN_DURATION_DEFAULT);
  } else {
    ESP_LOGE(TAG, "Failed to start scanning: %s", esp_err_to_name(ret));
  }
}

void ble_scan_timer_cb(void *arg) {
  ESP_LOGI(TAG, "BLE scan timer expired, stopping scan");
  is_scanning = false;
}

void stop_ble_scan(void) {
  if (!scan_config_done) {
    ESP_LOGW(TAG, "Scan parameters not set yet");
    return;
  }

  if (esp_timer_is_active(ble_scan_timer)) {
    ESP_ERROR_CHECK(esp_timer_stop(ble_scan_timer));
  }

  if (!is_scanning) {
    ESP_LOGW(TAG, "Scan not active");
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

void handle_device_discovery(esp_ble_gap_cb_param_t *scan_result, uint8_t *adv_name, uint8_t adv_name_len) {
  if (adv_name == NULL) {
    return;
  }

  if (strlen(device_name) == adv_name_len && strncmp((char *)adv_name, device_name, adv_name_len) == 0) {
    ESP_LOGI(TAG, "Device found: %s", device_name);

    if (connect == false) {
      connect = true;
      ESP_LOGI(TAG, "Target device found. Connecting...");

      stop_ble_scan();
      esp_ble_gatt_creat_conn_params_t creat_conn_params = {0};

      memcpy(&creat_conn_params.remote_bda, scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
      creat_conn_params.remote_addr_type = scan_result->scan_rst.ble_addr_type;
      creat_conn_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
      creat_conn_params.is_direct = true;
      creat_conn_params.is_aux = false;
      creat_conn_params.phy_mask = 0x0;

      esp_ble_gattc_enh_open(gl_profile_tab[PROFILE_PONG_APP_ID].gattc_if, &creat_conn_params);
    }
  }
}

void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
  ESP_LOGD(TAG, "GATTC event = %d", event);

  switch (event) {
    case ESP_GATTC_REG_EVT:
      ESP_LOGI(GATTC_TAG, "GATT client register, status %d, app_id %d, gattc_if %d", param->reg.status, param->reg.app_id, gattc_if);
      break;

    default:
      ESP_LOGW(GATTC_TAG, "Unhandled GATTC event: %d", event);
      break;
  }
}

static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
  esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
  switch (event) {
    default:
      ESP_LOGW(GATTC_PROFILE_TAG, "Unhandled or unknown GATTC_PROFILE_TAG event: %d", event);
      break;
  }
}