#include "sdlinput.h"

struct input inputs;

void sdlinput_init(void)
{
  inputs.w = inputs.a = inputs.s = inputs.d = inputs.left = inputs.right = 0;
}

struct input sdlinput_get(void)
{
  return inputs;
}