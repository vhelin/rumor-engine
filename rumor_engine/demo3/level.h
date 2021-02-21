
#ifndef __LEVEL_H
#define __LEVEL_H

/* level data */
extern struct level *levels[256];
extern int level_current;

struct level {
  int id;
  int dx, dy;
  struct item **items;
  struct ai **bots, *bots_list;
  unsigned char *tiles;
  float *navigation, *modifier;
};

int level_init(void);
int level_load(int level);
int level_display(struct level *l, int x, int y, int dx, int dy);

#endif
