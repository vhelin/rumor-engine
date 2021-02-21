
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ncurses.h>

#include "defines.h"
#include "../rumor/person.h"
#include "../rumor/rumor.h"
#include "level.h"
#include "ai.h"
#include "main.h"


/* the player character */
struct ai *ai_player = NULL;

/* debug buffer */
char debug_data[256], debug_info = 0;


int main(int argc, char *argv[]) {

  struct level *l;
  int c;


  srand(time(NULL));

  /* ncurses init */
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  /* game init */
  ai_init();
  level_init();
  l = levels[100];
  ai_add(l, AI_RACE_HUMAN, 1, 1, &ai_player);
  ai_player->control = AI_CONTROL_PLAYER;

  ai_add(l, AI_RACE_HUMAN, 10, 5, NULL);
  ai_add(l, AI_RACE_CAT, 14, 15, NULL);
  ai_add(l, AI_RACE_ORC, 30, 6, NULL);
  ai_add(l, AI_RACE_RAT, 40, 15, NULL);
  ai_add(l, AI_RACE_RAT, 34, 16, NULL);

  /* main loop */
  while (1) {
    /* display graphics */
    clear();

    level_display(l, ai_player->mx, ai_player->my, 61, 21);

    if (debug_info > 0) {
      debug_info--;
      move(23, 0);
      printw(debug_data);
    }

    move(ai_player->sy, ai_player->sx);
    refresh();

    /* read the keyboard */
    c = getch();

    /* QUIT? */
    if (c == 'Q')
      break;
    /* PICK? */
    else if (c == ',') {
      ai_pick(l, ai_player);
    }
    /* OPERATE? */
    else if (c == 'o') {
      move(22, 0);
      printw("Operate direction: ");
      refresh();

      /* read the keyboard */
      c = getch();

      if (c == KEY_UP)
	ai_operate(l, ai_player, AI_DIRECTION_UP);
      else if (c == KEY_DOWN)
	ai_operate(l, ai_player, AI_DIRECTION_DOWN);
      else if (c == KEY_LEFT)
	ai_operate(l, ai_player, AI_DIRECTION_LEFT);
      else if (c == KEY_RIGHT)
	ai_operate(l, ai_player, AI_DIRECTION_RIGHT);
    }
    /* MOVE? */
    else if (c == KEY_UP) {
      ai_move(l, ai_player, AI_DIRECTION_UP);
    }
    else if (c == KEY_DOWN) {
      ai_move(l, ai_player, AI_DIRECTION_DOWN);
    }
    else if (c == KEY_LEFT) {
      ai_move(l, ai_player, AI_DIRECTION_LEFT);
    }
    else if (c == KEY_RIGHT) {
      ai_move(l, ai_player, AI_DIRECTION_RIGHT);
    }

    /* let the ai live for one cycle */
    ai_live(l);
  }

  /* ncurses exit */
  endwin();

  return 0;
}
