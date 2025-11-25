///*
// * snake.c
// *
// *  Created on: Nov 19, 2025
// *      Author: hoang
// */
//
//

#include "snake.h"
#include "lcd.h"
#include "touch.h"
#include "software_timer.h" // if you need timers; not required
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CELL_SIZE   10      // px per grid cell (240/10 = 24, 320/10 = 32)
#define GRID_W      24
#define GRID_H      25
#define MAX_SNAKE   (GRID_W*GRID_H)
#define BG_COLOR    BLACK
#define SNAKE_COLOR 0x07E0   // green-ish (if your 16-bit color macro differs, adjust)
#define SNAKE_HEAD_COLOR 0xFFE0 // yellow-ish
#define FOOD_COLOR  RED

// --- On-screen button layout (bottom of screen) ---
#define BTN_SIZE   60   // 60×60 px buttons

// Positions (adjust to your LCD resolution)
#define BTN_Y      (lcddev.height - BTN_SIZE - 5)

//#define BTN_LEFT_X   10
//#define BTN_UP_X     90
//#define BTN_DOWN_X   170
//#define BTN_RIGHT_X  250

// For 240px wide screens (e.g. 240x320)
#define BTN_LEFT_X   10
#define BTN_UP_X     70
#define BTN_DOWN_X   130
#define BTN_RIGHT_X  190   // fits inside 240



uint16_t SNAKE_TICK_MS = 180; // default tick (can be tuned)

typedef struct {
    uint16_t x;
    uint16_t y;
} Cell;

static Cell snake[MAX_SNAKE];
static int snake_len;
static Direction dir;
static Cell food;
static uint8_t alive;
static uint32_t score;

/* helpers to convert grid cell -> pixel coords */
static inline uint16_t cell_x_px(int gx){ return gx * CELL_SIZE; }
static inline uint16_t cell_y_px(int gy){ return gy * CELL_SIZE; }

static void draw_cell(int gx, int gy, uint16_t color){
    uint16_t x = cell_x_px(gx);
    uint16_t y = cell_y_px(gy);
    /* fill cell rectangle */
    lcd_Fill(x, y, x + CELL_SIZE - 1, y + CELL_SIZE - 1, color);
}

static void place_food(void){
    // simple random free-cell placement
    int tries = 0;
    while(tries++ < 2000){
        int fx = rand() % GRID_W;
        int fy = rand() % GRID_H;
        // ensure not on snake
        int on_snake = 0;
        for(int i=0;i<snake_len;i++){
            if(snake[i].x == fx && snake[i].y == fy){ on_snake = 1; break; }
        }
        if(!on_snake){
            food.x = fx; food.y = fy;
            draw_cell(food.x, food.y, FOOD_COLOR);
            return;
        }
    }
    // fallback: scan for free cell
    for(int gy=0; gy<GRID_H; gy++){
      for(int gx=0; gx<GRID_W; gx++){
        int on_snake = 0;
        for(int i=0;i<snake_len;i++){
            if(snake[i].x == gx && snake[i].y == gy){ on_snake = 1; break; }
        }
        if(!on_snake){ food.x = gx; food.y = gy; draw_cell(food.x, food.y, FOOD_COLOR); return; }
      }
    }
}

//static void place_food(void){
//    while(1){
//        int fx = rand() % GRID_W;
//        int fy = rand() % GRID_H;
//
//        // make sure not on snake
//        int bad = 0;
//        for(int i = 0; i < snake_len; i++){
//            if(snake[i].x == fx && snake[i].y == fy){
//                bad = 1;
//                break;
//            }
//        }
//        if(!bad){
//            food.x = fx;
//            food.y = fy;
//            draw_cell(food.x, food.y, FOOD_COLOR);  // <-- REQUIRED
//            return;
//        }
//    }
//}

static int cell_equals(const Cell *a, const Cell *b){
    return a->x == b->x && a->y == b->y;
}

/* direction helper */
static Direction opposite(Direction d){
    return (d+2)%4;
}

void snake_restart(void){
    // clear screen area (we assume full screen is used)
    lcd_Fill(0, 0, lcddev.width, lcddev.height, BG_COLOR);
    draw_buttons();

    snake_len = 5;
    // start near center
    int startx = GRID_W/2;
    int starty = GRID_H/2;
    for(int i=0;i<snake_len;i++){
        snake[i].x = startx - i;
        snake[i].y = starty;
    }
    dir = DIR_RIGHT;
    alive = 1;
    score = 0;
    // draw snake & food
    for(int i=0;i<snake_len;i++){
        draw_cell(snake[i].x, snake[i].y, (i==0)?SNAKE_HEAD_COLOR:SNAKE_COLOR);
    }
    place_food();
    draw_cell(8, 12, BLACK);
}

void snake_init(void){
    srand( (unsigned) (HAL_GetTick() & 0xFFFF) );
    snake_restart();
    draw_buttons();
}

void snake_draw_full(void){
    lcd_Fill(0, 0, lcddev.width, lcddev.height, BG_COLOR);
    for(int i=0;i<snake_len;i++){
        draw_cell(snake[i].x, snake[i].y, (i==0)?SNAKE_HEAD_COLOR:SNAKE_COLOR);
    }
    draw_cell(food.x, food.y, FOOD_COLOR);
}

/* set direction: disallow immediate 180 turn */
void snake_set_direction(Direction d){
    if(opposite(d) == dir) return; // ignore 180° turn
    dir = d;
}

/* determine direction by touch: if touchscreen touched, pick cardinal direction that points from head->touch */
void snake_set_direction_from_touch(void){
    if(!touch_IsTouched()) return;
    uint16_t tx = touch_GetX();
    uint16_t ty = touch_GetY();
    // get head pixel center
    int hx = snake[0].x * CELL_SIZE + (CELL_SIZE/2);
    int hy = snake[0].y * CELL_SIZE + (CELL_SIZE/2);
    int dx = (int)tx - hx;
    int dy = (int)ty - hy;
    // choose bigger axis
    if(abs(dx) > abs(dy)){
        if(dx > 0) snake_set_direction(DIR_RIGHT);
        else snake_set_direction(DIR_LEFT);
    } else {
        if(dy > 0) snake_set_direction(DIR_DOWN);
        else snake_set_direction(DIR_UP);
    }
}

/* main tick: move, collision, eat */
//void snake_on_tick(void){
//    if(!alive) return;
//    // optionally update dir by touch
//    snake_set_direction_from_touch();
//
//    // compute new head
//    Cell new_head = snake[0];
//    switch(dir){
//        case DIR_UP:    new_head.y = (new_head.y==0)? (GRID_H-1) : new_head.y-1; break;
//        case DIR_DOWN:  new_head.y = (new_head.y+1==GRID_H)? 0 : new_head.y+1; break;
//        case DIR_LEFT:  new_head.x = (new_head.x==0)? (GRID_W-1) : new_head.x-1; break;
//        case DIR_RIGHT: new_head.x = (new_head.x+1==GRID_W)? 0 : new_head.x+1; break;
//    }
//
//    // check self-collision: if new head matches any body cell -> game over
//    for(int i=0;i<snake_len;i++){
//        if(cell_equals(&new_head, &snake[i])){
//            alive = 0;
//            // draw game over text
//            lcd_ShowStr(40, 140, "GAME OVER", RED, BLACK, 24, 1);
//            char tmp[32];
//            sprintf(tmp, "Score: %lu", (unsigned long)score);
//            lcd_ShowStr(60, 170, tmp, WHITE, BLACK, 16, 1);
//            lcd_ShowStr(10, 200, "Touch screen to restart", WHITE, BLACK, 12, 0);
//            return;
//        }
//    }
//
//    // move body: shift array
//    // erase last cell (we will redraw moved parts)
//    draw_cell(snake[snake_len-1].x, snake[snake_len-1].y, BG_COLOR);
//
//    for(int i=snake_len-1; i>0; i--){
//        snake[i] = snake[i-1];
//    }
//    snake[0] = new_head;
//
//    // check food
//    if(cell_equals(&snake[0], &food)){
//        // grow: add one at the tail (we kept last cell erased, so set new tail)
//        if(snake_len < MAX_SNAKE){
//            // tail duplicate of last cell (we shifted already)
//            snake[snake_len] = snake[snake_len-1];
//            snake_len++;
//        }
//        score++;
//        // place new food
//        place_food();
//    } else {
//        // no growth: we already erased last cell earlier
//    }
//
//    // redraw head and the second cell (old head becomes body)
//    draw_cell(snake[1].x, snake[1].y, SNAKE_COLOR);
//    draw_cell(snake[0].x, snake[0].y, SNAKE_HEAD_COLOR);
//}

void snake_on_tick(void){
    if(!alive) return;

//    snake_set_direction_from_touch();
    read_button_input();

    // compute new head
    Cell new_head = snake[0];
    switch(dir){
        case DIR_UP:    new_head.y = (new_head.y == 0) ? (GRID_H - 1) : new_head.y - 1; break;
        case DIR_DOWN:  new_head.y = (new_head.y + 1 == GRID_H) ? 0 : new_head.y + 1; break;
        case DIR_LEFT:  new_head.x = (new_head.x == 0) ? (GRID_W - 1) : new_head.x - 1; break;
        case DIR_RIGHT: new_head.x = (new_head.x + 1 == GRID_W) ? 0 : new_head.x + 1; break;
    }

    // self collision
    for(int i = 0; i < snake_len; i++){
        if(cell_equals(&new_head, &snake[i])){
            alive = 0;
            lcd_ShowStr(40, 140, "GAME OVER", RED, BLACK, 24, 1);
            return;
        }
    }

    // check if eating food BEFORE we erase the tail
    int ate = cell_equals(&new_head, &food);

    // shift body
    for(int i = snake_len - 1; i > 0; i--){
        snake[i] = snake[i - 1];
    }
    snake[0] = new_head;

    if(ate){
        // grow snake
        if(snake_len < MAX_SNAKE){
            snake_len++;
        }
        score++;
        place_food();  // place new food
    } else {
        // erase tail ONLY when not growing
        draw_cell(snake[snake_len - 1].x, snake[snake_len - 1].y, BG_COLOR);
    }

    // redraw head and first body cell
    draw_cell(snake[1].x, snake[1].y, SNAKE_COLOR);
    draw_cell(snake[0].x, snake[0].y, SNAKE_HEAD_COLOR);
}


uint8_t snake_is_alive(void){ return alive; }

//void draw_buttons(void){
//    // left
//    lcd_Fill(BTN_LEFT_X, BTN_Y, BTN_LEFT_X + BTN_SIZE, BTN_Y + BTN_SIZE, BLUE);
//    lcd_ShowStr(BTN_LEFT_X + 22, BTN_Y + 20, "<", WHITE, BLUE, 24, 0);
//
//    // up
//    lcd_Fill(BTN_UP_X, BTN_Y, BTN_UP_X + BTN_SIZE, BTN_Y + BTN_SIZE, BLUE);
//    lcd_ShowStr(BTN_UP_X + 22, BTN_Y + 20, "^", WHITE, BLUE, 24, 0);
//
//    // down
//    lcd_Fill(BTN_DOWN_X, BTN_Y, BTN_DOWN_X + BTN_SIZE, BTN_Y + BTN_SIZE, BLUE);
//    lcd_ShowStr(BTN_DOWN_X + 15, BTN_Y + 20, "v", WHITE, BLUE, 24, 0);
//
//    // right
//    lcd_Fill(BTN_RIGHT_X, BTN_Y, BTN_RIGHT_X + BTN_SIZE, BTN_Y + BTN_SIZE, BLUE);
//    lcd_ShowStr(BTN_RIGHT_X + 20, BTN_Y + 20, ">", WHITE, BLUE, 24, 0);
//}

void draw_buttons(void){
    // LEFT
    lcd_Fill(BTN_LEFT_X, BTN_Y, BTN_LEFT_X + BTN_SIZE, BTN_Y + BTN_SIZE, BLUE);
    lcd_ShowStr(BTN_LEFT_X + 22, BTN_Y + 20, "L", WHITE, BLUE, 24, 1);

    // UP
    lcd_Fill(BTN_UP_X, BTN_Y, BTN_UP_X + BTN_SIZE, BTN_Y + BTN_SIZE, BLUE);
    lcd_ShowStr(BTN_UP_X + 22, BTN_Y + 20, "U", WHITE, BLUE, 24, 1);

    // DOWN
    lcd_Fill(BTN_DOWN_X, BTN_Y, BTN_DOWN_X + BTN_SIZE, BTN_Y + BTN_SIZE, BLUE);
    lcd_ShowStr(BTN_DOWN_X + 22, BTN_Y + 20, "D", WHITE, BLUE, 24, 1);

    // RIGHT
    lcd_Fill(BTN_RIGHT_X, BTN_Y, BTN_RIGHT_X + BTN_SIZE, BTN_Y + BTN_SIZE, BLUE);
    lcd_ShowStr(BTN_RIGHT_X + 22, BTN_Y + 20, "R", WHITE, BLUE, 24, 1);
}


void read_button_input(void){
    if(!touch_IsTouched()) return;

    uint16_t tx = touch_GetX();
    uint16_t ty = touch_GetY();

    // LEFT
    if(tx >= BTN_LEFT_X && tx <= BTN_LEFT_X + BTN_SIZE &&
       ty >= BTN_Y      && ty <= BTN_Y + BTN_SIZE){
        snake_set_direction(DIR_LEFT);
        return;
    }

    // UP
    if(tx >= BTN_UP_X && tx <= BTN_UP_X + BTN_SIZE &&
       ty >= BTN_Y    && ty <= BTN_Y + BTN_SIZE){
        snake_set_direction(DIR_UP);
        return;
    }

    // DOWN
    if(tx >= BTN_DOWN_X && tx <= BTN_DOWN_X + BTN_SIZE &&
       ty >= BTN_Y      && ty <= BTN_Y + BTN_SIZE){
        snake_set_direction(DIR_DOWN);
        return;
    }

    // RIGHT
    if(tx >= BTN_RIGHT_X && tx <= BTN_RIGHT_X + BTN_SIZE &&
       ty >= BTN_Y       && ty <= BTN_Y + BTN_SIZE){
        snake_set_direction(DIR_RIGHT);
        return;
    }
}


/* Expose a function to be called from other code for touch direction by button area optionally */
