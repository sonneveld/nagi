#ifndef NAGI_SYS_DELAY_H
#define NAGI_SYS_DELAY_H

//_DoClock                       cseg     00007E35 0000007C
//_DoDelay                       cseg     00007EB1 0000001D

extern u32 calc_agi_tick(void);
extern void do_delay(void);

extern void delay_init(void);

#endif /* NAGI_SYS_DELAY_H */
