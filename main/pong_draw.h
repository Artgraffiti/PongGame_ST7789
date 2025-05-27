#ifndef PONG_DRAW_H
#define PONG_DRAW_H

#include "pong_types.h"
#include "st7789.h"

void draw_midcourt_line(PongGame *game, uint16_t color);
void draw_paddle(PongGame *game, Paddle paddle, uint16_t color);
void draw_ball(PongGame *game, Ball ball, uint16_t color);
void draw_scores(PongGame *game, uint16_t p1_sc, uint16_t p2_sc,
                 uint16_t color);
void draw_game_over(PongGame *game);
void draw_playing(PongGame *game);
void draw_game(PongGame *game);

#endif  // PONG_DRAW_H