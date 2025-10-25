#include "ble.h"

#include <string.h>

#include "ble_client.h"
#include "ble_server.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"

const static char *TAG = "BLE_MAIN";
const static char *GAP_TAG = "BLE_GAP";

char device_name[ESP_BLE_ADV_NAME_LEN_MAX] = "PongGame BLE";

uint8_t SERVICE_UUID128[16] = {
    /* LSB <----------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0x00, 0x00, 0xdd, 0xdd,
};

esp_err_t init_ble() {
  esp_err_t ret;
  ESP_LOGI(TAG, "Initializing Bluetooth");
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret) {
    ESP_LOGE(TAG, "%s init controller failed: %s", __func__, esp_err_to_name(ret));
    return ret;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
    ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG, "%s init bluedroid", __func__);

  ret = esp_bluedroid_init();
  if (ret) {
    ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
    return ret;
  }
  ret = esp_bluedroid_enable();
  if (ret) {
    ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
    return ret;
  }

  ret = esp_ble_gatts_register_callback(gatts_event_handler);
  if (ret) {
    ESP_LOGE(TAG, "gatts register error, error code = %x", ret);
    return ret;
  }
  ret = esp_ble_gattc_register_callback(gattc_event_handler);
  if (ret) {
    ESP_LOGE(TAG, "gattc register failed, error code = %x", ret);
    return ret;
  }

  ret = esp_ble_gap_register_callback(gap_event_handler);
  if (ret) {
    ESP_LOGE(TAG, "gap register error, error code = %x", ret);
    return ret;
  }

  ret = init_ble_server();
  if (ret) {
    ESP_LOGE(TAG, "init_ble_server failed: %d", ret);
    return ret;
  }

  ret = init_ble_client();
  if (ret) {
    ESP_LOGE(TAG, "init_ble_client failed: %d", ret);
    return ret;
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

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
  ESP_LOGD(GAP_TAG, "event = %d", event);
  switch (event) {
    // BLE server events
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
      ESP_LOGI(GAP_TAG, "Adv data set complete, status %d", param->adv_data_cmpl.status);
      break;

    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
      ESP_LOGI(GAP_TAG, "Scan response data set complete, status %d", param->scan_rsp_data_cmpl.status);
      break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
      // advertising start complete event to indicate advertising start
      // successfully or failed
      if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(GAP_TAG, "Advertising start failed, status %d", param->adv_start_cmpl.status);
        break;
      }
      ESP_LOGI(GAP_TAG, "Advertising start successfully");
      break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
      if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(GAP_TAG, "Advertising stop failed, status %d", param->adv_stop_cmpl.status);
        break;
      }
      ESP_LOGI(GAP_TAG, "Advertising stop successfully");
      break;

    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
      ESP_LOGI(GAP_TAG, "Packet length update, status %d, rx %d, tx %d", param->pkt_data_length_cmpl.status,
               param->pkt_data_length_cmpl.params.rx_len, param->pkt_data_length_cmpl.params.tx_len);
      break;

    case ESP_GAP_BLE_AUTH_CMPL_EVT: {
      esp_bd_addr_t bd_addr;
      memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
      ESP_LOGI(GAP_TAG, "Authentication complete, addr_type %u, addr " ESP_BD_ADDR_STR "", param->ble_security.auth_cmpl.addr_type,
               ESP_BD_ADDR_HEX(bd_addr));
      if (!param->ble_security.auth_cmpl.success) {
        ESP_LOGI(GAP_TAG, "Pairing failed, reason 0x%x", param->ble_security.auth_cmpl.fail_reason);
      } else {
        ESP_LOGI(GAP_TAG, "Pairing successfully, auth_mode %s", esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode));
      }
      break;
    }

    // BLE client events
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
      ESP_LOGI(GAP_TAG, "Scan param set complete, status %d", param->scan_param_cmpl.status);
      break;
    }

    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
      esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
      switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
          uint8_t adv_name_len = 0;
          uint8_t *adv_name =
              esp_ble_resolve_adv_data_by_type(scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len + scan_result->scan_rst.scan_rsp_len,
                                               ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);

          if (adv_name_len != 0) {
            ESP_LOGI(GAP_TAG, "Scan result, device " ESP_BD_ADDR_STR ", name len %u", ESP_BD_ADDR_HEX(scan_result->scan_rst.bda), adv_name_len);
            ESP_LOG_BUFFER_CHAR(GAP_TAG, adv_name, adv_name_len);
          }

          handle_device_discovery(scan_result, adv_name, adv_name_len);
          break;

        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
          break;
        
        default:
          break;
      }

      break;
    }

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
      if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(GAP_TAG, "Scanning start failed, status %x", param->scan_start_cmpl.status);
        break;
      }
      ESP_LOGI(GAP_TAG, "Scanning start successfully");
      break;

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
      if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(GAP_TAG, "Scanning stop failed, status %x", param->scan_stop_cmpl.status);
        break;
      }
      ESP_LOGI(GAP_TAG, "Scanning stop successfully");
      break;

    default:
      ESP_LOGW(GAP_TAG, "Unhandled or unknown GAP event: %d", event);
      break;
  }
}
