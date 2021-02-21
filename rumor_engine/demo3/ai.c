
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
#include "main.h"


/* action table for the rumor engine */
struct action actions[] = {
  { ACTION_FART, ACTION_STATUS_OTHER,     0.1, 0.8, 1.0, 1.0, 0.0, 0.0, 0.0, +0.1 },
  { ACTION_SALIVATE, ACTION_STATUS_OTHER, 0.1, 0.8, 1.0, 1.0, 0.0, 0.0, 0.0, -0.1 },
  { ACTION_HIT, ACTION_STATUS_OTHER,      0.6, 0.8, 1.0, 1.0, 0.0, 0.0, 0.0, -0.2 },
  { ACTION_SAW_WATER, ACTION_STATUS_IS,   0.7, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.0 },
  { ACTION_DESCRIPTION, ACTION_STATUS_DESCRIPTION, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
};


int ai_init(void) {

  /* init the rumor engine */
  rumor_init(actions);

  return SUCCEEDED;
}


int ai_free(struct ai *a) {

  struct mode *m1, *m2;
  int i;


  if (a == NULL)
    return FAILED;

  /* free the modes */
  m1 = a->stats.mode;
  while (m1 != NULL) {
    m2 = m1->next;
    free(m1);
    m1 = m2;
  }

  /* free the maps */
  for (i = 0; i < 256; i++) {
    if (a->maps[i] != NULL)
      map_free(a->maps[i]);
  }

  free(a);

  return SUCCEEDED;
}


int ai_dies(struct level *l, struct ai *a) {

  struct item *i, *m;
  int p;


  if (l == NULL || a == NULL)
    return FAILED;

  /* compute the position */
  p = a->my*l->dx + a->mx;

  /* remove the bot */
  l->bots[p] = NULL;

  if (a->prev != NULL)
    a->prev->next = a->next;
  else
    l->bots_list = a->next;

  if (a->next != NULL)
    a->next->prev = a->prev;

  /* drop the items */
  i = a->items;
  while (i != NULL) {
    m = i->next;

    i->next = l->items[p];
    i->prev = NULL;

    if (l->items[p] != NULL)
      l->items[p]->prev = i;
    l->items[p] = i;

    i->x = a->mx;
    i->y = a->my;

    i = m;
  }

  a->items = NULL;

  /* replace the bot with its corpse */
  if (item_new(&i) == FAILED)
    return FAILED;

  i->type = ITEM_TYPE_FOOD;
  i->a = FOOD_TYPE_CORPSE;
  i->b = a->mem.self->race;
  i->x = a->mx;
  i->y = a->my;
  i->condition = 1.0f;

  i->next = l->items[p];
  i->prev = NULL;
  if (l->items[p] != NULL)
    l->items[p]->prev = i;
  l->items[p] = i;

  ai_free(a);

  return SUCCEEDED;
}


int ai_live_human(struct level *l, struct ai *a) {

  if (l == NULL || a == NULL)
    return FAILED;

  /* consume a bit of energy */
  a->stats.nutrition -= AI_ENERGY_LOSS_HUMAN;

  return SUCCEEDED;
}


int ai_live_orc(struct level *l, struct ai *a) {

  if (l == NULL || a == NULL)
    return FAILED;

  /* consume a bit of energy */
  a->stats.nutrition -= AI_ENERGY_LOSS_ORC;

  return SUCCEEDED;
}


int ai_live_cat(struct level *l, struct ai *a) {

  if (l == NULL || a == NULL)
    return FAILED;

  /* consume a bit of energy */
  a->stats.nutrition -= AI_ENERGY_LOSS_CAT;

  return SUCCEEDED;
}


int ai_navigate_rat(struct level *l, struct ai *a) {

  int sx, sy, ex, ey, x, y, i, r;
  struct map_item *tmpi;
  struct map_ai *tmpa;
  struct map *ma;
  unsigned char c;
  float dx, dy;


  if (l == NULL || a == NULL || r <= 0)
    return FAILED;

  if (l->dx > l->dy)
    r = l->dx;
  else
    r = l->dy;

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

  ma = a->maps[l->id];

  /* init the navigation map */
  for (y = sy; y < ey; y++) {
    for (x = sx; x < ex; x++) {
      i = y*l->dx + x;

      l->navigation[i] = l->dx*l->dy;
      c = ma->tiles[i];

      if (c == '.' || c == ' ' || c == '\'' || c == '<' || c == '>' || c == 0) {
	/* achievable tiles */
	l->modifier[i] = 1.0f;
      }
      else {
	/* blocks */
	l->modifier[i] = l->dx*l->dy;
      }
    }
  }

  /* set the targets */
  for (y = sy; y < ey; y++) {
    for (x = sx; x < ex; x++) {
      /* compute the distance to the ai bot */
      dy = y - a->my;
      dx = x - a->mx;
      if (dx*dx + dy*dy > r*r)
	continue;

      i = y*l->dx + x;
      tmpa = ma->bots[i];
      tmpi = ma->items[i];

      /* ai bots */
      if (tmpa != NULL) {
	/* one cannot walk through another bot */
	l->modifier[i] += l->dx*l->dy;
	if (tmpa->race != AI_RACE_RAT) {
	  /* rats are afraid of everyone else */
	  if (x > sx)
	    l->modifier[i-1] += 4.0f;
	  if (y > sy)
	    l->modifier[i-l->dx] += 4.0f;
	  if (x < ex-1)
	    l->modifier[i+1] += 4.0f;
	  if (y < ey-1)
	    l->modifier[i+l->dx] += 4.0f;
	}
      }
      /* no ai bot -> check the items */
      else if (tmpi != NULL) {
	/* the items */
	while (tmpi != NULL) {
	  if (tmpi->type == ITEM_TYPE_FOOD) {
	    /* set a target */
	    l->navigation[i] = 0.0f;
	  }
	  tmpi = tmpi->next;
	}
      }
      /* no items -> check the land type */
      else if (ma->tiles[i] == 0) {
	/* uncharted waters! */
	l->navigation[i] = 0.5f;
      }
    }
  }

  ai_propagate(l->dx, sy, ey, sx, ex, l->navigation, l->modifier);

  return SUCCEEDED;
}


int ai_propagate(int dx, int sy, int ey, int sx, int ex, float *nav, float *mod) {

  int x, y, f, i;


  if (nav == NULL || mod == NULL)
    return FAILED;

  /* propagate the values */
  f = 0;
  while (f == 0) {
    /* loop until there is no change in the values */
    f = 1;

    /* forward */
    for (y = sy; y < ey; y++) {
      for (x = sx; x < ex; x++) {
	i = y*dx + x;
	/* left */
	if (x > sx && (nav[i-1]+1)*mod[i] < nav[i]) {
	  nav[i] = (nav[i-1]+1)*mod[i];
	  f = 0;
	}
	/* right */
	if (x < ex-1 && (nav[i+1]+1)*mod[i] < nav[i]) {
	  nav[i] = (nav[i+1]+1)*mod[i];
	  f = 0;
	}
	/* up */
	if (y > sy && (nav[i-dx]+1)*mod[i] < nav[i]) {
	  nav[i] = (nav[i-dx]+1)*mod[i];
	  f = 0;
	}
	/* down */
	if (y < ey-1 && (nav[i+dx]+1)*mod[i] < nav[i]) {
	  nav[i] = (nav[i+dx]+1)*mod[i];
	  f = 0;
	}
      }
    }

    /* backward */
    for (y = ey-1; y >= sy; y--) {
      for (x = ex-1; x >= sx; x--) {
	i = y*dx + x;
	/* left */
	if (x > sx && (nav[i-1]+1)*mod[i] < nav[i]) {
	  nav[i] = (nav[i-1]+1)*mod[i];
	  f = 0;
	}
	/* right */
	if (x < ex-1 && (nav[i+1]+1)*mod[i] < nav[i]) {
	  nav[i] = (nav[i+1]+1)*mod[i];
	  f = 0;
	}
	/* up */
	if (y > sy && (nav[i-dx]+1)*mod[i] < nav[i]) {
	  nav[i] = (nav[i-dx]+1)*mod[i];
	  f = 0;
	}
	/* down */
	if (y < ey-1 && (nav[i+dx]+1)*mod[i] < nav[i]) {
	  nav[i] = (nav[i+dx]+1)*mod[i];
	  f = 0;
	}
      }
    }
  }

  /*
  {
    FILE *mos;

    mos = fopen("tmp.txt", "ab");
    for (y = sy; y < ey; y++) {
      for (x = sx; x < ex; x++) {
	fprintf(mos, "%5d ", (int)nav[y*dx + x]);
      }
      fprintf(mos, "\n");
    }
    fclose(mos);
  }
  */

  return SUCCEEDED;
}


static int _directions_sort(const void *a, const void *b) {

  struct direction *da, *db;


  da = (struct direction *)a;
  db = (struct direction *)b;

  if (da->cost < db->cost)
    return -1;

  return 1;
}


int ai_live_rat(struct level *l, struct ai *a) {

  struct direction di[4];
  int i, ax, ay, ex, ey;
  float *f, t;


  if (l == NULL || a == NULL)
    return FAILED;

  /* navigate */
  ai_navigate_rat(l, a);

  /* state changes */
  if (a->stats.mode->mode == AI_MODE_IDLE) {
    i = rand() % 3;
    if (i == 0) {
      /* search the surroundings */
      ai_mode_add(a, AI_MODE_EXPLORE);
    }
  }

  ax = a->mx - 1;
  ex = a->mx + 1;
  ay = a->my - 1;
  ey = a->my + 1;

  if (ax < 0)
    ax = 0;
  if (ay < 0)
    ay = 0;
  if (ex >= l->dx)
    ex = l->dx-1;
  if (ey >= l->dy)
    ey = l->dy-1;

  ax = a->my*l->dx + ax;
  ex = a->my*l->dx + ex;
  ay = ay*l->dx + a->mx;
  ey = ey*l->dx + a->mx;

  /* action */
  if (a->stats.mode->mode == AI_MODE_EXPLORE) {
    f = l->navigation;
    t = f[a->my*l->dx + a->mx];
    if (t <= f[ax] && t <= f[ex] && t <= f[ay] && t <= f[ey]) {
      /* don't move, i'm already in a local minimum */
      ai_mode_remove(a);
    }
    else {
      di[0].dir = AI_DIRECTION_LEFT;
      di[0].cost = f[ax];
      di[1].dir = AI_DIRECTION_RIGHT;
      di[1].cost = f[ex];
      di[2].dir = AI_DIRECTION_UP;
      di[2].cost = f[ay];
      di[3].dir = AI_DIRECTION_DOWN;
      di[3].cost = f[ey];

      /* sort the options */
      qsort(di, 4, sizeof(struct direction), _directions_sort);

      sprintf(debug_data, "%f %f %f %f\n", di[0].cost, di[1].cost, di[2].cost, di[3].cost);
      debug_info = 1;

      /* choose one from the best options */
      for (i = 1; i < 4; i++) {
	if (di[i].cost > di[0].cost)
	  break;
      }

      i = rand() % i;
      ai_move(l, a, di[i].dir);
    }
  }
  else if (a->stats.mode->mode == AI_MODE_HUNGRY) {
    f = l->navigation;
    t = f[a->my*l->dx + a->mx];
    if (t <= f[ax] && t <= f[ex] && t <= f[ay] && t <= f[ey]) {
      /* don't move, i'm already in a local minimum */
      ai_mode_remove(a);
    }
    else {
      di[0].dir = AI_DIRECTION_LEFT;
      di[0].cost = f[ax];
      di[1].dir = AI_DIRECTION_RIGHT;
      di[1].cost = f[ex];
      di[2].dir = AI_DIRECTION_UP;
      di[2].cost = f[ay];
      di[3].dir = AI_DIRECTION_DOWN;
      di[3].cost = f[ey];

      /* sort the options */
      qsort(di, 4, sizeof(struct direction), _directions_sort);

      sprintf(debug_data, "%f %f %f %f\n", di[0].cost, di[1].cost, di[2].cost, di[3].cost);
      debug_info = 1;

      /* choose one from the best options */
      for (i = 1; i < 4; i++) {
	if (di[i].cost > di[0].cost)
	  break;
      }

      i = rand() % i;
      ai_move(l, a, di[i].dir);
    }
  }

  return SUCCEEDED;
}


int ai_live(struct level *l) {

  struct ai *a;


  if (l == NULL)
    return FAILED;

  a = l->bots_list;
  while (a != NULL) {
    /* is this the ai's first visit to the current level? */
    if (a->maps[l->id] == NULL) {
      if (map_new(l->dx, l->dy, &a->maps[l->id]) == FAILED)
	continue;
    }

    map_look(l, a->maps[l->id], a, 6);

    /* skip the player */
    if (a->control == AI_CONTROL_PLAYER) {
      a = a->next;
      continue;
    }

    if (a->stats.nutrition <= 0) {
      /* ai dies from hunger */
      ai_dies(l, a);
      a = a->next;
      continue;
    }

    /* let the bot live for one cycle */
    if (a->mem.self->race == AI_RACE_HUMAN)
      ai_live_human(l, a);
    else if (a->mem.self->race == AI_RACE_ORC)
      ai_live_orc(l, a);
    else if (a->mem.self->race == AI_RACE_CAT)
      ai_live_cat(l, a);
    else if (a->mem.self->race == AI_RACE_RAT)
      ai_live_rat(l, a);

    a = a->next;
  }

  return SUCCEEDED;
}


/* add the new mode to the ai's mode stack */
int ai_mode_add(struct ai *a, int mode) {

  struct mode *m;


  if (a == NULL)
    return FAILED;

  m = calloc(sizeof(struct mode), 1);
  if (m == NULL) {
    fprintf(stderr, "AI_MODE_ADD: Out of memory error.\n");
    return FAILED;
  }

  m->mode = mode;
  m->next = a->stats.mode;
  m->prev = NULL;
  if (a->stats.mode != NULL)
    a->stats.mode->prev = m;
  a->stats.mode = m;

  return SUCCEEDED;
}


int ai_mode_remove(struct ai *a) {

  struct mode *m;


  if (a == NULL)
    return FAILED;

  /* don't remove IDLE mode as it's the bottommost mode in the mode stack */
  if (a->stats.mode->mode == AI_MODE_IDLE)
    return FAILED;

  m = a->stats.mode->next;
  free(a->stats.mode);
  a->stats.mode = m;
  if (m != NULL)
    m->prev = NULL;

  return SUCCEEDED;
}


int ai_add(struct level *l, int race, int x, int y, struct ai **out) {

  struct person *p, tmp;
  static int id = 0;
  struct ai *a;
  int i;


  if (l == NULL)
    return FAILED;

  if (x < 0 || y < 0 || x >= l->dx || y >= l->dy)
    return FAILED;

  if (l->bots[y*l->dx + x] != NULL)
    return FAILED;

  a = calloc(sizeof(struct ai), 1);
  if (a == NULL) {
    fprintf(stderr, "AI_ADD: Out of memory error.\n");
    return FAILED;
  }

  a->mx = x;
  a->my = y;
  a->control = AI_CONTROL_COMPUTER;
  a->items = NULL;

  /* clear the level memory */
  for (i = 0; i < 256; i++)
    a->maps[i] = NULL;

  /* all creatures are born with their stomachs full */
  a->stats.nutrition = 1.0;

  /* install hp */
  a->stats.hp_max = 10;
  a->stats.hp_current = a->stats.hp_max;

  /* init mode */
  a->stats.mode = NULL;
  ai_mode_add(a, AI_MODE_IDLE);

  /* clear the memory */
  a->mem.person_list = NULL;
  a->mem.rumor_list = NULL;
  a->mem.old_rumors_list = NULL;
  a->mem.experience_list = NULL;

  /* make the creature know itself */
  tmp.id = id;
  tmp.race = race;

  person_create_and_add(&(a->mem.person_list), &tmp, &p);
  a->mem.self = p;
  p->respect = 1.0;

  l->bots[y*l->dx + x] = a;

  a->next = l->bots_list;
  a->prev = NULL;
  if (l->bots_list != NULL)
    l->bots_list->prev = a;
  l->bots_list = a;

  if (out != NULL)
    *out = a;

  id++;

  return SUCCEEDED;
}


int ai_pick(struct level *l, struct ai *a) {

  struct item *i;


  if (l == NULL || a == NULL)
    return FAILED;

  i = l->items[a->my*l->dx + a->mx];
  if (i == NULL)
    return FAILED;

  l->items[a->my*l->dx + a->mx] = NULL;

  i->next = a->items;
  i->prev = NULL;
  if (a->items != NULL)
    a->items->prev = i;
  a->items = i;

  return SUCCEEDED;
}


static int _ai_compute_position(struct level *l, struct ai *a, int dir, int *x, int *y) {

  int mx, my;


  if (l == NULL || a == NULL || x == NULL || y == NULL)
    return FAILED;

  mx = a->mx;
  my = a->my;

  if (dir == AI_DIRECTION_UP)
    my--;
  else if (dir == AI_DIRECTION_DOWN)
    my++;
  else if (dir == AI_DIRECTION_LEFT)
    mx--;
  else if (dir == AI_DIRECTION_RIGHT)
    mx++;

  if (mx < 0 || my < 0 || mx >= l->dx || my >= l->dy)
    return FAILED;

  *x = mx;
  *y = my;

  return SUCCEEDED;
}


int ai_operate(struct level *l, struct ai *a, int dir) {

  unsigned char c;
  struct item *i;
  int x, y;


  if (l == NULL || a == NULL)
    return FAILED;

  if (_ai_compute_position(l, a, dir, &x, &y) == FAILED)
    return FAILED;

  /* get the destination map piece */
  c = l->tiles[y*l->dx + x];

  /* CLOSED DOOR? */
  if (c == '+') {
    /* do we have key to it? */
    i = a->items;
    while (i != NULL) {
      if (i->type == ITEM_TYPE_KEY && i->a == x && i->b == y)
	break;
      i = i->next;
    }

    /* no key? */
    if (i == NULL)
      return FAILED;

    /* we have the key to it! */
    l->tiles[y*l->dx + x] = '\'';
  }
  /* OPEN DOOR? */
  else if (c == '\'') {
    /* close it */
    l->tiles[y*l->dx + x] = '+';

    /* do we have key to it? */
    i = a->items;
    while (i != NULL) {
      if (i->type == ITEM_TYPE_KEY && i->a == x && i->b == y)
	break;
      i = i->next;
    }

    /* we have the key to it! */
    if (i != NULL) {
      /* lock it? */
    }
  }

  return SUCCEEDED;
}


int ai_move(struct level *l, struct ai *a, int dir) {

  unsigned char c;
  int x, y;


  if (l == NULL || a == NULL)
    return FAILED;

  if (_ai_compute_position(l, a, dir, &x, &y) == FAILED)
    return FAILED;

  /* get the destination map piece */
  c = l->tiles[y*l->dx + x];

  /* find out if we can step on it */
  if (!(c == '.' || c == ' ' || c == '<' || c == '>' || c == '\''))
    return FAILED;

  /* we cannot step on another bot */
  if (l->bots[y*l->dx + x] != NULL)
    return FAILED;

  /* clear old position */
  l->bots[a->my*l->dx + a->mx] = NULL;

  /* move to the destination */
  a->mx = x;
  a->my = y;

  l->bots[a->my*l->dx + a->mx] = a;

  return SUCCEEDED;
}
