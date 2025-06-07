#ifndef PONG_TYPES_H
#define PONG_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "fontx.h"
#include "st7789.h"

// Game configuration
#define PADDLE_WIDTH 8
#define PADDLE_HEIGHT 56
#define PADDLE_MARGIN 10
#define BALL_SIZE 12
#define INITIAL_BALL_SPEED_X 2
#define INITIAL_BALL_SPEED_Y 5
#define PADDLE_SPEED 6
#define SCORE_TO_WIN 10
#define MAX_SPEED 6
#define SPEED_INCREASE 0.2f

typedef enum {
  GAME_STATE_PLAYING,
  GAME_STATE_GAME_OVER,
  GAME_STATE_PAUSED
} GameState;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
  int16_t speed;
} Paddle;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t size;
  float speed_x;
  float speed_y;
  float speed_multiplier;
} Ball;

typedef struct {
  Paddle paddle;
  uint8_t score;
} Player;

typedef struct {
  FontxFile default_font[2];
  FontxFile large_font[2];
  FontxFile small_font[2];

  uint16_t paddle_color;
  uint16_t ball_color;
  uint16_t background_color;
} GameResources;

typedef struct {
  uint16_t width;
  uint16_t height;
} GameFieldSize;

typedef struct {
  Player player1;
  Player player2;
  Ball ball;
  GameState state;
  GameResources resources;
  GameFieldSize field_size;
  TFT_t *display;
} PongGame;

#endif  // PONG_TYPES_H