#ifndef PONG_GAME_H
#define PONG_GAME_H

// Game configuration
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 60
#define PADDLE_MARGIN 10
#define BALL_SIZE 12
#define INITIAL_BALL_SPEED_X 2
#define INITIAL_BALL_SPEED_Y 5
#define PADDLE_SPEED 6
#define SCORE_TO_WIN 10
#define MAX_SPEED 6
#define SPEED_INCREASE 0.2f

#include "st7789.h"

void init_game(TFT_t *dev);
void update_game();
void reset_ball(int direction);

#endif  // PONG_GAME_H