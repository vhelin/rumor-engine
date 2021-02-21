
#ifndef __MAP_H
#define __MAP_H

struct map_item {
  int type;
  struct map_item *next, *prev;
};

struct map_ai {
  int id;
  int race;
};

struct map {
  int dx, dy;
  struct map_item **items;
  struct map_ai **bots;
  unsigned char *tiles;
};

int map_free(struct map *m);
int map_new(int dx, int dy, struct map **m);
int map_look(struct level *l, struct map *m, struct ai *a, int r);

#endif
