
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "../rumor/person.h"
#include "../rumor/rumor.h"
#include "defines.h"
#include "main.h"


#define CREATURES 5


struct action actions[] = {
  { ACTION_FART,         0.1, 0.8, 1.0, 1.0, 0.0, 0.0, 0.0, +0.1 },
  { ACTION_SALIVATE,     0.1, 0.8, 1.0, 1.0, 0.0, 0.0, 0.0, -0.1 },
  { ACTION_HIT,          0.6, 0.8, 1.0, 1.0, 0.0, 0.0, 0.0, -0.2 },
  { ACTION_SAW_TREASURE, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.0 },
};

struct creature creatures[CREATURES];
unsigned char *playfield, *output_buffer;
char name_playfield[] = "playfield.txt";
int playfield_x, playfield_y;


int main(int argc, char *argv[]) {

  int c, i, x, y;


  srand(time(NULL));

  rumor_init(actions);
  init_playfield();
  init_creatures();

  i = 0;
  while (1) {
    printf("%c[2J", 27);
    printf("%c[200A%c[200D", 27, 27);
    decay_rumors();
    gold_announce();
    display_playfield();
    display_respect();
    display_rumors();
    display_experiences();
    c = getchar();

    /* check the commands */
    if (c == 'q')
      return 0;

    move_creatures();

    /* introduce a new food plant every n cycles */
    if (i == 6) {
      i = 0;
      c = 0;
      while (1) {
	c++;
	if (c == 70)
	  break;
	x = rand() % playfield_x;
	y = rand() % playfield_y;
	x = y*playfield_x + x;
	if (playfield[x] == '.') {
	  playfield[x] = 'f';
	  break;
	}
      }
    }

    i++;
  }

  return 0;
}


int gold_announce(void) {

  struct person tmp;
  struct rumor *r;
  float e, f;
  int x, y, i;


  for (y = 0; y < playfield_y; y++) {
    for (x = 0; x < playfield_x; x++) {
      if (playfield[y*playfield_x + x] == '$') {
	for (i = 0; i < CREATURES; i++) {
	  e = creatures[i].x - x;
	  f = creatures[i].y - y;
	  e = sqrt(e*e + f*f);
	  if (e < 3) {
	    tmp.id = i;
	    tmp.race = creatures[i].mem.self->race;
	    r = rumor_create_experience(&tmp);
	    r->subject.id = i;
	    r->subject.race = tmp.race;
	    r->object.id = i;
	    r->object.race = tmp.race;
	    r->action = ACTION_SAW_TREASURE;
	    r->priority = actions[r->action].priority;
	    r->x = x;
	    r->y = y;
	    r->timer = 100;
	    rumor_add_with_checks(&creatures[i].mem.experience_list, &creatures[i].mem.person_list, r);

	    i = CREATURES;
	    playfield[y*playfield_x + x] = '.';
	  }
	}
      }
    }
  }

  return SUCCEEDED;
}


int decay_rumors(void) {

  int i;


  for (i = 0; i < CREATURES; i++) {
    rumor_decay(&creatures[i].mem.rumor_list);
    rumor_decay(&creatures[i].mem.experience_list);
    rumor_decay(&creatures[i].mem.old_rumors_list);
  }

  return SUCCEEDED;
}


int init_creatures(void) {

  struct person *p, tmp;
  int i, z;


  memcpy(output_buffer, playfield, playfield_x*playfield_y);

  for (i = 0; i < CREATURES; i++) {
    creatures[i].id = i;
    creatures[i].symbol = '0'+i;
    creatures[i].state = CSTATE_IDLE;

    while (1) {
      creatures[i].x = (int)(rand()/(double)RAND_MAX*playfield_x);
      creatures[i].y = (int)(rand()/(double)RAND_MAX*playfield_y);
      z = creatures[i].y*playfield_x + creatures[i].x;
      if (output_buffer[z] == '.')
	break;
    }

    output_buffer[z] = 'D';

    creatures[i].mem.person_list = NULL;
    creatures[i].mem.rumor_list = NULL;
    creatures[i].mem.old_rumors_list = NULL;
    creatures[i].mem.experience_list = NULL;

    tmp.id = i;
    tmp.race = (rand()%2)+1;

    /* make the creature know itself */
    person_create_and_add(&creatures[i].mem.person_list, &tmp, &p);
    creatures[i].mem.self = p;
    p->respect = 1.0;
    /*
    if (i != 1) {
      person_create_and_add(&creatures[i].mem.person_list, 1, &p);
      p->respect = 0.8;
    }
    if (i != 0) {
      person_create_and_add(&creatures[i].mem.person_list, 0, &p);
      p->respect = 0.69;
    }
    if (i == 0) {
      struct rumor *r;
      r = rumor_create();
      r->priority = 1.0;
      r->subject = 1;
      r->object = 0;
      r->action = ACTION_HIT;
      r->teller = 0;
      r->original_teller = 0;
      r->timer = 100;
      r->status = RUMOR_STATUS_SAW | RUMOR_STATUS_TRUE;
      r->x = 0;
      r->y = 0;
      r->next = NULL;
      rumor_add_with_checks(&creatures[i].mem.experience_list, &creatures[i].mem.person_list, r);
    }
    */
  }
  return SUCCEEDED;
}


int init_playfield(void) {

  FILE *f;
  int x, y, i, z;
  unsigned char c;


  f = fopen(name_playfield, "rb");
  if (f == NULL) {
    fprintf(stderr, "INIT_PLAYFIELD: Could not open file \"%s\".\n", name_playfield);
    return FAILED;
  }

  fscanf(f, "%d %d", &x, &y);

  playfield = malloc(x*y);
  output_buffer = malloc(x*y);
  if (playfield == NULL || output_buffer == NULL) {
    fprintf(stderr, "INIT_PLAYFIELD: Out of memory error.\n");
    if (output_buffer != NULL)
      free(output_buffer);
    if (playfield != NULL)
      free(playfield);
    fclose(f);
    return FAILED;
  }

  /* read in the playfield */
  for (z = 0, i = 0; z < x*y; i++) {
    fscanf(f, "%c", &c);
    if (c >= '.' || c == '$') {
      *(playfield+z) = c;
      z++;
    }
  }

  playfield_x = x;
  playfield_y = y;

  fclose(f);

  return SUCCEEDED;
}


int display_playfield(void) {

  int x, y, i;


  fill_output_buffer();

  for (i = 0, y = 0; y < playfield_y; y++) {
    for (x = 0; x < playfield_x; x++, i++) {
      printf("%c", output_buffer[i]);
    }
    printf("\n");
  }

  return SUCCEEDED;
}


int move_creatures(void) {

  float e, f, l;
  int i, d, m, n, q, p, r;


  for (i = 0; i < CREATURES; i++) {
    /* idling? */
    if (creatures[i].state == CSTATE_IDLE) {
      if ((rand() % 7) == 0)
	new_state(&creatures[i].state, i);
    }
    /* hitting? */
    else if (creatures[i].state == CSTATE_HIT) {
      q = 0;
      l = 10000;
      for (d = 0; d < CREATURES; d++) {
	if (d == i)
	  continue;
	e = creatures[i].x - creatures[d].x;
	f = creatures[i].y - creatures[d].y;
	e = sqrt(e*e + f*f);
	if (e < l) {
	  l = e;
	  q = d;
	}
      }

      m = creatures[q].x - creatures[i].x;
      n = creatures[q].y - creatures[i].y;

      if (abs(n)+abs(m) == 1) {
	rumor_generate(&creatures[i], &creatures[q], ACTION_HIT);
	new_state(&creatures[i].state, i);
      }

      new_state(&creatures[i].state, i);
    }
    /* look for the treasure */
    else if (creatures[i].state == CSTATE_LOOK_TREASURE) {

      struct rumor *r;


      r = creatures[i].mem.experience_list;
      while (r != NULL) {
	if (r->action == ACTION_SAW_TREASURE && (r->status & RUMOR_STATUS_TRUE) != 0)
	  break;
	r = r->next;
      }
      if (r == NULL) {
	r = creatures[i].mem.rumor_list;
	while (r != NULL) {
	  if (r->action == ACTION_SAW_TREASURE && (r->status & RUMOR_STATUS_TRUE) != 0)
	    break;
	  r = r->next;
	}
      }

      /* can i see the treasure? */
      d = 0;
      if (r != NULL) {
	for (p = 0; p < playfield_y; p++) {
	  for (q = 0; q < playfield_x; q++) {
	    if (playfield[p*playfield_x + q] == '$') {
	      m = q - creatures[i].x;
	      n = p - creatures[i].y;
	      if (sqrt(m*m + n*n) < 2) {
		d = 1;
		p = playfield_y;
		q = playfield_x;
	      }
	    }
	  }
	}

	/* it wasn't there! */
	if (d == 0) {
	  /* it was a lie! handle the situation */
	  rumor_handle_lie(&creatures[i].mem, r);
	}
	/* it's there ok! */
	else {
	  rumor_remove(&creatures[i].mem.rumor_list, r);
	}
      }

      new_state(&creatures[i].state, i);
    }
    /* go check out the treasure */
    else if (creatures[i].state == CSTATE_FIND_TREASURE) {

      struct rumor *r;


      r = creatures[i].mem.experience_list;
      while (r != NULL) {
	if (r->action == ACTION_SAW_TREASURE && (r->status & RUMOR_STATUS_TRUE) != 0)
	  break;
	r = r->next;
      }
      if (r == NULL) {
	r = creatures[i].mem.rumor_list;
	while (r != NULL) {
	  if (r->action == ACTION_SAW_TREASURE && (r->status & RUMOR_STATUS_TRUE) != 0)
	    break;
	  r = r->next;
	}
      }

      d = 0;

      /* i knows about a treasure */
      if (r != NULL) {
	m = r->x - creatures[i].x;
	n = r->y - creatures[i].y;

	if (abs(n)+abs(m) == 1) {
	  creatures[i].state = CSTATE_LOOK_TREASURE;
	  d = 1;
	}
	else {
	/* move towards the treasure */
	  if (m < 0)
	    m = -1;
	  else if (m > 0)
	    m = 1;
	  if (n < 0)
	    n = -1;
	  else if (n > 0)
	    n = 1;

	  if (creature_move(&creatures[i], m, n) == FAILED) {
	    new_state(&creatures[i].state, i);
	    d = 1;
	  }
	}
      }
      if (d == 0)
	new_state(&creatures[i].state, i);
    }
    /* find someone to talk with */
    else if (creatures[i].state == CSTATE_FIND_COMPANY) {

      struct person *p;

      q = -1;
      l = 10000;
      for (d = 0; d < CREATURES; d++) {
	if (d == i)
	  continue;
	e = creatures[i].x - creatures[d].x;
	f = creatures[i].y - creatures[d].y;

	/* check out if i liked about d enough */
	p = person_get(creatures[i].mem.person_list, creatures[d].mem.self);
	if (p != NULL && p->respect < 0.5)
	  continue;

	e = sqrt(e*e + f*f);
	if (e < l) {
	  l = e;
	  q = d;
	}
      }

      if (q >= 0) {
	m = creatures[q].x - creatures[i].x;
	n = creatures[q].y - creatures[i].y;

	if (abs(n)+abs(m) == 1) {
	  /* does 5 hit instead? */
	  if (i == 5) {
	    if (rand() % 10 == 0)
	      creatures[i].state = CSTATE_HIT;
	    else {
	      creatures[i].state = CSTATE_TALK;
	      creatures[q].state = CSTATE_TALK;
	    }
	  }
	  else {
	    creatures[i].state = CSTATE_TALK;
	    creatures[q].state = CSTATE_TALK;
	  }
	}
	else if (l < 10) {
	  /* move towards the nearest partner */
	  if (m < 0)
	    m = -1;
	  else if (m > 0)
	    m = 1;
	  if (n < 0)
	    n = -1;
	  else if (n > 0)
	    n = 1;

	  if (creature_move(&creatures[i], m, n) == FAILED) {
	    new_state(&creatures[i].state, i);
	  }
	}
	else
	  new_state(&creatures[i].state, i);
      }
      else {
	new_state(&creatures[i].state, i);
      }
    }
    /* blow a juicy fart */
    else if (creatures[i].state == CSTATE_FART) {
      rumor_generate(&creatures[i], &creatures[i], ACTION_FART);
      new_state(&creatures[i].state, i);
    }
    /* salivate */
    else if (creatures[i].state == CSTATE_SALIVATE) {
      rumor_generate(&creatures[i], &creatures[i], ACTION_SALIVATE);
      new_state(&creatures[i].state, i);
    }
    /* find some food */
    else if (creatures[i].state == CSTATE_FIND_FOOD) {
      p = 0;
      r = 0;
      l = 10000;
      /* locate closest food */
      for (m = 0; m < playfield_y; m++) {
	for (n = 0; n < playfield_x; n++) {
	  q = m*playfield_x + n;
	  if (playfield[q] == 'f') {
	    e = creatures[i].x - n;
	    f = creatures[i].y - m;
	    e = sqrt(e*e + f*f);
	    if (e < l) {
	      p = n;
	      r = m;
	      l = e;
	    }
	  }
	}
      }

      if (l < 10) {
	m = r - creatures[i].y;
	n = p - creatures[i].x;

	if (abs(n)+abs(m) == 1) {
	  creatures[i].state = CSTATE_EAT;
	}
	else {
	  /* move towards the nearest food */
	  if (m < 0)
	    m = -1;
	  else if (m > 0)
	    m = 1;
	  if (n < 0)
	    n = -1;
	  else if (n > 0)
	    n = 1;

	  if (creature_move(&creatures[i], n, m) == FAILED) {
	    new_state(&creatures[i].state, i);
	  }
	}
      }
      else {
	new_state(&creatures[i].state, i);
      }
    }
    /* talk */
    else if (creatures[i].state == CSTATE_TALK) {
      q = 0;
      l = 10000;
      for (d = 0; d < CREATURES; d++) {
	if (d == i)
	  continue;
	e = creatures[i].x - creatures[d].x;
	f = creatures[i].y - creatures[d].y;
	e = sqrt(e*e + f*f);
	if (e < l) {
	  l = e;
	  q = d;
	}
      }

      m = creatures[q].x - creatures[i].x;
      n = creatures[q].y - creatures[i].y;

      if (abs(n)+abs(m) == 1) {
	rumor_tell(&creatures[i].mem, &creatures[q].mem);
      }

      new_state(&creatures[i].state, i);
    }
    /* eat */
    else if (creatures[i].state == CSTATE_EAT) {
      
      n = creatures[i].y*playfield_x + creatures[i].x;
      if (playfield[n + 1] == 'f')
	playfield[n + 1] = '.';
      else if (playfield[n - 1] == 'f')
	playfield[n - 1] = '.';
      else if (playfield[n + playfield_x] == 'f')
	playfield[n + playfield_x] = '.';
      else if (playfield[n - playfield_x] == 'f')
	playfield[n - playfield_x] = '.';

      new_state(&creatures[i].state, i);
    }
  }

  return SUCCEEDED;
}


int creature_move(struct creature *c, int x, int y) {

  int a;


  fill_output_buffer();
  a = c->y*playfield_x + c->x;

  if (c->x == 0 && x < 0)
    return FAILED;
  if (c->y == 0 && y < 0)
    return FAILED;
  if (c->x == playfield_x-1 && x > 0)
    return FAILED;
  if (c->y == playfield_y-1 && y > 0)
    return FAILED;

  if (output_buffer[a + x + y*playfield_x] == '.') {
    c->x += x;
    c->y += y;
    return SUCCEEDED;
  }

  return FAILED;
}


int fill_output_buffer(void) {

  int i, x;


  memcpy(output_buffer, playfield, playfield_x*playfield_y);

  /* print creatures */
  for (i = 0; i < CREATURES; i++) {
    x = creatures[i].y*playfield_x + creatures[i].x;
    if (creatures[i].state == CSTATE_FART)
      output_buffer[x] = 'F';
    else if (creatures[i].state == CSTATE_SALIVATE)
      output_buffer[x] = 'S';
    else if (creatures[i].state == CSTATE_HIT)
      output_buffer[x] = 'H';
    else
      output_buffer[x] = creatures[i].symbol;
  }

  return SUCCEEDED;
}


int display_respect(void) {

  struct person *p;
  int i, r;
  char c, s;


  printf("STATE:\n");
  for (i = 0; i < CREATURES; i++) {
    r = creatures[i].mem.self->race;
    if (r == RACE_HUMAN)
      c = 'h';
    if (r == RACE_ORC)
      c = 'o';
    r = creatures[i].state;
    if (r == CSTATE_IDLE)
      s = 'I';
    if (r == CSTATE_FIND_COMPANY)
      s = 'C';
    if (r == CSTATE_FART)
      s = 'F';
    if (r == CSTATE_SALIVATE)
      s = 'S';
    if (r == CSTATE_FIND_TREASURE)
      s = 'T';
    if (r == CSTATE_FIND_FOOD)
      s = 'Q';
    if (r == CSTATE_TALK)
      s = 'D';
    if (r == CSTATE_EAT)
      s = 'E';
    if (r == CSTATE_HIT)
      s = 'H';
    if (r == CSTATE_LOOK_TREASURE)
      s = 'L';
    printf("%c:%c%c ", creatures[i].symbol, c, s);
  }
  printf("\n");

  printf("RESPECT:\n");
  for (i = 0; i < CREATURES; i++) {
    printf("%c ---> ", creatures[i].symbol);
    p = creatures[i].mem.person_list;
    while (p != NULL) {
      printf("%c:%f ", creatures[p->id].symbol, p->respect);
      p = p->next;
    }
    printf("\n");
  }

  return SUCCEEDED;
}


int display_rumors(void) {

  struct rumor *r;
  int i;


  printf("RUMORS:\n");
  for (i = 0; i < CREATURES; i++) {
    printf("%c ---> ", creatures[i].symbol);
    r = creatures[i].mem.rumor_list;
    while (r != NULL) {
      if (r->action == ACTION_FART)
	{
	  r = r->next;
	  continue;
	}
      /*
	printf("%c farted ", creatures[r->subject].symbol);
      */

      if ((r->status & RUMOR_STATUS_FALSE) != 0)
	printf("F:");
      if ((r->status & RUMOR_STATUS_TRUE) != 0)
	printf("T:");

      if (r->action == ACTION_HIT)
	printf("%c hit %c ", creatures[r->subject.id].symbol, creatures[r->object.id].symbol);
      if (r->action == ACTION_SALIVATE)
	printf("%c salivated ", creatures[r->subject.id].symbol);
      if (r->action == ACTION_SAW_TREASURE)
	printf("%c saw $ in (%d, %d) ", creatures[r->subject.id].symbol, (int)r->x, (int)r->y);

      printf("%d ", r->timer);

      if ((r->status & RUMOR_STATUS_HEARD) != 0)
	printf("(h:%d o:%d ", r->teller.id, r->original_teller.id);
      if ((r->status & RUMOR_STATUS_SAW) != 0)
	printf("(s:%d o:%d ", r->teller.id, r->original_teller.id);
      printf("e: %f). ", r->effect_subject);
      r = r->next;
    }
    printf("\n");
  }

  printf("OLD RUMORS (which have no effect in the future):\n");
  for (i = 0; i < CREATURES; i++) {
    printf("%c ---> ", creatures[i].symbol);
    r = creatures[i].mem.old_rumors_list;
    while (r != NULL) {
      if (r->action == ACTION_FART)
	{
	  r = r->next;
	  continue;
	}

      if ((r->status & RUMOR_STATUS_FALSE) != 0)
	printf("F:");
      if ((r->status & RUMOR_STATUS_TRUE) != 0)
	printf("T:");

      if (r->action == ACTION_HIT)
	printf("%c hit %c ", creatures[r->subject.id].symbol, creatures[r->object.id].symbol);
      if (r->action == ACTION_SALIVATE)
	printf("%c salivated ", creatures[r->subject.id].symbol);
      if (r->action == ACTION_SAW_TREASURE)
	printf("%c saw $ in (%d, %d) ", creatures[r->subject.id].symbol, (int)r->x, (int)r->y);

      printf("%d ", r->timer);

      if ((r->status & RUMOR_STATUS_HEARD) != 0)
	printf("(h:%d o:%d). ", r->teller.id, r->original_teller.id);
      if ((r->status & RUMOR_STATUS_SAW) != 0)
	printf("(s:%d o:%d). ", r->teller.id, r->original_teller.id);
      r = r->next;
    }
    printf("\n");
  }

  return SUCCEEDED;
}


int display_experiences(void) {

  struct rumor *r;
  int i;


  printf("EXPERIENCES:\n");
  for (i = 0; i < CREATURES; i++) {
    printf("%c ---> ", creatures[i].symbol);
    r = creatures[i].mem.experience_list;
    while (r != NULL) {
      if (r->action == ACTION_FART)
	{
	  r = r->next;
	  continue;
	}

      if ((r->status & RUMOR_STATUS_FALSE) != 0)
	printf("F:");
      if ((r->status & RUMOR_STATUS_TRUE) != 0)
	printf("T:");

      if (r->action == ACTION_HIT)
	printf("%c hit %c ", creatures[r->subject.id].symbol, creatures[r->object.id].symbol);
      if (r->action == ACTION_SALIVATE)
	printf("%c salivated ", creatures[r->subject.id].symbol);
      if (r->action == ACTION_SAW_TREASURE)
	printf("%c saw $ in (%d, %d) ", creatures[r->subject.id].symbol, (int)r->x, (int)r->y);

      printf("%d ", r->timer);

      if ((r->status & RUMOR_STATUS_HEARD) != 0)
	printf("(h:%d o:%d). ", r->teller.id, r->original_teller.id);
      if ((r->status & RUMOR_STATUS_SAW) != 0)
	printf("(s:%d o:%d). ", r->teller.id, r->original_teller.id);
      r = r->next;
    }
    printf("\n");
  }

  return SUCCEEDED;
}


int new_state(int *s, int f) {

  /* '9' can only FART and IDLE */
  if (f == 9) {
    *s = rand() % (CSTATE_LAST+1);
    while (!(*s == CSTATE_FART || *s == CSTATE_IDLE))
      *s = rand() % (CSTATE_LAST+1);
    return SUCCEEDED;
  }

  /* only '8' can SALIVATE */
  if (f == 8) {
    *s = rand() % (CSTATE_LAST+1);
    while (*s == CSTATE_FART)
      *s = rand() % (CSTATE_LAST+1);
    return SUCCEEDED;
  }

  *s = rand() % (CSTATE_LAST+1);
  while (*s == CSTATE_FART || *s == CSTATE_SALIVATE) {
    *s = rand() % (CSTATE_LAST+1);
  }

  return SUCCEEDED;
}


int rumor_generate(struct creature *c, struct creature *t, int ract) {

  struct person *p;
  struct rumor *r;
  float x, y;
  int i;


  for (i = 0; i < CREATURES; i++) {
    /* generate a fart rumor */
    if (ract == ACTION_FART) {
      x = c->x - creatures[i].x;
      y = c->y - creatures[i].y;
      x = sqrt(x*x + y*y);
      /* can the other person hear the fart? */
      if (x < 3) {
	r = rumor_create();
	r->subject.id = c->mem.self->id;
	r->subject.race = c->mem.self->race;
	r->object.id = t->mem.self->id;
	r->object.race = t->mem.self->race;
	r->action = ract;
	r->priority = actions[r->action].priority;
	r->original_teller.id = creatures[i].mem.self->id;
	r->original_teller.race = creatures[i].mem.self->race;
	r->teller.id = creatures[i].mem.self->id;
	r->teller.race = creatures[i].mem.self->race;
	r->timer = 100;
	r->next = NULL;
	r->status = RUMOR_STATUS_TRUE | RUMOR_STATUS_SAW;
	r->x = 0;
	r->y = 0;
	if (c != &creatures[i])
	  rumor_experience(&creatures[i].mem.experience_list, &creatures[i].mem.person_list, r);
	rumor_add_with_checks(&creatures[i].mem.experience_list, &creatures[i].mem.person_list, r);
      }
    }
    /* generate a salivate rumor */
    if (ract == ACTION_SALIVATE) {
      x = c->x - creatures[i].x;
      y = c->y - creatures[i].y;
      x = sqrt(x*x + y*y);
      /* can the other person see the saliva? */
      if (x < 2) {
	r = rumor_create();
	r->subject.id = c->mem.self->id;
	r->subject.race = c->mem.self->race;
	r->object.id = t->mem.self->id;
	r->object.race = t->mem.self->race;
	r->action = ract;
	r->priority = actions[r->action].priority;
	r->original_teller.id = creatures[i].mem.self->id;
	r->original_teller.race = creatures[i].mem.self->race;
	r->teller.id = creatures[i].mem.self->id;
	r->teller.race = creatures[i].mem.self->race;
	r->timer = 100;
	r->next = NULL;
	r->status = RUMOR_STATUS_TRUE | RUMOR_STATUS_SAW;
	r->x = 0;
	r->y = 0;
	if (c != &creatures[i])
	  rumor_experience(&creatures[i].mem.experience_list, &creatures[i].mem.person_list, r);
	rumor_add_with_checks(&creatures[i].mem.experience_list, &creatures[i].mem.person_list, r);
      }
    }
    /* generate a hit rumor */
    if (ract == ACTION_HIT) {
      x = c->x - creatures[i].x;
      y = c->y - creatures[i].y;
      x = sqrt(x*x + y*y);
      /* can the other person see the beating? */
      if (x < 2) {
	if (&creatures[i] == t) {
	  person_check(&t->mem.person_list, c->mem.self);
	  p = person_get(t->mem.person_list, c->mem.self);
	  rumor_change_respect(&t->mem.rumor_list, p, -0.2);
	}
	r = rumor_create();
	r->subject.id = c->mem.self->id;
	r->subject.race = c->mem.self->race;
	r->object.id = t->mem.self->id;
	r->object.race = t->mem.self->race;
	r->action = ract;
	r->priority = actions[r->action].priority;
	r->original_teller.id = creatures[i].mem.self->id;
	r->original_teller.race = creatures[i].mem.self->race;
	r->teller.id = creatures[i].mem.self->id;
	r->teller.race = creatures[i].mem.self->race;
	r->timer = 200;
	r->next = NULL;
	r->status = RUMOR_STATUS_TRUE | RUMOR_STATUS_SAW;
	r->x = 0;
	r->y = 0;
	if (c != &creatures[i])
	  rumor_experience(&creatures[i].mem.experience_list, &creatures[i].mem.person_list, r);
	rumor_add_with_checks(&creatures[i].mem.experience_list, &creatures[i].mem.person_list, r);
      }
    }
  }

  return SUCCEEDED;
}
