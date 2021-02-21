
int init_creatures(void);
int init_playfield(void);
int display_playfield(void);
int display_respect(void);
int display_rumors(void);
int display_experiences(void);
int gold_announce(void);
int move_creatures(void);
int creature_move(struct creature *c, int x, int y);
int fill_output_buffer(void);
int new_state(int *s, int f);
int decay_rumors(void);
int rumor_generate(struct creature *c, struct creature *t, int ract);
