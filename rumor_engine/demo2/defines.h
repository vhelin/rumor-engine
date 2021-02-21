
#ifndef DEFINES_H
#define DEFINES_H 0

#define FAILED 0
#define SUCCEEDED 1

#define NO 0
#define YES 1

#define OFF 0
#define ON 1

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define ACTION_FART         0
#define ACTION_SALIVATE     1
#define ACTION_HIT          2
#define ACTION_SAW_WATER    3
#define ACTION_SAW_TREASURE 4

#define CSTATE_IDLE          0
#define CSTATE_FIND_COMPANY  1
#define CSTATE_FART          2
#define CSTATE_SALIVATE      3
#define CSTATE_FIND_TREASURE 4
#define CSTATE_FIND_WATER    4
#define CSTATE_FIND_FOOD     5
#define CSTATE_LAST          5
#define CSTATE_TALK          6
#define CSTATE_EAT           7
#define CSTATE_HIT           8
#define CSTATE_LOOK_TREASURE 9
#define CSTATE_LOOK_WATER    10

#define RACE_HUMAN 0
#define RACE_ORC   1

struct creature {
  int id;
  int x, y;
  int state;
  unsigned char symbol;
  struct memories mem;
};

#endif
