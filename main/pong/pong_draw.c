#include "pong_draw.h"

#include <stdio.h>

#include "esp_log.h"
#include "pong_types.h"
#include "st7789.h"

const static char *TAG = __FILE__;

static void draw_midcourt_line(PongGame *game, uint16_t color) {
  TFT_t *dev = game->display;

  for (int y = 0; y < dev->_height; y += 10) {
    lcdDrawFillRect(dev, dev->_width / 2 - 1, y, dev->_width / 2 + 1, y + 5,
                    color);
  }
}

static void draw_paddle(PongGame *game, Paddle paddle, uint16_t color) {
  TFT_t *dev = game->display;

  lcdDrawFillRect(dev, paddle.x, paddle.y, paddle.x + paddle.width,
                  paddle.y + paddle.height, color);
}

static void draw_ball(PongGame *game, Ball ball, uint16_t color) {
  TFT_t *dev = game->display;

  lcdDrawFillCircle(dev, ball.x, ball.y, ball.size / 2, color);
}

static void draw_scores(PongGame *game, uint16_t p1_score, uint16_t p2_score,
                        uint16_t color) {
  TFT_t *dev = game->display;

  char score_str[16];
  snprintf(score_str, sizeof(score_str), "%d - %d", p1_score, p2_score);
  lcdDrawString(dev, game->resources.default_font, dev->_width / 2 - 30, 30,
                (uint8_t *)score_str, color);
}

static void draw_game_over(PongGame *game) {
  TFT_t *dev = game->display;

  uint16_t pl1_score = game->player1.score;
  uint16_t pl2_score = game->player2.score;
  FontxFile *def_font = game->resources.default_font;

  // Draw winner
  const char *winner =
      pl1_score > pl2_score ? "Player1 Wins!" : "Player2 Wins!";
  lcdDrawString(dev, def_font, dev->_width / 2 - 80, dev->_height / 2 - 30,
                (uint8_t *)winner, GREEN);

  // Draw final score
  char score_str[32];
  snprintf(score_str, sizeof(score_str), "Final Score: %d-%d", pl1_score,
           pl2_score);
  lcdDrawString(dev, def_font, dev->_width / 2 - 105, dev->_height / 2,
                (uint8_t *)score_str, WHITE);

  // Draw instructions
  lcdDrawString(dev, def_font, dev->_width / 2 - 90, dev->_height / 2 + 30,
                (uint8_t *)"CONFIRM: Restart", WHITE);
}

static void draw_playing(PongGame *game) {
  TFT_t *dev = game->display;

  // Draw center line
  draw_midcourt_line(game, GRAY);

  // Draw paddles
  draw_paddle(game, game->player1.paddle, game->resources.paddle_color);
  draw_paddle(game, game->player2.paddle, game->resources.paddle_color);

  // Draw ball
  draw_ball(game, game->ball, game->resources.ball_color);

  // Draw scores
  draw_scores(game, game->player1.score, game->player2.score, WHITE);

  // Draw pause state
  if (game->state == GAME_STATE_PAUSED)
    lcdDrawString(dev, game->resources.large_font, dev->_width / 2 - 48,
                  dev->_height / 2, (uint8_t *)"PAUSED", RED);
}

void draw_game(PongGame *game) {
  TFT_t *dev = game->display;
  lcdFillScreen(dev, game->resources.background_color);

  switch (game->state) {
    case GAME_STATE_PLAYING:
    case GAME_STATE_PAUSED:
      draw_playing(game);
      break;
    case GAME_STATE_GAME_OVER:
      draw_game_over(game);
      break;
    default:
      ESP_LOGW(TAG, "Wrong game state: %d", game->state);
  }

  lcdDrawFinish(dev);
}