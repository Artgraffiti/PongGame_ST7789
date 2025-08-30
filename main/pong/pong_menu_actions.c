#include "pong_menu_actions.h"

#include "esp_log.h"
#include "periph/ble.h"
#include "pong_engine.h"

const static char *TAG = "PONG_MENU_ACTIONS";

void menu_resume_game(void *pvParameters) {
  PongGame *game = (PongGame *)pvParameters;
  ESP_LOGI(TAG, "Resuming game");
  game->state = GAME_STATE_PLAYING;
}

void menu_restart_game(void *pvParameters) {
  PongGame *game = (PongGame *)pvParameters;
  ESP_LOGI(TAG, "Restarting game");
  restart_game(game);
}

void menu_toggle_bluetooth(void *pvParameters) {
  PongGame *game = (PongGame *)pvParameters;
  esp_err_t ret;
  extern bool advertising_enabled;

  if (!advertising_enabled) {
    ret = start_ble_advertising();
  } else {
    ret = stop_ble_advertising();
  }

  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Bluetooth advertising %s",
             advertising_enabled ? "enabled" : "disabled");
    sprintf(game->menu->items[3].title, "Bluetooth %s",
            advertising_enabled ? "ON" : "OFF");
  } else {
    ESP_LOGE(TAG, "Failed to toggle bluetooth: %s", esp_err_to_name(ret));
  }
}

void menu_nop(void *pvParameters) {
  PongGame *game = (PongGame *)pvParameters;
  printf("NOP\n");
}
