#ifndef NAGI_LOGIC_LOGIC_EXECUTE_H
#define NAGI_LOGIC_LOGIC_EXECUTE_H

extern u8 *logic_execute(LOGIC *log);
extern void execute_if(void);
extern void skip_true_or(void);
extern void skip_false_and(void);
extern void logic_cmd(void);
extern u8 logic_eval(void);

extern u8 *logic_data;	// si;
extern u8 op;		// al;

#endif /* NAGI_LOGIC_LOGIC_EXECUTE_H */
