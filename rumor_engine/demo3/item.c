
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "level.h"
#include "item.h"



int item_new(struct item **i) {

  if (i == NULL)
    return FAILED;

  *i = calloc(sizeof(struct item), 1);
  if (*i == NULL) {
    fprintf(stderr, "ITEM_NEW: Out of memory error.\n");
    return FAILED;
  }

  return SUCCEEDED;
}


int item_load(struct level *l, FILE *f) {

  struct item *i;
  char tmp[256];
  int x, y;


  if (l == NULL || f == NULL)
    return FAILED;

  while (fscanf(f, "%d %d", &x, &y) != EOF) {
    if (x < 0 || y < 0 || x >= l->dx || y >= l->dy)
      continue;

    if (item_new(&i) == FAILED)
      return FAILED;

    /* location */
    i->x = x;
    i->y = y;

    i->next = l->items[y*l->dx + x];
    i->prev = NULL;
    if (l->items[y*l->dx + x] != NULL)
      l->items[y*l->dx + x]->prev = i;
    l->items[y*l->dx + x] = i;

    /* condition */
    i->condition = 1.0f;

    /* type */
    fscanf(f, "%255s", tmp);

    if (strcmp(tmp, "key") == 0) {
      /* KEY */
      i->type = ITEM_TYPE_KEY;
      fscanf(f, "%d %d", &(i->a), &(i->b));
    }
    else if (strcmp(tmp, "food") == 0) {
      /* FOOD */
      i->type = ITEM_TYPE_FOOD;

      /* type */
      fscanf(f, "%255s", tmp);

      if (strcmp(tmp, "apple") == 0)
        i->a = FOOD_TYPE_APPLE;
    }
  }

  return SUCCEEDED;
}
