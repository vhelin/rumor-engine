
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "defines.h"
#include "action_memory.h"
#include "unit_test.h"


struct action_memory *memories = NULL;


int main(int argc, char *argv[]) {

  struct action_memory *m;


  srand(time(NULL));

  if (action_memory_create(&m) == FAILED)
    return 1;
  m->subject = 1;
  m->action = ACTION_HIT;
  m->object = 2;

  m->ress.hp = 3;
  if (action_memory_add(&memories, m) == FAILED)
    return 1;
  m->ress.hp = 6;
  if (action_memory_add(&memories, m) == FAILED)
    return 1;
  m->ress.hp = 3;
  if (action_memory_add(&memories, m) == FAILED)
    return 1;

  action_memory_list(memories);

  printf("\nTEST 1: PASSED.\n\n");

  m->object = 3;
  if (action_memory_add(&memories, m) == FAILED)
    return 1;

  action_memory_list(memories);

  printf("\nTEST 2: PASSED.\n\n");

  return 0;
}
