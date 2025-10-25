#include "pong_engine.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "esp_log.h"
#include "pong/pong_menu.h"
#include "pong/pong_menu_actions.h"
#include "pong_types.h"
#include "pong_utils.h"
#include "st7789.h"
#include "time.h"

const static char *TAG = __FILE_NAME__;

static void reset_ball(const PongGame *game, Ball *ball, int8_t direction) {
  const uint16_t f_width = game->field_size.width;
  const uint16_t f_height = game->field_size.height;

  ball->x = f_width / 2;
  ball->y = f_height / 2;
  ball->speed_x = INITIAL_BALL_SPEED_X * direction;
  ball->speed_y = INITIAL_BALL_SPEED_Y * (rand() % 2 ? 1 : -1);
}

static void init_ball(const PongGame *game, Ball *ball) {
  ball->size = BALL_SIZE;
  ball->speed_multiplier = 1.0f;
  reset_ball(game, ball, (rand() % 2 ? 1 : -1));
}

static void init_paddle(const PongGame *game, Paddle *pdl, uint8_t p_num) {
  if (p_num == 1)
    pdl->x = PADDLE_MARGIN;
  else if (p_num == 2)
    pdl->x = game->field_size.width - PADDLE_MARGIN - PADDLE_WIDTH;
  pdl->y = game->field_size.height / 2 - PADDLE_HEIGHT / 2;
  pdl->width = PADDLE_WIDTH;
  pdl->height = PADDLE_HEIGHT;
  pdl->speed = 0;
}

static void init_fonts(GameResources *res) {
  const char *fonts[] = {"/fonts/ILMH16XB.FNT", "/fonts/ILMH24XB.FNT", "/fonts/ILMH32XB.FNT"};
  InitFontx(res->small_font, fonts[0], "");
  InitFontx(res->default_font, fonts[1], "");
  InitFontx(res->large_font, fonts[2], "");
}

void restart_game(PongGame *game) {
  game->player1.score = 0;
  game->player2.score = 0;

  init_paddle(game, &game->player1.paddle, 1);
  init_paddle(game, &game->player2.paddle, 2);
  init_ball(game, &game->ball);

  game->state = GAME_STATE_PLAYING;
}

void init_game(PongGame *game, TFT_t *dev) {
  srand(time(NULL));

  if (dev == NULL) return;
  game->display = dev;

  // Init field size
  game->field_size.width = dev->_width;
  game->field_size.height = dev->_height;

  game->resources.paddle_color = WHITE;
  game->resources.ball_color = GREEN;
  game->resources.background_color = BLACK;
  init_fonts(&game->resources);

  // Menu create
  const char *titles[] = {"Resume", "Restart game", "Settings", "BLE ADV OFF", "BLE SCAN OFF"};
  MenuItemAction actions[] = {menu_resume_game, menu_restart_game, menu_nop, menu_toggle_ble_advertising, menu_toggle_ble_scan};
  game->menu = create_menu(titles, actions, 5);

  restart_game(game);
}

static void clamp_paddle(const PongGame *game, Paddle *p) {
  uint16_t height = game->field_size.height;

  if (p->y < 0) p->y = 0;
  if (p->y > height - p->height) p->y = height - p->height;
}

static void check_winner(PongGame *game) {
  if (game->player1.score >= SCORE_TO_WIN || game->player2.score >= SCORE_TO_WIN) {
    game->state = GAME_STATE_GAME_OVER;
  }
}

static void check_ball_wall_collision(const PongGame *game, Ball *ball) {
  int ball_top = ball->y - ball->size / 2;
  int ball_bottom = ball->y + ball->size / 2;

  if (ball_top <= 0) {
    ball->y = ball->size / 2;
    ball->speed_y *= -1;
  }

  if (ball_bottom >= game->field_size.height) {
    ball->y = game->field_size.height - ball->size;
    ball->speed_y *= -1;
  }
}

static void check_ball_out_of_bounds(PongGame *game, Ball *ball) {
  const int width = game->field_size.width;

  if (game->ball.x < 0) {
    game->player2.score++;
    reset_ball(game, ball, 1);

    ESP_LOGI(TAG, "Point for Player 2");
  } else if (game->ball.x > width) {
    game->player1.score++;
    reset_ball(game, ball, -1);

    ESP_LOGI(TAG, "Point for Player 1");
  }
}

static void clamp_ball_speed(const PongGame *game, Ball *ball) {
  if (ball->speed_multiplier > MAX_SPEED) ball->speed_multiplier = MAX_SPEED;
}

static void move_ball(Ball *ball) {
  ball->x += ball->speed_x * ball->speed_multiplier;
  ball->y += ball->speed_y * ball->speed_multiplier;
}

static void move_paddle(Paddle *paddle) { paddle->y += paddle->speed; }

static bool is_ball_colliding_with_paddle(const Ball *ball, const Paddle *paddle) {
  int ball_left = calc_ball_left(ball);
  int ball_right = calc_ball_right(ball);
  int ball_top = calc_ball_top(ball);
  int ball_bottom = calc_ball_bottom(ball);

  int paddle_left = calc_paddle_left(paddle);
  int paddle_right = calc_paddle_right(paddle);
  int paddle_top = calc_paddle_top(paddle);
  int paddle_bottom = calc_paddle_bottom(paddle);

  bool overlap_x = (ball_right >= paddle_left) && (ball_left <= paddle_right);
  bool overlap_y = (ball_bottom >= paddle_top) && (ball_top <= paddle_bottom);

  return overlap_x && overlap_y;
}

static void handle_paddle_collision(PongGame *game, const Paddle *paddle, uint8_t p_num) {
  Ball *ball = &game->ball;

  if (!is_ball_colliding_with_paddle(ball, paddle)) return;

  const int paddle_right = calc_paddle_right(paddle);
  const int paddle_left = calc_paddle_left(paddle);

  if (p_num == 1) {
    ball->x = paddle_right + 1 + ball->size / 2;
  } else if (p_num == 2) {
    ball->x = paddle_left - 1 - ball->size / 2;
  }

  ball->speed_x = -ball->speed_x;
  ball->speed_multiplier += SPEED_INCREASE;

  ESP_LOGI(TAG, "Ball collided with paddle");
}

void update_game(PongGame *game) {
  if (game->state != GAME_STATE_PLAYING) return;

  Paddle *p1 = &game->player1.paddle;
  Paddle *p2 = &game->player2.paddle;
  Ball *ball = &game->ball;

  move_paddle(p1);
  move_paddle(p2);

  clamp_paddle(game, p1);
  clamp_paddle(game, p2);

  move_ball(ball);

  check_ball_wall_collision(game, ball);

  handle_paddle_collision(game, p1, 1);
  handle_paddle_collision(game, p2, 2);

  clamp_ball_speed(game, ball);
  check_ball_out_of_bounds(game, ball);

  check_winner(game);
}
