
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "defines.h"
#include "action_memory.h"



int action_memory_list(struct action_memory *list) {

  while (list != NULL) {
    printf("********************************\n");
    printf("SUBJECT: %d OBJECT: %d ACTION: %d\n", list->subject, list->object, list->action);
    printf("RESS.HP: %f\n", list->ress.hp);
    printf("********************************\n");
    list = list->next;
  }

  return SUCCEEDED;
}


int action_memory_add(struct action_memory **list, struct action_memory *new) {

  struct action_memory *e, **l;
  float a, b;


  if (list == NULL || new == NULL)
    return FAILED;

  l = list;

  /* find out if we've done this before */
  while (*l != NULL) {
    if ((*l)->subject == new->subject && (*l)->object == new->object && (*l)->action == new->action)
      break;
    l = &((*l)->next);
  }

  /* no previous action memory? */
  if (*l == NULL) {
    if (action_memory_clone(new, &e) == FAILED)
      return FAILED;

    e->count = 1;
    e->next = *list;
    *list = e;

    return SUCCEEDED;
  }

  /* merge the action memories (update averages) */
  a = ((float)(*l)->count) / ((float)(*l)->count + 1);
  b = 1.0 - a;

  (*l)->ress.hp = (*l)->ress.hp*a + new->ress.hp*b;
  (*l)->ress.nutrition = (*l)->ress.nutrition*a + new->ress.nutrition*b;
  (*l)->ress.pleasure = (*l)->ress.pleasure*a + new->ress.pleasure*b;
  (*l)->ress.gold = (*l)->ress.gold*a + new->ress.gold*b;
  (*l)->ress.items = (*l)->ress.items*a + new->ress.items*b;
  (*l)->ress.karma = (*l)->ress.karma*a + new->ress.karma*b;
  (*l)->ress.experience = (*l)->ress.experience*a + new->ress.experience*b;

  (*l)->reso.hp = (*l)->reso.hp*a + new->reso.hp*b;
  (*l)->reso.nutrition = (*l)->reso.nutrition*a + new->reso.nutrition*b;
  (*l)->reso.pleasure = (*l)->reso.pleasure*a + new->reso.pleasure*b;
  (*l)->reso.gold = (*l)->reso.gold*a + new->reso.gold*b;
  (*l)->reso.items = (*l)->reso.items*a + new->reso.items*b;
  (*l)->reso.karma = (*l)->reso.karma*a + new->reso.karma*b;
  (*l)->reso.experience = (*l)->reso.experience*a + new->reso.experience*b;

  (*l)->count++;

  return SUCCEEDED;
}


int action_memory_clone(struct action_memory *new, struct action_memory **clone) {

  struct action_memory *e;


  if (new == NULL || clone == NULL)
    return FAILED;

  e = malloc(sizeof(struct action_memory));
  if (e == NULL) {
    fprintf(stderr, "ACTION_MEMORY_CLONE: Out of memory error.\n");
    *clone = NULL;
    return FAILED;
  }

  e->subject = new->subject;
  e->object = new->object;
  e->action = new->action;
  e->count = new->count;
  e->transformed = new->transformed;

  e->ress.hp = new->ress.hp;
  e->ress.nutrition = new->ress.nutrition;
  e->ress.pleasure = new->ress.pleasure;
  e->ress.gold = new->ress.gold;
  e->ress.items = new->ress.items;
  e->ress.karma = new->ress.karma;
  e->ress.experience = new->ress.experience;

  e->reso.hp = new->reso.hp;
  e->reso.nutrition = new->reso.nutrition;
  e->reso.pleasure = new->reso.pleasure;
  e->reso.gold = new->reso.gold;
  e->reso.items = new->reso.items;
  e->reso.karma = new->reso.karma;
  e->reso.experience = new->reso.experience;

  *clone = e;

  return SUCCEEDED;
}


int action_memory_create(struct action_memory **out) {

  struct action_memory *e;


  e = malloc(sizeof(struct action_memory));
  if (e == NULL) {
    fprintf(stderr, "ACTION_MEMORY_CREATE: Out of memory error.\n");
    *out = NULL;
    return FAILED;
  }

  e->subject = 0;
  e->object = 0;
  e->action = 0;
  e->count = 0;
  e->transformed = 0;

  e->ress.hp = 0;
  e->ress.nutrition = 0;
  e->ress.pleasure = 0;
  e->ress.gold = 0;
  e->ress.items = 0;
  e->ress.karma = 0;
  e->ress.experience = 0;

  e->reso.hp = 0;
  e->reso.nutrition = 0;
  e->reso.pleasure = 0;
  e->reso.gold = 0;
  e->reso.items = 0;
  e->reso.karma = 0;
  e->reso.experience = 0;

  *out = e;

  return SUCCEEDED;
}
