#ifndef SDLINPUT_H
#define SDLINPUT_H

struct input {
  int w, a, s, d;
  int left, right;
};

void         sdlinput_init(void);
struct input sdlinput_get (void);

#endif