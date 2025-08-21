#include <stdbool.h>
#include <stdint.h>

#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include "periph/ble.h"
#include "periph/display.h"
#include "periph/fs.h"
#include "periph/kb.h"
#include "pong/pong_draw.h"
#include "pong/pong_engine.h"
#include "pong/pong_types.h"

const static char *TAG = "main";

#ifdef FRAME_RATE
#include "esp_timer.h"

static void draw_fps(TFT_t *dev, int32_t time_start, FontxFile *font) {
  int32_t delta_time = esp_timer_get_time() - time_start;
  float fps = 1000000.0 / delta_time;
  char fps_s[16];
  snprintf(fps_s, sizeof(fps_s), "FPS: %.2f", fps);
  lcdDrawString(dev, font, 10, 220, (uint8_t *)fps_s, BLUE);
  lcdDrawFinish(dev);
}
#endif

void game_task(void *pvParameters) {
  TFT_t dev;
  PongGame game;

  init_display(&dev);
  init_game(&game, &dev);
  kb_init(&game);

  while (1) {
#ifdef FRAME_RATE
    int32_t time_start = esp_timer_get_time();
#endif

    update_game(&game);
    draw_game(&game);

#ifdef FRAME_RATE
    draw_fps(&dev, time_start, game.resources.small_font);
#endif

    // Delay for watchdog
    vTaskDelay(pdMS_TO_TICKS(10));
  }  // end while

  // never reach here
  vTaskDelete(NULL);
}

void app_main(void) {
  esp_err_t ret;
  ESP_LOGI(TAG, "Hello, User!!:)");

  // Initialize NVS.
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Initialize SPIFFS
  ESP_LOGI(TAG, "Initializing SPIFFS");
  ESP_ERROR_CHECK(mountSPIFFS("/fonts", "storage1", 3));
  listSPIFFS("/fonts/");

  esp_event_loop_create_default();

  xTaskCreate(game_task, "game_task", 1024 * 3, NULL, 3, NULL);

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Check available heap memory
  ESP_LOGI(TAG, "Default: %u", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  ESP_LOGI(TAG, "Internal: %u", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  ESP_LOGI(TAG, "DMA: %u", heap_caps_get_free_size(MALLOC_CAP_DMA));
  ESP_LOGI(TAG, "SPIRAM: %u", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

  ESP_LOGI(TAG, "Largest block (internal): %u",
           heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
  ESP_LOGI(TAG, "Largest block (default): %u",
           heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));

  // Initialize Bluetooth
  ESP_ERROR_CHECK(init_ble());
}