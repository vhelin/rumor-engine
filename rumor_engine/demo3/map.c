
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "defines.h"
#include "../rumor/person.h"
#include "../rumor/rumor.h"
#include "level.h"
#include "item.h"
#include "map.h"
#include "ai.h"


int map_free(struct map *m) {

  if (m == NULL)
    return FAILED;

  if (m->items != NULL)
    free(m->items);
  if (m->bots != NULL)
    free(m->bots);
  if (m->tiles != NULL)
    free(m->tiles);

  return SUCCEEDED;
}


int map_new(int dx, int dy, struct map **m) {

  struct map *t;


  if (dx <= 0 || dy <= 0 || m == NULL)
    return FAILED;

  t = calloc(sizeof(struct map), 1);
  if (t == NULL) {
    fprintf(stderr, "MAP_NEW: Out of memory error.\n");
    return FAILED;
  }

  t->dx = dx;
  t->dy = dy;
  t->items = calloc(sizeof(struct map_item *) * dx*dy, 1);
  t->bots = calloc(sizeof(struct map_ai *) * dx*dy, 1);
  t->tiles = calloc(dx*dy, 1);
  if (t->items == NULL || t->bots == NULL || t->tiles == NULL) {
    fprintf(stderr, "MAP_NEW: Out of memory error.\n");
    map_free(t);
    return FAILED;
  }

  *m = t;

  return SUCCEEDED;
}


/* the ai looks around, and stores what it sees into its memory */
int map_look(struct level *l, struct map *m, struct ai *a, int r) {

  struct map_item *mi, *mt, **mp;
  struct map_ai *am;
  struct item *it;
  struct ai *at;
  int sx, sy, ex, ey, x, y;
  float dx, dy;


  if (l == NULL || m == NULL || a == NULL)
    return FAILED;

  sx = a->mx - r;
  sy = a->my - r;
  ex = a->mx + r;
  ey = a->my + r;

  if (sx < 0)
    sx = 0;
  if (sy < 0)
    sy = 0;
  if (ex > l->dx)
    ex = l->dx;
  if (ey > l->dy)
    ey = l->dy;

  /* copy part of the level map to the ai's memory */
  for (y = sy; y < ey; y++) {
    for (x = sx; x < ex; x++) {
      /* does the ai see the tile? */
      dx = a->mx - x;
      dy = a->my - y;
      if (dx*dx + dy*dy > r*r)
        continue;

      /* copy the tile */
      m->tiles[y*m->dx + x] = l->tiles[y*l->dx + x];

      /* handle the items */
      if (l->items[y*l->dx + x] == NULL) {
        if (m->items[y*m->dx + x] != NULL) {
          /* free the lost items */
          mi = m->items[y*m->dx + x];
          while (mi != NULL) {
            mt = mi->next;
            free(mi);
            mi = mt;
          }
          m->items[y*m->dx + x] = NULL;
        }
      }
      else {
        it = l->items[y*l->dx + x];
        if (m->items[y*m->dx + x] != NULL) {
          /* do the items match? */
          mi = m->items[y*m->dx + x];
          while (mi != NULL) {
            if (it->type != mi->type)
              break;
            mi = mi->next;
            it = it->next;
          }

          if (mi != NULL || it != NULL) {
            /* no match -> free the old ones */
            mi = m->items[y*m->dx + x];
            while (mi != NULL) {
              mt = mi->next;
              free(mi);
              mi = mt;
            }
            m->items[y*m->dx + x] = NULL;
            it = l->items[y*l->dx + x];
          }
        }

        /* it != NULL -> copy the items */
        mp = &(m->items[y*m->dx + x]);
        *mp = NULL;
        while (it != NULL) {
          mi = malloc(sizeof(struct map_item));
          if (mi == NULL) {
            fprintf(stderr, "MAP_LOOK: Out of memory error.\n");
            return FAILED;
          }
          mi->type = it->type;
          it = it->next;
          if (*mp != NULL)
            (*mp)->next = mi;
          mi->prev = *mp;
          mi->next = NULL;
          *mp = mi;
          mp = &(mi->next);
        }
      }

      /* handle the ai bots */
      if (l->bots[y*l->dx + x] == NULL) {
        if (m->bots[y*m->dx + x] != NULL) {
          free(m->bots[y*m->dx + x]);
          m->bots[y*m->dx + x] = NULL;
        }
      }
      else {
        at = l->bots[y*l->dx + x];
        if (m->bots[y*m->dx + x] != NULL) {
          am = m->bots[y*m->dx + x];
          if (am->race != at->mem.self->race || am->id != at->mem.self->id) {
            /* bots differ -> free the old memory */
            free(m->bots[y*m->dx + x]);
            m->bots[y*m->dx + x] = NULL;
          }
        }

        if (m->bots[y*m->dx + x] == NULL) {
          /* remember the new visual */
          am = malloc(sizeof(struct map_ai));
          if (am == NULL) {
            fprintf(stderr, "MAP_LOOK: Out of memory error.\n");
            return FAILED;
          }
          am->id = at->mem.self->id;
          am->race = at->mem.self->race;
          m->bots[y*m->dx + x] = am;
        }
      }
    }
  }

  /* DEBUG
     {
     FILE *f;
     char tmp[256];

     sprintf(tmp, "memory_%d.txt", (int)a);
     f = fopen(tmp, "wb");
     for (y = 0; y < m->dy; y++) {
     for (x = 0; x < m->dx; x++) {
     if (m->bots[y*m->dx + x] != NULL)
     fprintf(f, "@");
     else if (m->items[y*m->dx + x] != NULL)
     fprintf(f, "?");
     else if (m->tiles[y*m->dx + x] != 0)
     fprintf(f, "%c", m->tiles[y*m->dx + x]);
     else
     fprintf(f, " ");
     }
     fprintf(f, "\n");
     }
     fclose(f);
     }
  */

  return SUCCEEDED;
}
