
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "defines.h"
#include "person.h"



/* compute the initial respect for race r based on person list l */
float person_compute_initial_respect(struct person *l, struct person *p) {

  float a;
  int i;


  /* compute the initial respect by averaging */
  a = 0;
  i = 0;
  while (l != NULL) {
    if (l->race == p->race) {
      a += l->respect;
      i++;
    }
    l = l->next;
  }

  /* the race of p is unknown -> use the default initial respect */
  if (i == 0)
    return PERSON_INITIAL_RESPECT;

  a /= i;

  /* limit the initial respect */
  if (a > PERSON_INITIAL_RESPECT)
    a = PERSON_INITIAL_RESPECT;

  return a;
}


/* create a new person and add it to the list l */
int person_create_and_add(struct person **l, struct person *p, struct person **out) {

  struct person *n;


  n = person_create();
  if (n == NULL)
    return FAILED;

  n->respect = person_compute_initial_respect(*l, p);
  n->id = p->id;
  n->race = p->race;
  n->next = *l;
  *l = n;

  if (out != NULL)
    *out = n;

  return SUCCEEDED;
}


/* create a new person */
struct person *person_create(void) {

  struct person *p;


  p = calloc(sizeof(struct person), 1);
  if (p == NULL) {
    fprintf(stderr, "PERSON_CREATE: Out of memory error.\n");
    return NULL;
  }

  return p;
}


/* check if person p is in the list l */
int person_check(struct person **l, struct person *p) {

  struct person *n;


  /* do we know p already? */
  n = *l;
  while (n != NULL) {
    if (n->id == p->id)
      return SUCCEEDED;
    n = n->next;
  }

  /* no, add p to the list */
  person_create_and_add(l, p, NULL);

  return SUCCEEDED;
}


/* return person p from the list l */
struct person *person_get(struct person *l, struct person *p) {

  while (l != NULL) {
    if (l->id == p->id)
      return l;
    l = l->next;
  }

  return NULL;
}
