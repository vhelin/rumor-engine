
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "defines.h"
#include "../rumor/person.h"
#include "../rumor/rumor.h"
#include "level.h"
#include "item.h"
#include "ai.h"


/* level data */
struct level *levels[256];
int level_current = 100;

/* view data */
char *level_view = NULL;
int level_view_dx = 0, level_view_dy = 0;


int level_init(void) {

  int i;


  /* init the structures */
  for (i = 0; i < 256; i++)
    levels[i] = NULL;

  /* load the first level */
  level_load(level_current);

  return SUCCEEDED;
}


int level_load(int level) {

  unsigned char c, *t;
  struct item **m;
  struct level *l;
  struct ai **a;
  char tmp[256];
  FILE *f;
  int dx, dy, i;
  float *b, *d;


  if (levels[level] != NULL) {
    fprintf(stderr, "LEVEL_LOAD: Level %3d is already in memory.\n", level);
    return FAILED;
  }

  sprintf(tmp, "data/%3d.txt", level);
  f = fopen(tmp, "rb");
  if (f == NULL) {
    fprintf(stderr, "LEVEL_LOAD: Could not open file \"%s\" for reading.\n", tmp);
    return FAILED;
  }

  /* compute the level's dimensions */
  dx = 0;
  dy = 0;
  while (fscanf(f, "%c", &c) != EOF) {
    if (c == 0xA)
      dy++;
    else if (dy == 0)
      dx++;
    if (c == '@')
      break;
  }

  l = calloc(sizeof(struct level), 1);
  t = calloc(dx*dy, 1);
  m = calloc(sizeof(struct item *) * dx*dy, 1);
  a = calloc(sizeof(struct ai *) * dx*dy, 1);
  b = malloc(sizeof(float) * dx*dy);
  d = malloc(sizeof(float) * dx*dy);

  if (l == NULL || t == NULL || m == NULL || a == NULL || b == NULL || d == NULL) {
    fprintf(stderr, "LEVEL_LOAD: Out of memory error.\n");
    fclose(f);
    return FAILED;
  }

  l->id = level;
  l->tiles = t;
  l->items = m;
  l->bots = a;
  l->bots_list = NULL;
  l->dx = dx;
  l->dy = dy;
  l->navigation = b;
  l->modifier = d;

  /* clear the items */
  for (i = 0; i < dx*dy; i++)
    l->items[i] = NULL;

  /* clear the bots */
  for (i = 0; i < dx*dy; i++)
    l->bots[i] = NULL;

  /* read the level tiles */
  fseek(f, 0, SEEK_SET);
  for (i = 0; i < dx*dy; ) {
    fscanf(f, "%c", &c);
    if (c == 0xA)
      continue;
    l->tiles[i++] = c;
  }

  /* skip the end-of-map marker */
  while (fscanf(f, "%c", &c) != EOF) {
    if (c == '@')
      break;
  }

  /* load all items */
  item_load(l, f);

  fclose(f);

  levels[level] = l;

  return SUCCEEDED;
}


/* collect the final level image from all essential data structures */
int level_display(struct level *l, int x, int y, int dx, int dy) {

  struct item *m;
  struct ai *a;
  int ax, ay, ex, ey, i, bx, by;
  char tmp[256];


  if (level_view_dx*level_view_dy != dx*dy) {
    level_view_dx = dx;
    level_view_dy = dy;
    level_view = realloc(level_view, dx*dy);
    if (level_view == NULL) {
      level_view_dx = 0;
      level_view_dy = 0;
      fprintf(stderr, "LEVEL_DISPLAY: Out of memory error.\n");
      return FAILED;
    }
  }

  if (dx > l->dx)
    dx = l->dx;
  if (dy > l->dy)
    dy = l->dy;

  ax = x - dx/2;
  ay = y - dy/2;
  ex = x + dx/2;
  ey = y + dy/2;

  if (ax < 0) {
    ax = 0;
    ex = dx-1;
  }
  if (ay < 0) {
    ay = 0;
    ey = dy-1;
  }
  if (ex >= l->dx) {
    ax = l->dx - dx;
    ex = l->dx - 1;
  }
  if (ey >= l->dy) {
    ay = l->dy - dy;
    ey = l->dy - 1;
  }

  /* copy the base */
  for (i = 0, by = ay; by <= ey; by++) {
    for (bx = ax; bx <= ex; bx++) {
      level_view[i++] = l->tiles[by*l->dx + bx];
    }
  }

  /* insert the items */
  for (i = 0, by = ay; by <= ey; by++) {
    for (bx = ax; bx <= ex; bx++, i++) {
      m = l->items[by*l->dx + bx];
      if (m == NULL)
	continue;
      if (m->type == ITEM_TYPE_KEY)
	level_view[i] = '(';
      else if (m->type == ITEM_TYPE_FOOD)
	level_view[i] = '%';
    }
  }

  /* insert the bots */
  for (i = 0, by = ay; by <= ey; by++) {
    for (bx = ax; bx <= ex; bx++, i++) {
      a = l->bots[by*l->dx + bx];
      if (a == NULL)
	continue;

      if (a->mem.self->race == AI_RACE_HUMAN)
	level_view[i] = '@';
      else if (a->mem.self->race == AI_RACE_ORC)
	level_view[i] = 'o';
      else if (a->mem.self->race == AI_RACE_CAT)
	level_view[i] = 'f';
      else if (a->mem.self->race == AI_RACE_RAT)
	level_view[i] = 'r';


      /* update the on screen coordinates */
      a->sx = bx - ax;
      a->sy = by - ay;
    }
  }

  /* print the buffer line by line */
  for (by = 0; by <= dy; by++) {
    for (i = 0, bx = 0; bx < dx; bx++) {
      /* print the food symbol correctly */
      if (level_view[by*dx + bx] == '%') {
	tmp[i++] = '%';
	tmp[i++] = '%';
      }
      else
	tmp[i++] = level_view[by*dx + bx];
    }
    tmp[i] = 0;
    strcat(tmp, "\n");
    printw(tmp);
  }

  return SUCCEEDED;
}
