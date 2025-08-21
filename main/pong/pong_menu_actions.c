#include "pong_menu_actions.h"

#include "esp_log.h"
#include "periph/ble.h"
#include "pong_engine.h"

const static char *TAG = "PONG_MENU_ACTIONS";

extern esp_ble_adv_params_t adv_params;

void menu_resume_game(void *pvParameters) {
  PongGame *game = (PongGame *)pvParameters;
  game->state = GAME_STATE_PLAYING;
}

void menu_restart_game(void *pvParameters) {
  PongGame *game = (PongGame *)pvParameters;
  restart_game(game);
}

void menu_toggle_bluetooth(void *pvParameters) {
  PongGame *game = (PongGame *)pvParameters;
  static bool bluetooth_enabled = false;
  bluetooth_enabled = !bluetooth_enabled;

  if (bluetooth_enabled) {
    ESP_LOGI(TAG, "Bluetooth adv enabled");
    esp_ble_gap_start_advertising(&adv_params);
  } else {
    ESP_LOGI(TAG, "Bluetooth adv disabled");
    esp_ble_gap_stop_advertising();
  }
  sprintf(game->menu->items[3].title, "Bluetooth %s",
          bluetooth_enabled ? "ON" : "OFF");
}

void menu_nop(void *pvParameters) {
  PongGame *game = (PongGame *)pvParameters;
  printf("NOP\n");
}
