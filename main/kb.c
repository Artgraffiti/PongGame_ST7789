#include "kb.h"

#include "button.h"
#include "esp_log.h"
#include "pong/pong_engine.h"
#include "pong/pong_types.h"

void kb_init(void *pvParameters) {
  button_debounce_cfg debounce_cfg = {
      .debounce_checks = DEBOUNCE_CHECKS,
      .debounce_interval_ms = DEBOUNCE_INTERVAL_MS,
  };

  static button_t btn_left, btn_up, btn_down, btn_right, btn_confirm,
      btn_cancel;
  button_init(&btn_left, BUTTON_LEFT_GPIO, true);
  button_init(&btn_up, BUTTON_UP_GPIO, true);
  button_init(&btn_down, BUTTON_DOWN_GPIO, true);
  button_init(&btn_right, BUTTON_RIGHT_GPIO, true);
  button_init(&btn_confirm, BUTTON_CONFIRM_GPIO, true);
  button_init(&btn_cancel, BUTTON_CANCEL_GPIO, true);

  button_set_pullmode(&btn_left, GPIO_PULLUP_ONLY);
  button_set_pullmode(&btn_up, GPIO_PULLUP_ONLY);
  button_set_pullmode(&btn_down, GPIO_PULLUP_ONLY);
  button_set_pullmode(&btn_right, GPIO_PULLUP_ONLY);
  button_set_pullmode(&btn_confirm, GPIO_PULLUP_ONLY);
  button_set_pullmode(&btn_cancel, GPIO_PULLUP_ONLY);

  button_set_debounce_conf(&btn_left, debounce_cfg);
  button_set_debounce_conf(&btn_up, debounce_cfg);
  button_set_debounce_conf(&btn_down, debounce_cfg);
  button_set_debounce_conf(&btn_right, debounce_cfg);
  button_set_debounce_conf(&btn_confirm, debounce_cfg);
  button_set_debounce_conf(&btn_cancel, debounce_cfg);

  button_set_event_handler(&btn_left, kb_event_handler, pvParameters);
  button_set_event_handler(&btn_up, kb_event_handler, pvParameters);
  button_set_event_handler(&btn_down, kb_event_handler, pvParameters);
  button_set_event_handler(&btn_right, kb_event_handler, pvParameters);
  button_set_event_handler(&btn_confirm, kb_event_handler, pvParameters);
  button_set_event_handler(&btn_cancel, kb_event_handler, pvParameters);
}

void kb_event_handler(void *handler_args, esp_event_base_t base, int32_t id,
                      void *event_data) {
  button_state_info_t *state_info = (button_state_info_t *)event_data;
  PongGame *game = (PongGame *)handler_args;
  bool pressed = state_info->state;

  if (id == BUTTON_LEFT_GPIO) {
    game->player1.paddle.speed = pressed ? -PADDLE_SPEED : 0;
    ESP_LOGI("LEFT", "button %d: %s", (int)id,
             pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_UP_GPIO) {
    game->player2.paddle.speed = pressed ? -PADDLE_SPEED : 0;
    ESP_LOGI("UP", "button %d: %s", (int)id, pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_DOWN_GPIO) {
    game->player1.paddle.speed = pressed ? PADDLE_SPEED : 0;
    ESP_LOGI("DOWN", "button %d: %s", (int)id,
             pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_RIGHT_GPIO) {
    game->player2.paddle.speed = pressed ? PADDLE_SPEED : 0;
    ESP_LOGI("RIGHT", "button %d: %s", (int)id,
             pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_CONFIRM_GPIO) {
    if (pressed && game->state == GAME_STATE_GAME_OVER) {
      restart_game(game);
    }
    ESP_LOGI("CONFIRM", "button %d: %s", (int)id,
             pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_CANCEL_GPIO) {
    if (pressed) {
      if (game->state == GAME_STATE_PLAYING) {
        game->state = GAME_STATE_PAUSED;
      } else if (game->state == GAME_STATE_PAUSED) {
        game->state = GAME_STATE_PLAYING;
      }
    }
    ESP_LOGI("CANCEL", "button %d: %s", (int)id,
             pressed ? "PRESSED" : "RELEASED");
  }
}