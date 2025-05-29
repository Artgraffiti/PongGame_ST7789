#include "pong_game.h"
#include <stdint.h>

#include "pong_types.h"
#include "pong_utils.h"
#include "st7789.h"

static void init_ball(PongGame *game, Ball *ball) {
  uint16_t width = game->field_size.width;
  uint16_t height = game->field_size.height;

  ball->x = width / 2;
  ball->y = height / 2;
  ball->size = BALL_SIZE;
  ball->speed_x = INITIAL_BALL_SPEED_X * (rand() % 2 ? 1 : -1);
  ball->speed_y = INITIAL_BALL_SPEED_Y * (rand() % 2 ? 1 : -1);
}

static void init_player(PongGame *game, Player *player, uint8_t p_num) {
  Paddle *pdl = &player->paddle;

  if (p_num == 1)
    pdl->x = PADDLE_MARGIN;
  else if (p_num == 2)
    pdl->x = game->field_size.width - PADDLE_MARGIN - PADDLE_WIDTH;
  pdl->y = game->field_size.height / 2 - PADDLE_HEIGHT / 2;
  pdl->width = PADDLE_WIDTH;
  pdl->height = PADDLE_HEIGHT;
  pdl->speed = 0;

  player->score = 0;
}

static void init_fonts(GameResources *res) {
  const char *fonts[] = {"/fonts/ILMH16XB.FNT", "/fonts/ILMH24XB.FNT",
                         "/fonts/ILMH32XB.FNT"};
  InitFontx(res->small_font, fonts[0], "");
  InitFontx(res->default_font, fonts[1], "");
  InitFontx(res->large_font, fonts[2], "");
}

void init_game(PongGame *game, TFT_t *dev) {
  if (dev == NULL) return;
  game->display = dev;

  // Init field size
  game->field_size.width = dev->_width;
  game->field_size.height = dev->_height;

  game->resources.paddle_color = WHITE;
  game->resources.ball_color = GREEN;
  game->resources.background_color = BLACK;
  init_fonts(&game->resources);

  init_player(game, &game->player1, 1);
  init_player(game, &game->player2, 2);

  init_ball(game, &game->ball);

  game->ball_speed_multiplier = 1.0f;

  game->state = GAME_STATE_PLAYING;
}

static void reset_ball(PongGame *game, uint8_t direction) {
  Ball *ball = &game->ball;

  ball->x = game->field_size.width / 2 - BALL_SIZE / 2;
  ball->y = game->field_size.height / 2 - BALL_SIZE / 2;
  ball->speed_x = INITIAL_BALL_SPEED_X * direction;
  ball->speed_y = INITIAL_BALL_SPEED_Y * (rand() % 2 ? 1 : -1);
}

static void clamp_paddle(PongGame *game, Paddle *p) {
  uint16_t height = game->field_size.height;

  if (p->y < 0) p->y = 0;
  if (p->y > height - p->height) p->y = height - p->height;
}

static void check_winner(PongGame *game) {
  if (game->player1.score >= SCORE_TO_WIN ||
      game->player2.score >= SCORE_TO_WIN) {
    game->state = GAME_STATE_GAME_OVER;
  }
}

static void check_ball_wall_collision(PongGame *game, Ball *ball) {
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

static void handle_paddle_collision(PongGame *game, const Paddle *p, uint8_t p_num) {
  Ball *ball = &game->ball;
  int ball_top = calc_ball_top(ball);
  int ball_bottom = calc_ball_bottom(ball);
  int ball_left = calc_ball_left(ball);
  int ball_right = calc_ball_right(ball);

  int p_top = calc_paddle_top(p);
  int p_bottom = calc_paddle_bottom(p);

  if (p_num == 1) {
    int p_right = calc_paddle_right(p);
    if ((ball_left <= p_right) && (p_top <= ball_top) && (ball_bottom <= p_bottom)) {
      ball->x = p_right + ball->size / 2;
      ball->speed_x *= -1;
      game->ball_speed_multiplier += SPEED_INCREASE;
    }
  } else if (p_num == 2) {
    int p_left = calc_paddle_left(p);
    if ((ball_right >= p_left) && (p_top <= ball_top) && (ball_bottom <= p_bottom)) {
      ball->x = p_left - ball->size / 2;
      ball->speed_x *= -1;
      game->ball_speed_multiplier += SPEED_INCREASE;
    }
  }
}

void update_game(PongGame *game) {
  if (game->state != GAME_STATE_PLAYING) return;

  const GameFieldSize f_size = game->field_size;
  Paddle *p1 = &game->player1.paddle;
  Paddle *p2 = &game->player2.paddle;
  Ball *ball = &game->ball;

  // Move paddles
  p1->y += p1->speed;
  p2->y += p2->speed;

  clamp_paddle(game, p1);
  clamp_paddle(game, p2);

  // Move ball
  ball->x += ball->speed_x * game->ball_speed_multiplier;
  ball->y += ball->speed_y * game->ball_speed_multiplier;

  check_ball_wall_collision(game, ball);

  // Ball collision with paddles
  handle_paddle_collision(game, p1, 1);
  handle_paddle_collision(game, p2, 2);

  // Limit maximum speed
  if (game->ball_speed_multiplier > MAX_SPEED) {
    game->ball_speed_multiplier = MAX_SPEED;
  }

  // Ball out of bounds - score points
  if (game->ball.x < 0) {
    game->player2.score++;
    reset_ball(game, 1);  // Reset ball towards player1
  }
  if (game->ball.x > f_size.width) {
    game->player1.score++;
    reset_ball(game, -1);  // Reset ball towards player2
  }

  check_winner(game);
}
