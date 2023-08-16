#include "sdlgfx.h"
#include <SDL.h>

/* Color palette (Color indices lookup) [0 is transparent/don't draw] */
static const uint32_t sdlgfx_palette[256] = {
  /* Basic colors */
  0x000000, 0x000000, 0xffffff, 0xff0000, 0x00ff00, 0x0000ff, 0xffffff, 0xffffff,
  /* Palette */
  0x472d3c, 0x5e3643, 0x7a444a, 0xa05b53, 0xbf7958, 0xeea160, 0xf4cca1, 0xb6d53c,
  0x71aa34, 0x397b44, 0x3c5956, 0x302c2e, 0x5a5353, 0x7d7071, 0xa0938e, 0xcfc6b8,
  0xdff6f5, 0x8aebf1, 0x28ccdf, 0x3978a8, 0x394778, 0x39314b, 0x564064, 0x8e478c,
  0xcd6093, 0xffaeb6, 0xf4b41b, 0xf47e1b, 0xe6482e, 0xa93b3b, 0x827094, 0x4f546b,
  /* Darker palette */
  0x23161e, 0x2f1b21, 0x3d2225, 0x502d29, 0x5f3c2c, 0x775030, 0x7a6650, 0x5b6a1e,
  0x38551a, 0x1c3d22, 0x1e2c2b, 0x181617, 0x2d2929, 0x3e3828, 0x504947, 0x67635c,
  0x6f7a7a, 0x457578, 0x14666f, 0x1c3c54, 0x1c233c, 0x1c1825, 0x2b2032, 0x472346,
  0x663049, 0x7f575b, 0x7a5a0d, 0x7a3f0d, 0x732417, 0x541d1d, 0x41384a, 0x272a35
};

/* The color index buffer */
#define CIB_LEN (WIN_W * WIN_H)
static volatile uint8_t sdlgfx_cib[CIB_LEN];

SDL_Renderer *renderer;

/*
* sdlgfx_init
*
* @desc
*   Initializes everything needed for
*   graphics to run
*/
void sdlgfx_init(SDL_Window *window)
{
  renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void sdlgfx_quit(void)
{
  SDL_DestroyRenderer(renderer);
}

/*
* sdlgfx_send
*
* @desc
*   Sends the framebuffer to the
*   screen
*/
void sdlgfx_send(void)
{
  uint32_t x = 512;
  uint32_t y = -1;
  for (uint32_t i = 0; i < CIB_LEN; ++i) {
	if (i % WIN_W == 0) {
	  x = 512;
	  ++y;
	}
	SDL_SetRenderDrawColor(renderer,
							((sdlgfx_palette[sdlgfx_cib[i]] & 0xFF0000) >> 16),
							((sdlgfx_palette[sdlgfx_cib[i]] & 0x00FF00) >> 8),
							((sdlgfx_palette[sdlgfx_cib[i]] & 0x0000FF) >> 0),							
							255);
	SDL_RenderDrawPoint(renderer, x, y);
	++x;
  }

  SDL_RenderPresent(renderer);
}

/*
* draw_pixel
*
* @desc
*   Draws a single pixel
*   (no transparency)
* @param x
*   x coordinate of the pixel (0 - WIN_XMAX)
* @param y
*   y coordinate of the pixel (0 - WIN_YMAX)
* @param color_index
*   Index of the color on the palette
*/
void sdlgfx_draw_pixel(uint8_t x, uint8_t y, uint8_t color_index)
{
  sdlgfx_cib[y * WIN_W + x] = color_index;
}

/*
* draw_rect
*
* @desc
*   Draws a filled in rectangle of a single color
*   (no transparency)
* @param x
*   x coordinate of the top left (0 - WIN_XMAX)
* @param y
*   y coordinate of the top left (0 - WIN_YMAX)
* @param w
*   Width of the rectangle
* @param h
*   Height of the rectangle
* @param color_index
*   Index of the color on the palette
*/
void sdlgfx_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color_index)
{
  if (x + (w - 1) > WIN_XMAX) w = WIN_XMAX - x + 1;
  if (y + (h - 1) > WIN_YMAX) h = WIN_YMAX - y + 1;

  for (uint8_t row = 0; row < h; ++row) {
	for (uint8_t col = 0; col < w; ++col) {
	  sdlgfx_cib[(y + row) * WIN_W + (x + col)] = color_index;
	}
  }
}

/*
* draw_array
*
* @desc
*   Draws an array
* @param x
*   x coordinate of the top left (0 - WIN_XMAX)
* @param y
*   y coordinate of the top left (0 - WIN_YMAX)
* @param w
*   Width of the array (columns)
* @param h
*   Height of the array (rows)
* @param array
*   Array of color indices
*/
void sdlgfx_draw_array(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *array)
{
  uint32_t i = 0;
  for (uint8_t iy = y; iy < y + h; ++iy) {
	if (iy > WIN_YMAX) break;
	for (uint8_t ix = x; ix < x + w; ++ix) {
	  if (ix > WIN_XMAX) {
		i += w;
		i -= WIN_W - x;
		break;
	  }
	  uint8_t color_index;
	  color_index = array[i++];
	  if (color_index) sdlgfx_cib[iy * WIN_W + ix] = color_index;
	}
  }
}