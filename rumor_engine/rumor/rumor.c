
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "defines.h"
#include "person.h"
#include "rumor.h"

/*
  #define RUMOR_DEBUG 1
*/

/* the action table */
static struct action *actions = NULL;


#ifdef RUMOR_DEBUG
static void _rumor_debug_print_rumor(FILE *f, struct rumor *r) {

  fprintf(f, "PRI: %f SUB: %d OBJ: %d ACT: %d TEL: %d ORG: %d TIM: %d STA: %d X: %f Y: %f esu: %f eob: %f ete: %f eor: %f ID: %d\n",
          r->priority, r->subject.id, r->object.id, r->action, r->teller.id, r->original_teller.id, r->timer,
          r->status, r->x, r->y, r->effect_subject, r->effect_object, r->effect_teller, r->effect_original_teller, r->id);
}
#endif


/* a sigmoid function */
static float _sigm(float x) {

  return 1.0/(1.0 + exp(-x));
}


int rumor_init(struct action *a) {

  if (a == NULL)
    return FAILED;

  actions = a;

  return SUCCEEDED;
}


int rumor_add_with_checks(struct rumor **l, struct person **p, struct rumor *r) {

  struct action *a;


  /* have we heard it before? */
  if (rumor_find_duplicate(*l, r) != FAILED)
    return SUCCEEDED;

  a = &actions[r->action];

  /* it's a new rumor! check the persons in r, and memorize r */
  if (a->status == ACTION_STATUS_OTHER) {
    person_check(p, &(r->subject));
    person_check(p, &(r->object));
  }
  else if (a->status == ACTION_STATUS_DESCRIPTION) {
    person_check(p, &(r->subject));
  }
  person_check(p, &(r->teller));
  person_check(p, &(r->original_teller));

  rumor_add(l, r);

  return SUCCEEDED;
}


/* experience r */
int rumor_experience(struct rumor **l, struct person **p, struct rumor *r) {

  struct action *a;
  struct person *pt, *po, *ps, *pb;
  float su, ob, te, or, e;


  r->effect_teller = 0;
  r->effect_original_teller = 0;
  r->effect_subject = 0;
  r->effect_object = 0;

  /* don't experience negated rumors */
  if ((r->status & RUMOR_STATUS_FALSE) != 0)
    return SUCCEEDED;

  a = &actions[r->action];
  if (a->status == ACTION_STATUS_OTHER) {
    /* subject and object are valid creatures */
    person_check(p, &(r->subject));
    person_check(p, &(r->object));
    person_check(p, &(r->teller));
    person_check(p, &(r->original_teller));

    ps = person_get(*p, &(r->subject));
    pb = person_get(*p, &(r->object));
    pt = person_get(*p, &(r->teller));
    po = person_get(*p, &(r->original_teller));

    if ((r->status & RUMOR_STATUS_HEARD) != 0)
      e = a->hear * a->severity;
    else
      e = a->see * a->severity;

    te = e * a->teller;
    or = e * a->original_teller;
    su = e * a->subject * (pb->respect - 0.5) * _sigm((ps->respect - 0.5) * 6);
    ob = e * a->object * (ps->respect - 0.5) * _sigm((pb->respect - 0.5) * 6);

    rumor_change_respect(l, pt, te);
    rumor_change_respect(l, po, or);
    rumor_change_respect(l, ps, su);
    rumor_change_respect(l, pb, ob);

    r->effect_teller = te;
    r->effect_original_teller = or;
    r->effect_subject = su;
    r->effect_object = ob;
  }
  else if (a->status == ACTION_STATUS_DESCRIPTION) {
    /* subject is a valid creature */
    person_check(p, &(r->subject));
    person_check(p, &(r->teller));
    person_check(p, &(r->original_teller));

    /* descriptive rumors don't affect the relationships */
    r->effect_teller = 0;
    r->effect_original_teller = 0;
    r->effect_subject = 0;
    r->effect_object = 0;
  }
  else if (a->status == ACTION_STATUS_IS) {
    /* the rumor is about a place/thing, subject and object are not creatures */
    person_check(p, &(r->teller));
    person_check(p, &(r->original_teller));

    pt = person_get(*p, &(r->teller));
    po = person_get(*p, &(r->original_teller));

    if ((r->status & RUMOR_STATUS_HEARD) != 0)
      e = a->hear * a->severity;
    else
      e = a->see * a->severity;

    te = e * a->teller;
    or = e * a->original_teller;

    rumor_change_respect(l, pt, te);
    rumor_change_respect(l, po, or);

    r->effect_teller = te;
    r->effect_original_teller = or;
    r->effect_subject = 0;
    r->effect_object = 0;
  }

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "RUMOR_EXPERIENCE:\n");
    _rumor_debug_print_rumor(g, r);
    fprintf(g, "subject: %f object: %f teller: %f original_teller: %f\n", su, ob, te, or);
    fflush(g);
    fclose(g);
  }
#endif

  return SUCCEEDED;
}


/* select a rumor a wants to tell b */
struct rumor *rumor_select(struct memories *a, struct memories *b) {

  struct rumor *r, *e;
  int i;


  /* take a rumor from rumor list or experience list */
  i = rand() % 2;

  /* get the rumor from experience list */
  if (i == 0)
    e = a->experience_list;
  /* get the rumor from rumor list */
  else
    e = a->rumor_list;

  if (e == NULL)
    return NULL;

  i = 0;
  r = e;
  while (r != NULL) {
    i++;
    r = r->next;
  }

  /* get the rumor o c wants to give to r */
  i = rand() % i;
  r = e;
  while (r != NULL && i > 0) {
    i--;
    r = r->next;
  }

  /* don't tell b what b has said to a */
  if (r->teller.id == b->self->id)
    return NULL;

  return r;
}


/* c tells a rumor to r */
int rumor_tell(struct memories *c, struct memories *r) {

  struct rumor *o, *e, **t;
  struct action *a;
  struct person *p;
  float f, v;
  int i, n;


  /* the persons introduce each other */
  person_check(&r->person_list, c->self);
  person_check(&c->person_list, r->self);

  /* find out what r thinks of c */
  p = person_get(r->person_list, c->self);

  /* will r listen to c? */
  if (p->respect < 0.5) {
    return SUCCEEDED;
  }

  /* select one rumor c wants to tell to r */
  o = rumor_select(c, r);
  if (o == NULL)
    return FAILED;

  o->tell_count++;

  /* can r believe c's rumor o? */
  a = &actions[o->action];
  if (a->status == ACTION_STATUS_OTHER) {
    person_check(&r->person_list, &(o->subject));
    person_check(&r->person_list, &(o->object));
  }
  else if (a->status == ACTION_STATUS_DESCRIPTION) {
    person_check(&r->person_list, &(o->subject));
  }
  person_check(&r->person_list, &(o->original_teller));

  i = o->teller.id;
  n = o->teller.race;
  o->teller.id = c->self->id;
  o->teller.race = c->self->race;
  f = rumor_compute_trust(r->person_list, o);
  o->teller.id = i;
  o->teller.race = n;

  /* is the rumor o believable? */
  if (f < 0.5)
    return FAILED;

  /* now check if we should believe the rumor or not */
  v = rand() / (double)RAND_MAX;
  if (v < f)
    return FAILED;

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "\nRUMOR_SPREAD: %d hears a rumor.\n", r->self->id);
    fprintf(g, "NEW:\n");
    _rumor_debug_print_rumor(g, o);
    fflush(g);
    fclose(g);
  }
#endif

  /* has r already heard it (and acted upon it)? */
  e = rumor_clone(o);
  e->status = RUMOR_STATUS_HEARD | (o->status & RUMOR_STATUS_TRUE) | (o->status & RUMOR_STATUS_FALSE);
  e->teller.id = c->self->id;
  if (rumor_find_clone(r->old_rumors_list, e) == SUCCEEDED) {
    rumor_free(e);
    return FAILED;
  }

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "i haven't heard it before\n");
    fflush(g);
    fclose(g);
  }
#endif

  /* is the rumor about r itself? */
  if (a->status == ACTION_STATUS_OTHER) {
    if (e->subject.id == r->self->id || e->object.id == r->self->id || e->original_teller.id == r->self->id) {
      rumor_handle_personal_rumor(r, c, e);
      return SUCCEEDED;
    }
  }
  else if (a->status == ACTION_STATUS_DESCRIPTION || a->status == ACTION_STATUS_IS) {
    if (e->original_teller.id == r->self->id) {
      rumor_handle_personal_rumor(r, c, e);
      return SUCCEEDED;
    }
  }

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "it's not about myself\n");
    fflush(g);
    fclose(g);
  }
#endif

  /* does r know the rumor already? */
  t = &r->rumor_list;
  i = rumor_find_duplicate(*t, e);
  if (i == FAILED) {
    t = &r->experience_list;
    i = rumor_find_duplicate(*t, e);
    if (i == FAILED) {
      t = &r->old_rumors_list;
      i = rumor_find_duplicate(*t, e);
    }
  }

  if (i != FAILED) {
    if (i == RUMOR_CLASH)
      rumor_handle_clash(t, &r->person_list, e);
    else if (i == SUCCEEDED)
      rumor_handle_duplicate(&r->rumor_list, t, r->person_list, e, r->self->id);
    return SUCCEEDED;
  }

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "it's a totally new thing\n");
    fflush(g);
    fclose(g);
  }
#endif

  /* remember this rumor */
  e->timer = 300;
  rumor_add(&r->old_rumors_list, e);

  /* create a new rumor */
  e = rumor_clone(o);
  e->status = RUMOR_STATUS_HEARD | (o->status & RUMOR_STATUS_TRUE) | (o->status & RUMOR_STATUS_FALSE);
  e->teller.id = c->self->id;
  e->teller.race = c->self->race;

  /* experience the rumor! */
  rumor_experience(&r->rumor_list, &r->person_list, e);
  rumor_add(&r->rumor_list, e);

  return SUCCEEDED;
}


/* find if there is a duplicate of rumor r in the list l */
int rumor_find_duplicate(struct rumor *l, struct rumor *r) {

  struct rumor *e;


  e = rumor_get_duplicate(l, r);

  if (e == NULL)
    return FAILED;
  if ((e->status & RUMOR_STATUS_TRUE) != (r->status & RUMOR_STATUS_TRUE))
    return RUMOR_CLASH;

  /* refresh the memory of the rumor */
  e->timer = 100;

  return SUCCEEDED;
}


/* get a duplicate of r in rumor list l */
struct rumor *rumor_get_duplicate(struct rumor *l, struct rumor *r) {

  while (l != NULL) {
    if (l->subject.id == r->subject.id && l->object.id == r->object.id && l->action == r->action && l->x == r->x && l->y == r->y)
      return l;
    l = l->next;
  }

  return NULL;
}


/* forget a little of the rumors in the list l */
int rumor_decay(struct rumor **l) {

  struct rumor *rp, *rc, *rt;


  rp = NULL;
  rc = *l;
  while (rc != NULL) {
    /* CHANGE THIS TO TAKE THE INTELLIGENCE INTO ACCOUNT */
    /* decay only unimportant rumors */
    if (rc->priority >= 0.5) {
      rp = rc;
      rc = rc->next;
      continue;
    }
    rc->timer--;
    /* forget the rumor? */
    if (rc->timer == 0) {
      if (rp == NULL) {
        *l = rc->next;
      }
      else {
        rp->next = rc->next;
      }
      rt = rc->next;
      rumor_free(rc);
      rc = rt;
    }
    else {
      rp = rc;
      rc = rc->next;
    }
  }

  return SUCCEEDED;
}


/* filter away rumors told by id in the list l */
int rumor_filter(struct rumor **l, int id) {

  struct rumor *rp, *rc, *rt;


  rp = NULL;
  rc = *l;
  while (rc != NULL) {
    /* forget the rumor? */
    if (rc->teller.id == id) {
      if (rp == NULL) {
        *l = rc->next;
      }
      else {
        rp->next = rc->next;
      }
      rt = rc->next;
      rumor_free(rc);
      rc = rt;
    }
    else {
      rp = rc;
      rc = rc->next;
    }
  }

  return SUCCEEDED;
}


/* add rumor to list l */
int rumor_add(struct rumor **l, struct rumor *r) {

  r->next = *l;
  *l = r;

  return SUCCEEDED;
}


/* remove rumor r from the list l */
int rumor_remove(struct rumor **l, struct rumor *r) {

  struct rumor *rp, *rc, *rt;


  rp = NULL;
  rc = *l;
  while (rc != NULL) {
    /* forget the rumor? */
    if (rc == r) {
      if (rp == NULL)
        *l = rc->next;
      else
        rp->next = rc->next;
      rt = rc->next;
      rumor_free(rc);
      rc = rt;

      return SUCCEEDED;
    }
    else {
      rp = rc;
      rc = rc->next;
    }
  }

  return FAILED;
}


/* remove rumors that are like rumor r (only the teller may differ) in the list l */
int rumor_remove_all(struct rumor **l, struct rumor *r) {

  struct rumor *rp, *rc, *rt;


  rp = NULL;
  rc = *l;
  while (rc != NULL) {
    /* forget the rumor? */
    if (rc->x == r->x && rc->y && r->y && rc->subject.id == r->subject.id && rc->object.id == r->object.id &&
        rc->action == r->action) {
      if (rp == NULL)
        *l = rc->next;
      else
        rp->next = rc->next;
      rt = rc->next;
      rumor_free(rc);
      rc = rt;
    }
    else {
      rp = rc;
      rc = rc->next;
    }
  }

  return SUCCEEDED;
}


/* change the respect of p by r, and perhaps filter away rumors told
   by him in the list l */
int rumor_change_respect(struct rumor **l, struct person *p, float r) {

  float o;


  o = p->respect;
  p->respect += r;
  if (p->respect > 1)
    p->respect = 1;
  else if (p->respect < 0)
    p->respect = 0;

  /* if the respect dropped below 0.5 then filter away all the
     rumors told by him */
  if (o >= 0.5 && p->respect < 0.5)
    rumor_filter(l, p->id);

  return SUCCEEDED;
}


/* create a rumor that says that the rumor r is false */
struct rumor *rumor_create_negated_rumor(struct rumor *r) {

  struct rumor *f;


  f = rumor_clone(r);
  if (f == NULL)
    return f;

  f->timer = (int)(r->priority * 100);

  if ((r->status & RUMOR_STATUS_FALSE) != 0)
    f->status = RUMOR_STATUS_TRUE;
  else
    f->status = RUMOR_STATUS_FALSE;

  return f;
}


/* create a new rumor */
struct rumor *rumor_create(void) {

  struct rumor *r;
  static int id = 0;


#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "RUMOR_CREATE:\n");
    fprintf(g, "creating rumor %d\n", id);
    fflush(g);
    fclose(g);
  }
#endif

  /* allocate, and set all fields to zero */
  r = calloc(sizeof(struct rumor), 1);
  if (r == NULL) {
    fprintf(stderr, "RUMOR_CREATE: Out of memory error.\n");
    return NULL;
  }

  r->id = id++;

  return r;
}


/* destroy a rumor */
int rumor_free(struct rumor *r) {

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "RUMOR_FREE:\n");
    fprintf(g, "freeing rumor %d\n", r->id);
    fflush(g);
    fclose(g);
  }
#endif

  if (r != NULL)
    free(r);

  return SUCCEEDED;
}


/* a found a lie r */
int rumor_handle_lie(struct memories *a, struct rumor *r) {

  struct person *e;
  struct rumor *f;


  /* was it a's own bad? just forget it if a hasn't told it to anybody */
  if (r->teller.id == a->self->id && r->tell_count == 0) {
    /* delete the original rumor/experience as it's not true! */
    rumor_remove(&a->rumor_list, r);
    rumor_remove(&a->experience_list, r);

    return SUCCEEDED;
  }

  /* create a negation of r, to clear things up */
  f = rumor_create_negated_rumor(r);
  f->teller.id = a->self->id;
  f->teller.race = a->self->race;
  f->original_teller.id = a->self->id;
  f->original_teller.race = a->self->race;
  f->timer = 300;
  rumor_add(&a->rumor_list, f);

  /* was it a's own bad!?!? */
  if (a->self->id == r->teller.id) {
    f->status &= ~RUMOR_STATUS_HEARD;
    f->status |= RUMOR_STATUS_SAW;
  }
  /* no, someone else lied */
  else {
    f->status |= RUMOR_STATUS_HEARD;
    f->status &= ~RUMOR_STATUS_SAW;

    /* downgrade respect for the liar */
    e = person_get(a->person_list, &(r->original_teller));
    rumor_change_respect(&a->rumor_list, e, -0.2);
  }

  /* delete the original rumor/experience as it's not true! */
  rumor_remove(&a->rumor_list, r);
  rumor_remove(&a->experience_list, r);

  return SUCCEEDED;
}


/* handle a situation where a person hears two conflicting rumors */
int rumor_handle_clash(struct rumor **l, struct person **p, struct rumor *r) {

  struct rumor *e;
  float a, b;


#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "RUMOR_HANDLE_CLASH:\n");
    fflush(g);
    fclose(g);
  }
#endif

  e = rumor_get_duplicate(*l, r);
  a = rumor_compute_trust(*p, e);
  b = rumor_compute_trust(*p, r);

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "NEW:\n");
    _rumor_debug_print_rumor(g, r);
    fprintf(g, "DUPLICATE:\n");
    _rumor_debug_print_rumor(g, r);
    fflush(g);
    fclose(g);
  }
#endif

  /* which one is more believable, e or r? */
  if (a >= b) {
    /* the old rumor e is better */
    rumor_free(r);
    return SUCCEEDED;
  }

  /* was the old one a true statement (one with an effect)? */
  if ((e->status & RUMOR_STATUS_TRUE) != 0) {
    /* remove the effect */
    rumor_neutralize_effect(l, p, e);
  }
  /* the old one was a negation so just skip it and experience the new one */
  else {
    rumor_experience(l, p, r);
  }

  /* forget the old rumor and remember the new one */
  rumor_remove(l, e);
  rumor_add(l, r);

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "the new rumor won\n");
    fflush(g);
    fclose(g);
  }
#endif

  return SUCCEEDED;
}


/* neutralize the effect of r */
int rumor_neutralize_effect(struct rumor **l, struct person **p, struct rumor *r) {

  struct action *a;
  struct person *x;


  a = &actions[r->action];

  /* descriptive rumors have no effect */
  if (a->status == ACTION_STATUS_DESCRIPTION)
    return SUCCEEDED;

  x = person_get(*p, &(r->teller));
  rumor_change_respect(l, x, -r->effect_teller);
  x = person_get(*p, &(r->original_teller));
  rumor_change_respect(l, x, -r->effect_original_teller);

  if (a->status == ACTION_STATUS_OTHER) {
    x = person_get(*p, &(r->subject));
    rumor_change_respect(l, x, -r->effect_subject);
    x = person_get(*p, &(r->object));
    rumor_change_respect(l, x, -r->effect_object);
  }

  return SUCCEEDED;
}


/* get the value describing this rumor's goodness */
float rumor_compute_trust(struct person *p, struct rumor *r) {

  struct person *e;
  float a, b;


  e = person_get(p, &(r->original_teller));
  a = e->respect;
  e = person_get(p, &(r->teller));
  b = e->respect;

  /* return the smaller of these */
  if (a < b)
    return a;

  return b;
}


/* create an experience rumor */
struct rumor *rumor_create_experience(struct person *p) {

  struct rumor *r;


  /* allocate a new rumor (sets all the fields to zero) */
  r = rumor_create();
  if (r == NULL)
    return r;

  r->status = RUMOR_STATUS_SAW | RUMOR_STATUS_TRUE;
  r->teller.id = p->id;
  r->teller.race = p->race;
  r->original_teller.id = p->id;
  r->original_teller.race = p->race;

  return r;
}


/* clone rumor r */
struct rumor *rumor_clone(struct rumor *r) {

  struct rumor *e;


  e = rumor_create();
  if (e == NULL)
    return e;

  memcpy(e, r, sizeof(struct rumor));

  return e;
}


/* find if there is a clone of r in list l */
int rumor_find_clone(struct rumor *l, struct rumor *r) {

  while (l != NULL) {
    if (l->subject.id == r->subject.id && l->object.id == r->object.id &&
        l->action == r->action && l->teller.id == r->teller.id &&
        l->original_teller.id == r->original_teller.id && l->status == r->status &&
        l->x == r->x && l->y == r->y) {
      return SUCCEEDED;
    }
    l = l->next;
  }

  return FAILED;
}


/* a heard rumor r from b and a is mentioned in it - handle the situation */
int rumor_handle_personal_rumor(struct memories *a, struct memories *b, struct rumor *r) {

  struct rumor **t;
  int i;


#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "RUMOR_HANDLE_PERSONAL_RUMOR: %d hears a personal rumor.\n", a->self->id);
    fflush(g);
    fclose(g);
  }
#endif

  /* first lets check if a knows about r already */
  t = &a->experience_list;
  i = rumor_find_duplicate(*t, r);

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "NEW:\n");
    _rumor_debug_print_rumor(g, r);
    fprintf(g, "DUPLICATE STATUS: ");
    if (i == FAILED)
      fprintf(g, "FAILED\n");
    if (i == SUCCEEDED)
      fprintf(g, "SUCCEEDED\n");
    if (i == RUMOR_CLASH)
      fprintf(g, "RUMOR_CLASH\n");
    fflush(g);
    fclose(g);
  }
#endif

  /* a has no knowledge of r */
  if (i == FAILED) {
    /* only if the rumor is not negated */
    if ((r->status & RUMOR_STATUS_TRUE) != 0)
      rumor_tell_its_not_true(a, b, r);
    return SUCCEEDED;
  }
  /* a knows r's negation! */
  else if (i == RUMOR_CLASH) {
    rumor_handle_clash(t, &a->person_list, r);
    return SUCCEEDED;
  }

  /* a knows r */
  rumor_handle_duplicate(&a->rumor_list, t, a->person_list, r, a->self->id);

  return SUCCEEDED;
}


/* a tells b that the rumor r is not true */
int rumor_tell_its_not_true(struct memories *a, struct memories *b, struct rumor *r) {

  struct person *p;
  struct rumor **t, *f, *c;
  float m, n;


#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "RUMOR_TELL_ITS_NOT_TRUE: %d is telling %d it's not true.\n", a->self->id, b->self->id);
    fprintf(g, "THE LIE:\n");
    _rumor_debug_print_rumor(g, r);
    fflush(g);
    fclose(g);
  }
#endif

  t = &b->rumor_list;
  /* rumors */
  f = rumor_get_duplicate(*t, r);
  if (f == NULL) {
    t = &b->experience_list;
    /* experiences */
    f = rumor_get_duplicate(*t, r);
  }

  p = person_get(b->person_list, &(f->teller));
  n = p->respect;
  p = person_get(b->person_list, a->self);
  m = p->respect;

  /* who is more respectable, a or the one who told the old story? */
  if (n >= m) {
    /* the other one */
    return SUCCEEDED;
  }

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "%d believes %d.\n", b->self->id, a->self->id);
    fflush(g);
    fclose(g);
  }
#endif

  /* a is -> create a negation of r, to clear things up */
  c = rumor_create_negated_rumor(r);
  c->teller.id = a->self->id;
  c->teller.race = a->self->race;
  c->original_teller.id = a->self->id;
  c->original_teller.race = a->self->race;
  c->timer = 300;
  c->status &= ~RUMOR_STATUS_SAW;
  c->status |= RUMOR_STATUS_HEARD;

  /* remove the false rumor (and its effect) */
  if ((f->status & RUMOR_STATUS_TRUE) != 0)
    rumor_neutralize_effect(&b->rumor_list, &b->person_list, f);

  /* b experiences the negation of r */
  rumor_experience(&b->rumor_list, &b->person_list, c);
  rumor_add(&b->rumor_list, c);

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "ROLLBACK: te: %f or: %f su: %f ob: %f\n", -f->effect_teller, -f->effect_original_teller, -f->effect_subject, -f->effect_object);
    fflush(g);
    fclose(g);
  }
#endif

  rumor_remove(t, f);

  return SUCCEEDED;
}


/* handle the situation when there is a duplicate of r in l (possibly different tellers) */
int rumor_handle_duplicate(struct rumor **rl, struct rumor **l, struct person *p, struct rumor *r, int id) {

  struct rumor *f;
  float n, m;


#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "RUMOR_HANDLE_DUPLICATE: %d handles a duplicate\n", id);
    fflush(g);
    fclose(g);
  }
#endif

  f = rumor_get_duplicate(*l, r);
  n = rumor_compute_trust(p, f);
  m = rumor_compute_trust(p, r);

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "OLD:\n");
    _rumor_debug_print_rumor(g, f);
    fprintf(g, "NEW:\n");
    _rumor_debug_print_rumor(g, r);
    fflush(g);
    fclose(g);
  }
#endif

  /* which one is more believable, r or its duplicate? */
  if (n >= m) {
    /* the duplicate is -> no changes */
    return SUCCEEDED;
  }
  /* the new rumor r */
  if (!(r->teller.id == id || r->original_teller.id == id)) {
    rumor_remove(l, f);
    rumor_add(rl, r);
  }

#ifdef RUMOR_DEBUG
  {
    FILE *g;
    g = fopen("debug", "ab");
    fprintf(g, "the new rumor was victorious\n");
    fflush(g);
    fclose(g);
  }
#endif

  return SUCCEEDED;
}
