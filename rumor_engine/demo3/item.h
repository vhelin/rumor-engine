
#ifndef _ITEM_H
#define _ITEM_H

#define ITEM_TYPE_KEY  0
#define ITEM_TYPE_FOOD 1

#define FOOD_TYPE_CORPSE 0
#define FOOD_TYPE_APPLE  1

struct item {
  int type;
  int a, b;
  int x, y;
  float condition; /* [0, 1] */
  struct item *next, *prev;
};

int item_new(struct item **i);
int item_load(struct level *l, FILE *f);

#endif
