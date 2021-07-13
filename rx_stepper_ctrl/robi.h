#pragma

void robi_init(void);
void robi_main(int ticks, int menu);
int robi_connected(void);
void robi_set_screw_current(uint8_t current);
void robi_set_output(int num, int val);
int robi_current_version(void);
int robi_is_updating(void);
int robi_is_init(void);
int robi_in_ref(void);
int robi_screwer_in_ref(void);
void robi_stop(void);
void robi_reference(void);
void robi_move_X_relative_steps(INT32 steps);
void robi_move_Y_relative_steps(INT32 steps);
void robi_turn_screw_relative(INT32 steps);
void robi_turn_screw_left(INT32 ticks);
void robi_turn_screw_right(INT32 ticks);
void robi_move_up(void);
void robi_move_down(void);
int robi_screwer_stalled(void);
int robi_not_started(void);
int robi_z_not_reached_up(void);
int robi_z_not_reached_down(void);
int robi_move_done(void);
