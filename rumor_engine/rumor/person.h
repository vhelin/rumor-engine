
#ifndef _PERSON_H
#define _PERSON_H

#define PERSON_INITIAL_RESPECT 0.5

struct person {
  int id;
  int race;
  float respect;
  struct person *next;
};

int person_check(struct person **l, struct person *p);
int person_create_and_add(struct person **l, struct person *p, struct person **out);
float person_compute_initial_respect(struct person *l, struct person *p);
struct person *person_get(struct person *l, struct person *p);
struct person *person_create(void);

#endif /* _PERSON_H */
