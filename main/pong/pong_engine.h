#ifndef PONG_ENGINE_H
#define PONG_ENGINE_H

#include "pong_types.h"
#include "st7789.h"

void init_game(PongGame *game, TFT_t *dev);
void restart_game(PongGame *game);
void update_game(PongGame *game);

#endif  // PONG_ENGINE_H