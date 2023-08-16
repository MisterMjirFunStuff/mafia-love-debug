#include "level.h"
#include "sdlgfx.h"
#include "sdlinput.h"
#include "raycast.h"
#include "mmath.h"

const uint8_t circle[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1,
  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

/* 16 x 16 test level */
const uint8_t sample_level[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1,
  1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

const uint8_t tiny_level[] = {
  1, 1, 1,
  1, 0, 1,
  1, 1, 1
};

static const uint8_t *current_world;
static       uint8_t  current_worldw;

/*
* level_draw
*
* @desc
*   Draws the level
*/
//#include "big_level.h"
void level_draw(uint16_t x, uint16_t y, uint16_t phi)
{
  sdlgfx_draw_rect(0, 0, WIN_W, WIN_H, 0);

  rc_draw_world(x, y, phi, current_world, current_worldw);

  sdlgfx_send();
}

/* Get the theta of the input */
#define PI 32768
uint16_t get_thetai(uint16_t phi, struct input *inputs)
{
  uint16_t thetai = 0;

  if (inputs->w) {
    if (inputs->a) {
      thetai = 0 + PI / 4;
    }
    else if (inputs->d) {
      thetai = 0 - PI / 4;
    }
    else {
      thetai = 0;
    }
  }
  else if (inputs->s) {
    if (inputs->a) {
      thetai = PI - PI / 4;
    }
    else if (inputs->d) {
      thetai = PI + PI / 4;
    }
    else {
      thetai = PI;
    }
  }
  else if (inputs->a) {
    thetai = 0 + PI / 2;
  }
  else if (inputs->d) {
    thetai = 0 - PI / 2;
  }

  return thetai;
}

/*
* level_loop
*
* @desc
*   Level loop
*/
#define X   (*x >> 8)
#define XL ((*x >> 8) - 1)
#define XR ((*x >> 8) + 1)
#define Y   (*y >> 8)
#define YU ((*y >> 8) - 1)
#define YD ((*y >> 8) + 1)
#define X_MOVL *x -= 0x0100; *x |=  0x00FF;
#define X_MOVR *x += 0x0100; *x &= ~0x00FF;
#define Y_MOVD *y += 0x0100; *y &= ~0x00FF;
#define Y_MOVU *y -= 0x0100; *y |=  0x00FF;
/*
 * @param d
 *   Sign of change in positions [0] x, [1] y (1 is negative, 0 is positive)
 */
void handle_collision_square(uint16_t *x, uint16_t *y, uint8_t d, const uint8_t *world, uint8_t worldw)
{
  /* NOTE: Switch this to a circle collision */
  if (world[YU * worldw + X ]) { Y_MOVD; } /* Up */
  if (world[YD * worldw + X ]) { Y_MOVU; } /* Down */
  /* Left Wall */
  if (world[Y  * worldw + XL]) { X_MOVR; }         /* Left */
  //if (world[YU * worldw + XL]) { X_MOVR; Y_MOVD; } /* Top Left */      
  //if (world[YD * worldw + XL]) { X_MOVR; Y_MOVU; } /* Bottom left */
  /* Right Wall */
  if (world[Y  * worldw + XR]) { X_MOVL; }         /* Right */
  //if (world[YU * worldw + XR]) { X_MOVL; Y_MOVD; } /* Top right */
  //if (world[YD * worldw + XR]) { X_MOVL; Y_MOVU; } /* Bottom Right */

  return;
}

/*
 * move
 * 
 * @desc
 *   Move the player
 *   Handles the collision using a 'circle'
 *   Don't make CR greater than 1 (plz)
 */
void move_quad(uint16_t *x, uint16_t *y, uint16_t dx, uint16_t dy, uint8_t quad)
{
  if      (quad == 0) { *y -= dy; *x += dx; } /* Q1 */
  else if (quad == 1) { *y -= dy; *x -= dx; } /* Q2 */
  else if (quad == 2) { *y += dy; *x += dx; } /* Q4 */
  else if (quad == 3) { *y += dy; *x -= dx; } /* Q3 */
}

#define CR 0x0050
#define MR 0x0005
void move(uint16_t* x, uint16_t* y, uint16_t theta, uint8_t quad, const uint8_t* world, uint8_t worldw)
{
  uint16_t cdx, cdy, mdx, mdy; /* Collision and move deltas */

  cdx = ((uint32_t) mm_cost(theta) * (uint32_t) CR) >> 16;
  cdy = ((uint32_t) mm_sint(theta) * (uint32_t) CR) >> 16;
  mdx = ((uint32_t) mm_cost(theta) * (uint32_t) MR) >> 16;
  mdy = ((uint32_t) mm_sint(theta) * (uint32_t) MR) >> 16;

  move_quad(x, y, mdx, mdy, quad);

  /* Check 8 points of the circle */
  if ((world[((*y + cdy) >> 8) * worldw + ((*x + cdx) >> 8)]) ||
      (world[((*y + cdy) >> 8) * worldw + ((*x - cdx) >> 8)]) ||
      (world[((*y - cdy) >> 8) * worldw + ((*x + cdx) >> 8)]) ||
      (world[((*y - cdy) >> 8) * worldw + ((*x - cdx) >> 8)]) ||
      (world[((*y - CR ) >> 8) * worldw + ((*x      ) >> 8)]) ||
      (world[((*y      ) >> 8) * worldw + ((*x + CR ) >> 8)]) ||
      (world[((*y + CR ) >> 8) * worldw + ((*x      ) >> 8)]) ||
      (world[((*y      ) >> 8) * worldw + ((*x - CR ) >> 8)])
    ) {
    /* Handle collision (just move backwards to negate the move)*/
    mdy = 0 - mdy;
    mdx = 0 - mdx;

    move_quad(x, y, mdx, mdy, quad);
  }
}
#undef MR
#undef CR

void level_loop(void)
{
  static uint16_t x = (8 << 8) + 0x80;
  static uint16_t y = (8 << 8) + 0x80;
  static uint16_t thetam = 0;
  static uint16_t dx = 0;
  static uint16_t dy = 0;
  static uint16_t phi = 0;

  struct input inputs;

  /* Turn */
  #define MOV_PHI 64
  inputs = sdlinput_get();
  if (inputs.left) {
    phi += MOV_PHI;
  }
  if (inputs.right) {
    phi -= MOV_PHI;
  }

  /* Move */
  if (inputs.w || inputs.a || inputs.s || inputs.d) {
    thetam = get_thetai(phi, &inputs) + phi;

    uint8_t quad = 0;
    while (thetam > 0x4000) {
      thetam -= 0x4000;
      ++quad;
    }
    if (quad & 0x1) thetam = 16384 - thetam; /* Make sure theta is based off x-axis and not y */
    if (quad > 1) quad ^= 0x1; /* Convert quadrant to format specified above */

    move(&x, &y, thetam, quad, current_world, current_worldw);
  }

  dx = 0; dy = 0;

  level_draw(x, y, phi);
}

/*
* level_init
*
* @desc
*   Switches the state to level,
*   and initializes the level
*/
extern void (*loop_fn)(void);
void level_init(uint8_t level)
{
  loop_fn = level_loop;
  current_world = circle;
  current_worldw = 16;
}