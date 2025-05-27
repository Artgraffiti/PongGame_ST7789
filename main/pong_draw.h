#ifndef PONG_DRAW_H
#define PONG_DRAW_H

#include "pong_types.h"
#include "st7789.h"

void draw_midcourt_line(TFT_t *dev, uint16_t color);
void draw_paddle(TFT_t *dev, Paddle paddle, uint16_t color);
void draw_ball(TFT_t *dev, Ball ball, uint16_t color);
void draw_scores(TFT_t *dev, uint16_t p1_sc, uint16_t p2_sc, uint16_t color);
void draw_game_over(TFT_t *dev);
void draw_playing(TFT_t *dev);
void draw_game(TFT_t *dev);

#endif  // PONG_DRAW_H