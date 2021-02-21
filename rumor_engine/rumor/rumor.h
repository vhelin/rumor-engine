
#ifndef _RUMOR_H
#define _RUMOR_H

#define RUMOR_STATUS_HEARD 1
#define RUMOR_STATUS_SAW   2
#define RUMOR_STATUS_TRUE  4
#define RUMOR_STATUS_FALSE 8

#define RUMOR_CLASH 2

struct rumor {
  int id;
  float priority;
  float effect_subject;
  float effect_object;
  float effect_teller;
  float effect_original_teller;
  struct person subject;
  struct person object;
  struct person teller;
  struct person original_teller;
  int action;
  int timer;
  int status;
  int tell_count;
  float x, y;
  struct rumor *next;
};

#define ACTION_STATUS_IS           1
#define ACTION_STATUS_OTHER        2
#define ACTION_STATUS_DESCRIPTION  3

struct action {
  int id;
  int status;
  float priority;
  float hear;
  float see;
  float subject;
  float object;
  float teller;
  float original_teller;
  float severity;
};

struct memories {
  struct person *self;
  struct person *person_list;
  struct rumor *rumor_list;
  struct rumor *old_rumors_list;
  struct rumor *experience_list;
};

int rumor_init(struct action *a);
int rumor_experience(struct rumor **l, struct person **p, struct rumor *r);
int rumor_tell(struct memories *c, struct memories *r);
int rumor_find_duplicate(struct rumor *l, struct rumor *r);
int rumor_decay(struct rumor **l);
int rumor_filter(struct rumor **l, int id);
int rumor_remove(struct rumor **l, struct rumor *r);
int rumor_remove_all(struct rumor **l, struct rumor *r);
int rumor_add(struct rumor **l, struct rumor *r);
int rumor_add_with_checks(struct rumor **l, struct person **p, struct rumor *r);
int rumor_change_respect(struct rumor **l, struct person *p, float r);
int rumor_find_clone(struct rumor *l, struct rumor *r);
int rumor_handle_clash(struct rumor **l, struct person **p, struct rumor *r);
int rumor_handle_lie(struct memories *a, struct rumor *r);
int rumor_handle_personal_rumor(struct memories *a, struct memories *b, struct rumor *r);
int rumor_handle_duplicate(struct rumor **rl, struct rumor **l, struct person *p, struct rumor *r, int id);
int rumor_tell_its_not_true(struct memories *a, struct memories *b, struct rumor *r);
int rumor_free(struct rumor *r);
int rumor_neutralize_effect(struct rumor **l, struct person **p, struct rumor *r);
float rumor_compute_trust(struct person *p, struct rumor *r);
struct rumor *rumor_select(struct memories *a, struct memories *b);
struct rumor *rumor_get_duplicate(struct rumor *l, struct rumor *r);
struct rumor *rumor_create_negated_rumor(struct rumor *r);
struct rumor *rumor_create(void);
struct rumor *rumor_create_experience(struct person *p);
struct rumor *rumor_clone(struct rumor *r);

#endif /* _RUMOR_H */
