
#ifndef _AI_H
#define _AI_H

#define AI_RACE_HUMAN 0
#define AI_RACE_ORC   1
#define AI_RACE_CAT   2
#define AI_RACE_RAT   3

#define AI_CONTROL_PLAYER   0
#define AI_CONTROL_COMPUTER 1

#define AI_ENERGY_LOSS_CAT   0.002
#define AI_ENERGY_LOSS_HUMAN 0.003
#define AI_ENERGY_LOSS_ORC   0.03
#define AI_ENERGY_LOSS_RAT   0.001

/* note that the ID of the mode is also the priority of the mode */
#define AI_MODE_IDLE        0
#define AI_MODE_EXPLORE     1
#define AI_MODE_PLAY_TRICKS 2
#define AI_MODE_CRAP        3
#define AI_MODE_HUNGRY      4
#define AI_MODE_ATTACK      5

struct mode {
  int mode;
  int a, b;            /* some modes use these variables */
  struct mode *next, *prev;
};

struct direction {
  int dir;
  float cost;
};

struct status {
  struct mode *mode;   /* the upmost mode is always the active operating mode */
  int hp_current;
  int hp_max;
  float nutrition;     /* [0, 1] */
};

struct ai {
  int control;
  int mx, my;          /* map coordinates    */
  int sx, sy;          /* screen coordinates */
  struct item *items;
  struct memories mem;
  struct status stats;
  struct map *maps[256];
  struct ai *next, *prev;
};

#define AI_DIRECTION_UP    0
#define AI_DIRECTION_DOWN  1
#define AI_DIRECTION_LEFT  2
#define AI_DIRECTION_RIGHT 3

#define ACTION_FART         0
#define ACTION_SALIVATE     1
#define ACTION_HIT          2
#define ACTION_SAW_WATER    3
#define ACTION_SAW_TREASURE 4
#define ACTION_DESCRIPTION  5

int ai_init(void);
int ai_free(struct ai *a);
int ai_add(struct level *l, int race, int x, int y, struct ai **out);

int ai_mode_add(struct ai *a, int mode);
int ai_mode_remove(struct ai *a);

int ai_move(struct level *l, struct ai *a, int dir);
int ai_pick(struct level *l, struct ai *a);
int ai_operate(struct level *l, struct ai *a, int dir);
int ai_live(struct level *l);
int ai_dies(struct level *l, struct ai *a);

int ai_live_cat(struct level *l, struct ai *a);
int ai_live_human(struct level *l, struct ai *a);
int ai_live_orc(struct level *l, struct ai *a);
int ai_live_rat(struct level *l, struct ai *a);

int ai_propagate(int dx, int sy, int ey, int sx, int ex, float *nav, float *mod);

#endif
