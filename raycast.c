#include "raycast.h"
#include "sdlgfx.h"
#include "mmath.h"

/* Debug functions (DO NOT EXIST IN KEIL CODE */
SDL_Renderer *renderer;
static void draw_world(uint16_t x, uint16_t y, uint16_t phi, const uint8_t* world, uint8_t worldw);
#define TILE_SIZE 32
#define HITBOX_SIZE 32

/*
* References
* https://lodev.org/cgtutor/raycasting.html
* https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/
*/

/* Field of view is 60 degrees, pi / 3, or 10,923 */
#define FOV 10880
#define FOV_2 5440
#define FOV_STEP 68

/* The screen is rotated, so it's a z buffer for x but there is y amount of x */
static uint16_t rc_zbuffer[WIN_H];

/*
* draw_world_strip
*
* @desc
*   Draws a strip (vertical line)
* @param screen_x
*   Where to draw the strip
* @param distance
*   Distance of the wall, what else can I say?
* @param data
*   Upper byte - Logic 0 for x hit, logic 1 for y hit
*   Lower byte - Fractional part of hit on the wall
* @param tile
*   Which tile was hit (basically what texture to use)
*/
#include "height_table.h"
#include "tex_step_table.h"
#include "textures.h"
void rc_draw_world_strip(uint8_t screen_x, uint16_t distance, uint16_t data, uint8_t tile)
{
  uint8_t color;
  const uint8_t *tex_row;
  uint8_t start_y;        /* Where to start drawing the strip */
  uint8_t height;
  uint8_t tex_step;       /* [7:2] integer, [1:0] fraction */
  uint8_t total_tex_step; /* [7:2] integer, [1:0] fraction */

						  /* Initialize */
  if (distance >=  32769) height = 0;
  else                    height = height_table[distance];

  start_y = (WIN_W - height) >> 1;

  tex_row = &textures[tile][((data & 0x00FF) >> 2) * TEXTURE_SIZE]; /* >> 2 to go from 256 -> 64 */
  tex_step = tex_step_table[height];
  total_tex_step = 0;


  /* Draw */
  for (uint8_t i = start_y; i < start_y + height; ++i) {
	/* Determine which texel to use */
	color = tex_row[total_tex_step >> 2];
	if (!(data & 0x8000)) color += 32;

	sdlgfx_draw_pixel(i, screen_x, color);

	total_tex_step += tex_step;
  }

  /* Set z buffer */
  rc_zbuffer[screen_x] = distance;
}

/*
* draw_world_ray
*
* @desc
*   Casts a single ray and draws the strip for it
* @param screen_x
*   How the ray is mapping to the screen
* @param x
*   Ray start x position (fixed [15:8] int [7:0] fraction)
* @param y
*   Ray start y position (fixed [15:8] int [7:0] fraction)
* @param theta
*   Angle of the ray (units of 2 * pi / 2^16)
* @param phi
*   Angle of the player (used to find distance)
* @param world
*   World/level array
* @param worldw
*   How many columns are in the world
*/

#define QX (quadrant & 0x1)
#define QY (quadrant & 0x2)
/* Find distance of vertical wall hit */
uint16_t rc_find_ri(uint16_t x, uint16_t y, uint16_t theta, uint8_t quadrant, const uint8_t *world, uint8_t worldw, uint16_t *tile, uint8_t *to)
{
  uint16_t first_step;
  uint16_t y_step;
  uint16_t x_step;
  uint16_t old_x;

  old_x = x;
  y_step = mm_tant(theta);

  /* Shift to starting point */
  if (QX) {
	first_step = ((uint32_t) y_step * (uint32_t) (x & 0xFF)) >> 8;
	x_step = 0xFF00;
	x &= 0xFF00;
  }
  else {
	first_step = ((uint32_t) y_step * (uint32_t) (0x0100 - (x & 0xFF))) >> 8;
	x += 0x0100;
	x_step = 0x0100;
	x &= 0xFF00;
  }
  if (!QY) { y_step = 0 - y_step; first_step = 0 - first_step; }
  y += first_step;

  /* Injected */
  uint16_t start_x = x, start_y = y;

  /* Loop until wall is hit */
  while (!(*tile = world[(y >> 8) * worldw + (x >> 8) - (QX)])) {
	x += x_step;
	y += y_step;
	if ((y >> 8) >= worldw) {
	  *tile = 1;
	  return 0xFFFF;
	}
  }

  *to = (y & 0xFF);

  /* Injected code */
  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 64);
  int ex = (x >> 8) * TILE_SIZE + (x & 0xFF) * (double) TILE_SIZE / 256.0;
  int ey = (y >> 8) * TILE_SIZE + (y & 0xFF) * (double) TILE_SIZE / 256.0;
  int sx = (start_x >> 8) * TILE_SIZE + (start_x & 0xFF) * (double) TILE_SIZE / 256.0;;
  int sy = (start_y >> 8) * TILE_SIZE + (start_y & 0xFF) * (double) TILE_SIZE / 256.0;;
  SDL_RenderDrawLine(renderer, sx, sy, ex, ey);

  if (QX) return ((uint32_t) mm_sect(theta) * (uint32_t) (old_x - x)) >> 8;
  else    return ((uint32_t) mm_sect(theta) * (uint32_t) (x - old_x)) >> 8;
}

/* Find distance of horizontal wall hit */
uint16_t rc_find_rj(uint16_t x, uint16_t y, uint16_t theta, uint8_t quadrant, const uint8_t *world, uint8_t worldw, uint16_t *tile, uint8_t *to)
{
  uint16_t first_step;
  uint16_t x_step;
  uint16_t y_step;
  uint16_t old_y;

  old_y = y;
  x_step = mm_cott(theta);

  /* Shift to starting point */
  if (QY) {
	first_step = ((uint32_t) x_step * (uint32_t) (0x0100 - (y & 0xFF))) >> 8;
	y_step = 0x0100;
	y &= 0xFF00;
	y += 0x0100;
  }
  else {
	first_step = ((uint32_t) x_step * (uint32_t) (y & 0xFF)) >> 8;
	y_step = 0xFF00;
	y &= 0xFF00;
  }
  if (QX) { x_step = 0 - x_step; first_step = 0 - first_step; }
  x += first_step;

  /* Injected */
  uint16_t start_x = x, start_y = y;

  /* Loop until wall is hit */
  if (QY) {
	while (!(*tile = world[(y >> 8) * worldw + (x >> 8)])) {
	  x += x_step;
	  y += y_step;
	  if ((x >> 8) >= worldw) {
		*tile = 1;
		return 0xFFFF;
	  }
	}
  }
  else {
	while (!(*tile = world[((y - 0x0100) >> 8) * worldw + (x >> 8)])) {
	  x += x_step;
	  y += y_step;
	  if ((x >> 8) >= worldw) {
		*tile = 1;
		return 0xFFFF;
	  }
	}
  }

  *to = (x & 0xFF);

  /* Injected code */
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 64);
  int ex = (x >> 8) * TILE_SIZE + (x & 0xFF) * (double) TILE_SIZE / 256.0;
  int ey = (y >> 8) * TILE_SIZE + (y & 0xFF) * (double) TILE_SIZE / 256.0;
  int sx = (start_x >> 8) * TILE_SIZE + (start_x & 0xFF) * (double) TILE_SIZE / 256.0;;
  int sy = (start_y >> 8) * TILE_SIZE + (start_y & 0xFF) * (double) TILE_SIZE / 256.0;;
  SDL_RenderDrawLine(renderer, sx, sy, ex, ey);

  // Level gets loaded once, there should be spongebobs

  if (QY) return ((uint32_t) mm_csct(theta) * (uint32_t) (y - old_y)) >> 8;
  else    return ((uint32_t) mm_csct(theta) * (uint32_t) (old_y - y)) >> 8;
}

void rc_draw_world_ray2(uint8_t screen_x, uint16_t x, uint16_t y, uint16_t theta, uint16_t phi, const uint8_t* world, uint8_t worldw)
{
  uint16_t ri;
  uint16_t rj;
  uint16_t distance;
  uint16_t tilei, tilej, tile;
  uint8_t toi, toj, to;
  uint16_t data;

  /* Turn phi into difference between theta and phi */
  phi = theta - phi;
  if (phi > FOV_2) phi = 0 - phi;

  /* Convert theta */
  data = 0;
  while (theta > 0x4000) {
	theta -= 0x4000;
	++data;
  }
  if (data & 0x1) theta = 16384 - theta; /* Make sure theta is based off x-axis and not y */
  if (data > 1) data ^= 0x1; /* Convert quadrant to format specified above */

  ri = rc_find_ri(x, y, theta, data & 0xFF, world, worldw, &tilei, &toi);
  rj = rc_find_rj(x, y, theta, data & 0xFF, world, worldw, &tilej, &toj);

  if (ri < rj) { distance = ri; tile = tilei; to = toi; }
  else		   { distance = rj; tile = tilej; to = toj; data |= 0x8000; }

  /* Correct distance */
  distance = ((uint32_t) distance * mm_cost(phi)) >> 16;

  data = (data & ~0xFF) | to;
  rc_draw_world_strip(screen_x, distance, data, tile);

  /* Injected code */
  /*
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
  int sx = (x >> 8) * TILE_SIZE + (x & 0xFF) * (double) TILE_SIZE / 256.0;
  int sy = (y >> 8) * TILE_SIZE + (y & 0xFF) * (double) TILE_SIZE / 256.0;
  int ex;
  int ey;
  SDL_RenderDrawLine(renderer, sx, sy, ex, ey);
  */
}

#define COLLISION (data & 0x8000)
#define QUADRANT  (data & 0x6000)
#define QX (data & 0x2000)
#define QY (data & 0x4000)
void rc_draw_world_ray(uint8_t screen_x, uint16_t x, uint16_t y, uint16_t theta, uint16_t phi, const uint8_t *world, uint8_t worldw)
{
  uint16_t data; /* Collision [15] 0 is x, 1 is y, Quadrant [1:0] YX, 0 is positive, 1 is negative */
  uint16_t ri, rj, drj, dri;
  uint16_t distance;
  uint16_t dx, dy;
  uint8_t tile;

  /* Injected code */
  uint16_t old_x = x, old_y = y;

  /*
  * 1. Initial values
  * 
  * Initial values are all based off the first quadrant,
  * the qaudrant variable determines how to step (+/-)
  */
  phi = theta - phi;
  if (phi > FOV_2) phi = 0 - phi;
  phi = 0x4000 - phi; /* Phi is now theta_s(ine) */
  /* Figure out the quadrant (real quadrant is quadrant + 1) */
  data = 0;
  while (theta > 0x4000) {
	theta -= 0x4000;
	++data;
  }
  if (data & 0x1) theta = 16384 - theta; /* Make sure theta is based off x-axis and not y */
  if (data > 1) data ^= 0x1; /* Convert quadrant to format specified above */
  data <<= 13;
							 /* Other calculations */
  rj = mm_csct(theta);
  ri = mm_sect(theta);

  if (QX) dri = ((uint32_t) ri * (uint32_t) (x & 0x00FF)) >> 8;            /* Negative x */
  else    dri = ((uint32_t) ri * (uint32_t) (0x0100 - (x & 0x00FF))) >> 8; /* Positive x */

  if (QY) drj = ((uint32_t) rj * (uint32_t) (y & 0x00FF)) >> 8;            /* Negative y */
  else    drj = ((uint32_t) rj * (uint32_t) (0x0100 - (y & 0x00FF))) >> 8; /* Positive y */

																		   /*
																		   * 2. Traverse grid until a wall is hit
																		   *
																		   * Because there is no initial jump, the wall will be
																		   * hit inside of it, if that makes any sense
																		   */
  if (QX) dx = 0xFF00;
  else    dx = 0x0100;
  if (QY) dy = 0x0100;
  else    dy = 0xFF00;

  while (1) {
	if (drj < dri) {
	  drj += rj;
	  y += dy;
	  data |= 0x8000;
	}
	else {
	  dri += ri;
	  x += dx;
	  data &= ~0x8000;
	}
	if ((tile = world[(y >> 8) * worldw + (x >> 8)])) break;
  }

  /*
  * 3. Calculate distance and wall hit
  *
  * Reuse theta to find the fractional part of
  * the wall hit
  */
#define LONG_EXP(fn) (((uint32_t) distance * (uint32_t) mm_##fn##t(theta)) >> 16)
  if (COLLISION) {
	distance = (drj - rj);
	if (QX) data = (data & ~0xFF) | (((x & 0xFF) + (0x0100 - LONG_EXP(cos))) & 0xFF);
	else    data = (data & ~0xFF) | (((x & 0xFF) + LONG_EXP(cos)) & 0xFF);
  }
  else {
	distance = (dri - ri);
	if (QY) data = (data & ~0xFF) | (((y & 0xFF) + LONG_EXP(sin)) & 0xFF);
	else    data = (data & ~0xFF) | (((y & 0xFF) + (0x0100 - LONG_EXP(sin))) & 0xFF);
  }

  /*
  * 4. Draw the thing
  */	
  rc_draw_world_strip(screen_x, distance, data, tile);

  /* Injected code */
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
  int sx = (old_x >> 8) * TILE_SIZE + (old_x & 0xFF) * (double) TILE_SIZE / 256.0;
  int sy = (old_y >> 8) * TILE_SIZE + (old_y & 0xFF) * (double) TILE_SIZE / 256.0;
  int ex = (x     >> 8) * TILE_SIZE;
  int ey = (y     >> 8) * TILE_SIZE;
  if (COLLISION) {
	ex += (data & 0xFF) * (double) TILE_SIZE / 256.0;
  }
  else {
	ey += (data & 0xFF) * (double) TILE_SIZE / 256.0;
  }
  SDL_RenderDrawLine(renderer, sx, sy, ex, ey);
}

/*
* draw_world
*
* @desc
*   Renders the world, meaning all tiles and no sprites,
*   but does set the z buffer
* @param x
*   Player x (fixed [15:8] int [7:0] fraction)
* @param y
*   Player y (fixed [15:8] int [7:0] fraction)
* @param phi
*   Player angle (units of 2 * pi / 2^16)
* @param world
*   World/level array
* @param worldw
*   How many columns are in the world
*/
void rc_draw_world(uint16_t x, uint16_t y, uint16_t phi, const uint8_t *world, uint8_t worldw)
{
  /* Injected functions */
  draw_world(x, y, phi, world, worldw);

  /* Cast a ray for every x position */
  uint8_t i;
  uint16_t angle;
  for (i = 0, angle = phi - FOV_2; i < WIN_H; ++i, angle += FOV_STEP) {
	rc_draw_world_ray2(i, x, y, angle, phi, world, worldw);
  }
}

#include <math.h>
static void draw_world(uint16_t x, uint16_t y, uint16_t phi, const uint8_t* world, uint8_t worldw)
{
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  /* The world */
  if (worldw != 16) return;
  for (int row = 0; row < 16; ++row) {
	for (int col = 0; col < 16; ++col) {
	  SDL_Rect r = {col * TILE_SIZE + 1, row * TILE_SIZE + 1, TILE_SIZE - 2, TILE_SIZE - 2};
	  switch (world[row * 16 + col]) {
	  case 0:
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		break;
	  default:
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	  }
	  SDL_RenderFillRect(renderer, &r);
	}
  }
  /* Lines */
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  for (int row = 0; row < 512 / TILE_SIZE; ++row) {
	SDL_RenderDrawLine(renderer, 0, row * TILE_SIZE, 511, row * TILE_SIZE);
	SDL_RenderDrawLine(renderer, row * TILE_SIZE, 0, row * TILE_SIZE, 511);
  }
  /* The player */
  SDL_Rect r = {
	(x >> 8) * TILE_SIZE + (x& 0xFF) * (double) TILE_SIZE / 256.0 - 1,
	(y >> 8) * TILE_SIZE + (y& 0xFF) * (double) TILE_SIZE / 256.0 - 1,
	3, 3};
  SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
  SDL_RenderFillRect(renderer, &r);
  /* Phi marker */
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  double ex = (cos(phi * ((2 * M_PI) / 65536)) * 16);
  double ey = (sin(phi * ((2 * M_PI) / 65536)) * 16);
  SDL_RenderDrawLine(renderer, r.x + 1, r.y + 1, 
	r.x + (int) ex, 
	r.y - (int) ey
  );
  /* Correct Ray directions */
  uint8_t i;
  uint32_t angle;
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 16);
  for (i = 0, angle = phi - FOV_2; i < WIN_H; ++i, angle += FOV_STEP) {
	SDL_RenderDrawLine(renderer, r.x + 1, r.y + 1, 
	  r.x + 1 + cos(angle * ((2 * M_PI) / 65536)) * 64,
	  r.y + 1 - sin(angle * ((2 * M_PI) / 65536)) * 64);
  }
  /* Hitbox */
  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
  r.x += 1;
  r.y += 1;
  r.x -= HITBOX_SIZE / (64 / TILE_SIZE) / 2;
  r.y -= HITBOX_SIZE / (64 / TILE_SIZE) / 2;
  r.w = r.h = HITBOX_SIZE / (64 / TILE_SIZE) + 1;
  SDL_RenderDrawRect(renderer, &r);
}