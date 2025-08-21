#include "ble.h"

#include <string.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_log.h"

const static char *TAG = "BLE_MAIN";
const static char *GATTS_TAG = "BLE_GATTS";
const static char *GAP_TAG = "BLE_GAP";
const static char *DEVICE_NAME = "PongGame BLE";

static uint8_t service_uuid128[16] = {
    /* LSB
       <-------------------------------------------------------------------------------->
       MSB */
    // first uuid, 16bit, [12],[13] is the value
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static uint8_t adv_config_done = 0;
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

esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x100,
    .adv_int_max = 0x100,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

esp_err_t init_ble() {
  esp_err_t ret;
  ESP_LOGI(TAG, "Initializing Bluetooth");
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret) {
    ESP_LOGE(TAG, "%s init controller failed: %s", __func__,
             esp_err_to_name(ret));
    return ret;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
    ESP_LOGE(TAG, "%s enable controller failed: %s", __func__,
             esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG, "%s init bluedroid", __func__);

  ret = esp_bluedroid_init();
  if (ret) {
    ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__,
             esp_err_to_name(ret));
    return ret;
  }
  ret = esp_bluedroid_enable();
  if (ret) {
    ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__,
             esp_err_to_name(ret));
    return ret;
  }

  ret = esp_ble_gatts_register_callback(gatts_event_handler);
  if (ret) {
    ESP_LOGE(TAG, "gatts register error, error code = %x", ret);
    return ret;
  }
  ret = esp_ble_gap_register_callback(gap_event_handler);
  if (ret) {
    ESP_LOGE(TAG, "gap register error, error code = %x", ret);
    return ret;
  }

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

static char *esp_auth_req_to_str(esp_ble_auth_req_t auth_req) {
  char *auth_str = NULL;
  switch (auth_req) {
    case ESP_LE_AUTH_NO_BOND:
      auth_str = "ESP_LE_AUTH_NO_BOND";
      break;
    case ESP_LE_AUTH_BOND:
      auth_str = "ESP_LE_AUTH_BOND";
      break;
    case ESP_LE_AUTH_REQ_MITM:
      auth_str = "ESP_LE_AUTH_REQ_MITM";
      break;
    case ESP_LE_AUTH_REQ_BOND_MITM:
      auth_str = "ESP_LE_AUTH_REQ_BOND_MITM";
      break;
    case ESP_LE_AUTH_REQ_SC_ONLY:
      auth_str = "ESP_LE_AUTH_REQ_SC_ONLY";
      break;
    case ESP_LE_AUTH_REQ_SC_BOND:
      auth_str = "ESP_LE_AUTH_REQ_SC_BOND";
      break;
    case ESP_LE_AUTH_REQ_SC_MITM:
      auth_str = "ESP_LE_AUTH_REQ_SC_MITM";
      break;
    case ESP_LE_AUTH_REQ_SC_MITM_BOND:
      auth_str = "ESP_LE_AUTH_REQ_SC_MITM_BOND";
      break;
    default:
      auth_str = "INVALID BLE AUTH REQ";
      break;
  }

  return auth_str;
}

void gap_event_handler(esp_gap_ble_cb_event_t event,
                       esp_ble_gap_cb_param_t *param) {
  ESP_LOGI(GAP_TAG, "event = %d", event);
  switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
      ESP_LOGI(GAP_TAG, "Adv data set complete, status %d",
               param->adv_data_cmpl.status);
      break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
      ESP_LOGI(GAP_TAG, "Scan response data set complete, status %d",
               param->scan_rsp_data_cmpl.status);
      break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
      // advertising start complete event to indicate advertising start
      // successfully or failed
      if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(GAP_TAG, "Advertising start failed, status %x",
                 param->adv_start_cmpl.status);
        break;
      }
      ESP_LOGI(GAP_TAG, "Advertising start successfully");
      break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
      if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(GAP_TAG, "Advertising stop failed, status %d",
                 param->adv_stop_cmpl.status);
        break;
      }
      ESP_LOGI(GAP_TAG, "Advertising stop successfully");
      break;
    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
      ESP_LOGI(GAP_TAG, "Packet length update, status %d, rx %d, tx %d",
               param->pkt_data_length_cmpl.status,
               param->pkt_data_length_cmpl.params.rx_len,
               param->pkt_data_length_cmpl.params.tx_len);
      break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
      esp_bd_addr_t bd_addr;
      memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr,
             sizeof(esp_bd_addr_t));
      ESP_LOGI(
          GAP_TAG,
          "Authentication complete, addr_type %u, addr " ESP_BD_ADDR_STR "",
          param->ble_security.auth_cmpl.addr_type, ESP_BD_ADDR_HEX(bd_addr));
      if (!param->ble_security.auth_cmpl.success) {
        ESP_LOGI(GAP_TAG, "Pairing failed, reason 0x%x",
                 param->ble_security.auth_cmpl.fail_reason);
      } else {
        ESP_LOGI(GAP_TAG, "Pairing successfully, auth_mode %s",
                 esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode));
      }
      break;
    default:
      ESP_LOGW(GAP_TAG, "Unhandled or unknown GAP event: %d", event);
      break;
  }
}

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                         esp_ble_gatts_cb_param_t *param) {
  ESP_LOGI(GATTS_TAG, "event = %d", event);
  ESP_LOGW(GATTS_TAG, "Unhandled or unknown GATTS event: %d", event);
}
