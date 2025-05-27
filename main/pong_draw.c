#include "pong_draw.h"

#include <stdio.h>

#include "pong_types.h"

extern PongGame game;

void draw_midcourt_line(TFT_t *dev, uint16_t color) {
  for (int y = 0; y < dev->_height; y += 10) {
    lcdDrawFillRect(dev, dev->_width / 2 - 1, y, dev->_width / 2 + 1, y + 5,
                    color);
  }
}

void draw_paddle(TFT_t *dev, Paddle paddle, uint16_t color) {
  lcdDrawFillRect(dev, paddle.x, paddle.y, paddle.x + paddle.width,
                  paddle.y + paddle.height, color);
}

void draw_ball(TFT_t *dev, Ball ball, uint16_t color) {
  lcdDrawFillCircle(dev, ball.x, ball.y, ball.size / 2, color);
}

void draw_scores(TFT_t *dev, uint16_t p1_score, uint16_t p2_score,
                 uint16_t color) {
  char score_str[16];
  snprintf(score_str, sizeof(score_str), "%d - %d", p1_score, p2_score);
  lcdDrawString(dev, game.resources.default_font, dev->_width / 2 - 30, 30,
                (uint8_t *)score_str, color);
}

void draw_game_over(TFT_t *dev) {
  uint16_t pl1_score = game.player1.score;
  uint16_t pl2_score = game.player2.score;
  FontxFile *def_font = game.resources.default_font;

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

void draw_playing(TFT_t *dev) {
  // Draw center line
  draw_midcourt_line(dev, GRAY);

  // Draw paddles
  draw_paddle(dev, game.player1.paddle, WHITE);
  draw_paddle(dev, game.player2.paddle, WHITE);

  // Draw ball
  draw_ball(dev, game.ball, GREEN);

  // Draw scores
  draw_scores(dev, game.player1.score, game.player2.score, WHITE);

  // Draw pause state
  if (game.state == GAME_STATE_PAUSED)
    lcdDrawString(dev, game.resources.large_font, dev->_width / 2 - 48,
                  dev->_height / 2, (uint8_t *)"PAUSED", RED);
}

void draw_game(TFT_t *dev) {
  lcdFillScreen(dev, BLACK);

  switch (game.state) {
    case GAME_STATE_PLAYING:
    case GAME_STATE_PAUSED:
      draw_playing(dev);
      break;
    case GAME_STATE_GAME_OVER:
      draw_game_over(dev);
      break;
  }

  lcdDrawFinish(dev);
}