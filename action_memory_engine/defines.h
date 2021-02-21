
#ifndef DEFINES_H
#define DEFINES_H


#define FAILED 0
#define SUCCEEDED 1

#define NO 0
#define YES 1

#define OFF 0
#define ON 1

#ifndef M_PI
#define M_PI 3.14159265
#endif


#define ITEM_TYPE_SELF    0
#define ITEM_TYPE_ORGANIC 1
#define ITEM_TYPE_FIRE    2
#define ITEM_TYPE_HARD    3
#define ITEM_TYPE_SHARP   4

#define ITEM_SELF          0
#define ITEM_APPLE         1
#define ITEM_TORCH         2
#define ITEM_MEAT          3
#define ITEM_HUMAN         4
#define ITEM_ORC           5
#define ITEM_GRILLED_APPLE 6
#define ITEM_GRILLED_MEAT  7
#define ITEM_KNIFE         8

#define ACTION_HIT           0
#define ACTION_THROW         1
#define ACTION_COMBINE       2
#define ACTION_GIVE          3
#define ACTION_TELL          4
#define ACTION_SELF_SQUEEZE  5
#define ACTION_SELF_EAT      6
#define ACTION_SELF_TASTE    7
#define ACTION_SELF_SNIFF    8
#define ACTION_SELF_TOUCH    9
#define ACTION_SELF_KISS    10


struct item {
  int id;
  int type;
  struct item *next;
};

struct result {
  float hp;
  float nutrition;
  float pleasure;
  float gold;
  float items;
  float karma;
  float experience;
};

struct action_memory {
  int subject;
  int action;
  int object;
  int count;
  int transformed;
  struct result ress;
  struct result reso;
  struct action_memory *next;
};

#endif
