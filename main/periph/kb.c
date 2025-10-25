#include "kb.h"

#include "button.h"
#include "esp_log.h"
#include "pong/pong_engine.h"
#include "pong/pong_menu.h"
#include "pong/pong_types.h"

#if !defined(BUTTON_LEFT_GPIO)
#error "BUTTON_LEFT_GPIO not defined"
#endif

#if !defined(BUTTON_UP_GPIO)
#error "BUTTON_UP_GPIO not defined"
#endif

#if !defined(BUTTON_DOWN_GPIO)
#error "BUTTON_DOWN_GPIO not defined"
#endif

#if !defined(BUTTON_RIGHT_GPIO)
#error "BUTTON_RIGHT_GPIO not defined"
#endif

#if !defined(BUTTON_CONFIRM_GPIO)
#error "BUTTON_CONFIRM_GPIO not defined"
#endif

#if !defined(BUTTON_CANCEL_GPIO)
#error "BUTTON_CANCEL_GPIO not defined"
#endif

const static char *TAG = "KEYBOARD";

void kb_init(void *pvParameters) {
  button_debounce_cfg debounce_cfg = {
      .debounce_checks = DEBOUNCE_CHECKS,
      .debounce_interval_ms = DEBOUNCE_INTERVAL_MS,
  };

  static button_t btn_left, btn_up, btn_down, btn_right, btn_confirm, btn_cancel;
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

void kb_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
  button_state_info_t *state_info = (button_state_info_t *)event_data;
  PongGame *game = (PongGame *)handler_args;
  bool pressed = state_info->state;

  if (id == BUTTON_LEFT_GPIO) {
    game->player1.paddle.speed = pressed ? -PADDLE_SPEED : 0;
    ESP_LOGI(TAG, "button(LEFT) %d: %s", (int)id, pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_UP_GPIO) {
    if (game->state == GAME_STATE_PLAYING) {
      game->player2.paddle.speed = pressed ? -PADDLE_SPEED : 0;
    } else if (game->state == GAME_STATE_PAUSED) {
      if (pressed) menu_up(game->menu);
    }
    ESP_LOGI(TAG, "button(UP) %d: %s", (int)id, pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_DOWN_GPIO) {
    if (game->state == GAME_STATE_PLAYING) {
      game->player1.paddle.speed = pressed ? PADDLE_SPEED : 0;
    } else if (game->state == GAME_STATE_PAUSED) {
      if (pressed) menu_down(game->menu);
    }
    ESP_LOGI(TAG, "button(DOWN) %d: %s", (int)id, pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_RIGHT_GPIO) {
    game->player2.paddle.speed = pressed ? PADDLE_SPEED : 0;
    ESP_LOGI(TAG, "button(RIGHT) %d: %s", (int)id, pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_CONFIRM_GPIO) {
    if (game->state == GAME_STATE_GAME_OVER) {
      if (pressed) restart_game(game);
    } else if (game->state == GAME_STATE_PAUSED) {
      if (pressed) menu_select(game->menu, game);
    }
    ESP_LOGI(TAG, "button(CONFIRM) %d: %s", (int)id, pressed ? "PRESSED" : "RELEASED");

  } else if (id == BUTTON_CANCEL_GPIO) {
    if (pressed) {
      if (game->state == GAME_STATE_PLAYING) {
        game->state = GAME_STATE_PAUSED;
      } else if (game->state == GAME_STATE_PAUSED) {
        game->state = GAME_STATE_PLAYING;
      }
    }
    ESP_LOGI(TAG, "button(CANCEL) %d: %s", (int)id, pressed ? "PRESSED" : "RELEASED");
  }
}