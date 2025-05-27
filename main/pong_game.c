#include "pong_game.h"

#include "pong_types.h"
#include "st7789.h"

void init_game(PongGame *game, TFT_t *dev) {
  if (dev == NULL) return;
  game->display = dev;

  // Init field size
  game->field_size.width = dev->_width;
  game->field_size.height = dev->_height;

  // Init fonts
  const char *fonts[] = {
      "/fonts/ILMH16XB.FNT",  // 8x16Dot Mincyo
      "/fonts/ILMH24XB.FNT",  // 12x24Dot Mincyo
      "/fonts/ILMH32XB.FNT"   // 16x32Dot Mincyo
  };
  InitFontx(game->resources.small_font, fonts[0], "");    // 8x16Dot Mincyo
  InitFontx(game->resources.default_font, fonts[1], "");  // 12x24Dot Mincyo
  InitFontx(game->resources.large_font, fonts[2], "");    // 16x32Dot Mincyo

  game->resources.paddle_color = WHITE;
  game->resources.ball_color = GREEN;
  game->resources.background_color = WHITE;

  // Padles
  Paddle *p1 = &game->player1.paddle;
  Paddle *p2 = &game->player2.paddle;

  // Player1 paddle
  p1->x = PADDLE_MARGIN;
  p1->y = game->field_size.height / 2 - PADDLE_HEIGHT / 2;
  p1->width = PADDLE_WIDTH;
  p1->height = PADDLE_HEIGHT;
  p1->speed = 0;

  // Player2 paddle
  p2->x = game->field_size.width - PADDLE_MARGIN - PADDLE_WIDTH;
  p2->y = game->field_size.height / 2 - PADDLE_HEIGHT / 2;
  p2->width = PADDLE_WIDTH;
  p2->height = PADDLE_HEIGHT;
  p2->speed = 0;

  // Ball
  Ball *ball = &game->ball;
  ball->x = game->field_size.width / 2;
  ball->y = game->field_size.height / 2;
  ball->size = BALL_SIZE;
  ball->speed_x = INITIAL_BALL_SPEED_X * (rand() % 2 ? 1 : -1);
  ball->speed_y = INITIAL_BALL_SPEED_Y * (rand() % 2 ? 1 : -1);

  game->ball_speed_multiplier = 1.0f;

  // Scores
  game->player1.score = 0;
  game->player2.score = 0;

  game->state = GAME_STATE_PLAYING;
}

void reset_ball(PongGame *game, int direction) {
  Ball *ball = &game->ball;

  ball->x = game->field_size.width / 2 - BALL_SIZE / 2;
  ball->y = game->field_size.height / 2 - BALL_SIZE / 2;
  ball->speed_x = INITIAL_BALL_SPEED_X * direction;
  ball->speed_y = INITIAL_BALL_SPEED_Y * (rand() % 2 ? 1 : -1);
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

  // Keep paddles on screen
  if (game->player1.paddle.y < 0) game->player1.paddle.y = 0;
  if (game->player1.paddle.y > f_size.height - p1->height)
    p1->y = f_size.height - p1->height;

  if (game->player2.paddle.y < 0) game->player2.paddle.y = 0;
  if (game->player2.paddle.y > f_size.height - p2->height)
    p2->y = f_size.height - p2->height;

  // Move ball
  ball->x += ball->speed_x * game->ball_speed_multiplier;
  ball->y += ball->speed_y * game->ball_speed_multiplier;

  // Ball collision with top and bottom
  int ball_top = ball->y - ball->size / 2;
  int ball_bottom = ball->y + ball->size / 2;
  int ball_left = ball->x - ball->size / 2;
  int ball_right = ball->x + ball->size / 2;
  if (ball_top <= 0) {
    ball->y = ball->size / 2;
    ball->speed_y *= -1;
  }

  if (ball_bottom >= f_size.height) {
    ball->y = f_size.height - ball->size;
    ball->speed_y *= -1;
  }

  // Ball collision with paddles
  int p1_right = p1->x + p1->width;
  int p1_top = p1->y;
  int p1_bottom = p1->y + p1->height;
  if ((ball_left <= p1_right) && (p1_top <= ball_top) &&
      (ball_bottom <= p1_bottom)) {
    ball->x = p1_right + ball->size / 2;
    ball->speed_x *= -1;
    game->ball_speed_multiplier += SPEED_INCREASE;
  }

  int p2_left = p2->x;
  int p2_top = p2->y;
  int p2_bottom = p2->y + p2->height;
  if ((ball_right >= p2_left) && (p2_top <= ball_top) &&
      (ball_bottom <= p2_bottom)) {
    ball->x = p2_left - ball->size / 2;
    ball->speed_x *= -1;
    game->ball_speed_multiplier += SPEED_INCREASE;
  }

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

  // Check for winner
  if (game->player1.score >= SCORE_TO_WIN ||
      game->player2.score >= SCORE_TO_WIN) {
    game->state = GAME_STATE_GAME_OVER;
  }
}
