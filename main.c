#include <SDL.h>
#include <stdio.h>
#include "sdlgfx.h"
#include "level.h"
#include "sdlinput.h"
extern struct input inputs;

/* Raycasting stuff */
#define FB_WIDTH 128
#define FB_HEIGHT 160

#define WINDOW_WIDTH  (512 + FB_WIDTH)
#define WINDOW_HEIGHT (512)

/* Loop function to call, its initialized to a dummy function for safety */
void (*loop_fn) (void);
void dummy_fn(void) {}


int main(int argc, char* args[])
{
  SDL_Window *window = NULL;
  int         running = 1;
  SDL_Event   e;

  /* Init SDL */
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL_Init failed: %s\n", SDL_GetError());
    return -1;
  }

  window = SDL_CreateWindow("Raycast Debug", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
  }

  /* Init game stuff */
  sdlgfx_init(window);
  sdlinput_init();

  loop_fn = dummy_fn;

  /* Initial state */
  level_init(0);

  /* Loop */
  while (running) {
    loop_fn();
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = 0;
      }
      else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_w:
          inputs.w = 1;
          break;
        case SDLK_a:
          inputs.a = 1;
          break;
        case SDLK_s:
          inputs.s = 1;
          break;
        case SDLK_d:
          inputs.d = 1;
          break;
        case SDLK_LEFT:
          inputs.left = 1;
          break;
        case SDLK_RIGHT:
          inputs.right = 1;
          break;
        }
      }
      else if (e.type == SDL_KEYUP) {
        switch (e.key.keysym.sym) {
        case SDLK_w:
          inputs.w = 0;
          break;
        case SDLK_a:
          inputs.a = 0;
          break;
        case SDLK_s:
          inputs.s = 0;
          break;
        case SDLK_d:
          inputs.d = 0;
          break;
        case SDLK_LEFT:
          inputs.left = 0;
          break;
        case SDLK_RIGHT:
          inputs.right = 0;
          break;
        }
      }
    }
  }

  sdlgfx_quit();
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}