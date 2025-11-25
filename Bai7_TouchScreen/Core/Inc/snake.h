///*
// * snake.h
// *
// *  Created on: Nov 19, 2025
// *      Author: hoang
// */
//
//#ifndef INC_SNAKE_H_
//#define INC_SNAKE_H_
//
//
//
//#endif /* INC_SNAKE_H_ */

#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

typedef enum {
    DIR_UP = 0,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT
} Direction;

void snake_init(void);                    // call once at startup
void snake_on_tick(void);                 // call on each game tick (move/update)
void snake_draw_full(void);               // redraw whole game (useful after clear)
void snake_set_direction(Direction d);    // set direction (will not allow 180° turns)
void snake_set_direction_from_touch(void); // read touch and set direction towards touch
void snake_restart(void);                 // restart game
uint8_t snake_is_alive(void);

// tuning
extern uint16_t SNAKE_TICK_MS;            // how many ms per snake movement

#endif // SNAKE_H
