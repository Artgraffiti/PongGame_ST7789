#ifndef PONG_UTILS_H
#define PONG_UTILS_H

#include "pong_types.h"

static inline __attribute__((always_inline)) int calc_ball_left(const Ball *b) { return b->x - b->size / 2; }
static inline __attribute__((always_inline)) int calc_ball_right(const Ball *b) { return b->x + b->size / 2; }
static inline __attribute__((always_inline)) int calc_ball_top(const Ball *b) { return b->y - b->size / 2; }
static inline __attribute__((always_inline)) int calc_ball_bottom(const Ball *b) { return b->y + b->size / 2; }

static inline __attribute__((always_inline)) int calc_paddle_top(const Paddle *p) { return p->y; }
static inline __attribute__((always_inline)) int calc_paddle_bottom(const Paddle *p) { return p->y + p->height; }
static inline __attribute__((always_inline)) int calc_paddle_left(const Paddle *p) { return p->x; }
static inline __attribute__((always_inline)) int calc_paddle_right(const Paddle *p) { return p->x + p->width; }

#endif