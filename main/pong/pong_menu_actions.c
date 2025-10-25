#include "pong_menu_actions.h"

#include "esp_log.h"
#include "periph/ble_client.h"
#include "periph/ble_server.h"
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
  extern bool advertising_enabled;

  if (!advertising_enabled) {
    start_ble_advertising();
  } else {
    stop_ble_advertising();
  }
  ESP_LOGI(TAG, "Bluetooth advertising %s", advertising_enabled ? "enabled" : "disabled");

  if (!is_scanning) {
    start_ble_scan();
  } else {
    stop_ble_scan();
  }
  ESP_LOGI(TAG, "%s BLE scan", is_scanning ? "Started" : "Stopped");

  sprintf(game->menu->items[3].title, "Bluetooth %s", advertising_enabled ? "ON" : "OFF");
}

void menu_nop(void *pvParameters) {
  PongGame *game = (PongGame *)pvParameters;
  printf("NOP\n");
}
