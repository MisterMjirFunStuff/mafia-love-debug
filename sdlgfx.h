#ifndef SDLGFX_H
#define SDLGFX_H

#include <SDL.h>
#include <stdint.h>
//#include "sdlgfxt.h"

/*
* Mihir Graphics (gfx)
*
* The ST7735 library we have is kinda slow, so
* I went ahead and made a (hopefully) faster one
*
* This is not a general purpose library, it is
* designed specifically for our game, so you
* might not be able to use it for your use
*
* This uses a color index framebuffer (there
* is a palette array and the "framebuffer" is
* indices of the pallete)
*/
#define WIN_W 128
#define WIN_H 160
#define WIN_XMAX (WIN_W - 1)
#define WIN_YMAX (WIN_H - 1)

void sdlgfx_init       (SDL_Window *window);
void sdlgfx_quit       (void);
void sdlgfx_send       (void);
void sdlgfx_draw_pixel (uint8_t x, uint8_t y, uint8_t color_index);
void sdlgfx_draw_rect  (uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color_index);
void sdlgfx_draw_array (uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *array);

#endif