#ifndef trace_h_file
#define trace_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
extern u16 trace_state;
extern u16 trace_logic;
extern u16 logic_called;

/* FUNCTIONS	---	---	---	---	---	---	--- */
extern u8 *cmd_trace_on(u8 *);
extern void trace_init(void);
extern u8 *cmd_trace_info(u8 *);
extern void trace_clear(void);
extern void trace_cmd(u16, u8 *);
extern void trace_eval(u16 , u8 *);

#endif
