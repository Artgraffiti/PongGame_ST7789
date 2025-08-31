#include "ble_server.h"

#include "ble.h"
#include "esp_log.h"

const static char *TAG = "BLE_SERVER";
const static char *GATTS_TAG = "BLE_GATTS";

static uint8_t service_uuid128[16] = {
    /* LSB
       <-------------------------------------------------------------------------------->
       MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
    0xee, 0xee, 0xee, 0xee, 0x00, 0x00, 0xdd, 0xdd,
};

static uint8_t adv_config_done = 0;
bool advertising_enabled = false;
#define ADV_CONFIG_FLAG (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = false,
    .include_txpower = false,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_uuid_len = sizeof(service_uuid128),
    .p_service_uuid = service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
};

#define BLE_ADV_INT_MIN 0x100  // 160 ms
#define BLE_ADV_INT_MAX 0x100  // 160 ms

esp_ble_adv_params_t adv_params = {
    .adv_int_min = BLE_ADV_INT_MIN,
    .adv_int_max = BLE_ADV_INT_MAX,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

esp_err_t init_ble_server(void) {
  esp_err_t ret;

  // Config advertising
  esp_ble_gap_set_device_name(DEVICE_NAME);

  ret = esp_ble_gap_config_adv_data(&adv_data);
  if (ret) {
    ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
  } else {
    adv_config_done |= ADV_CONFIG_FLAG;
  }

  ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
  if (ret) {
    ESP_LOGE(TAG, "rsp config adv data failed, error code = %x", ret);
  } else {
    adv_config_done |= SCAN_RSP_CONFIG_FLAG;
  }

  return ret;
}

void start_ble_advertising(void) {
  esp_err_t ret;

  if (!advertising_enabled && adv_config_done) {
    ret = esp_ble_gap_start_advertising(&adv_params);
    if (ret == ESP_OK) {
      advertising_enabled = true;
      ESP_LOGI(TAG, "BLE advertising started");
    } else {
      ESP_LOGE(TAG, "Failed to start advertising: %s", esp_err_to_name(ret));
    }
  }
}

void stop_ble_advertising(void) {
  esp_err_t ret;

  if (advertising_enabled) {
    ret = esp_ble_gap_stop_advertising();
    if (ret == ESP_OK) {
      advertising_enabled = false;
      ESP_LOGI(TAG, "BLE advertising stopped");
    } else {
      ESP_LOGE(TAG, "Failed to stop advertising: %s", esp_err_to_name(ret));
    }
  }
}

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                         esp_ble_gatts_cb_param_t *param) {
  ESP_LOGD(GATTS_TAG, "event = %d", event);
  ESP_LOGW(GATTS_TAG, "Unhandled or unknown GATTS event: %d", event);
}
