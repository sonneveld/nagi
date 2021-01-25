#ifndef NAGI_SYS_ERROR_H
#define NAGI_SYS_ERROR_H

extern void long_beep(void);
extern void beep_speaker(void);
extern AGI_NO_RETURN void set_agi_error(u16 err1, u16 err2);
extern u16 print_err_code(void);

extern jmp_buf agi_err_state;

#endif /* NAGI_SYS_ERROR_H */
