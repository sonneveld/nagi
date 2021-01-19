// memory allocations
// _Malloc                        cseg     000013D6 0000005A
// _GetMemPtr                     cseg     00001430 0000000C
// _SetMemPtr                     cseg     0000143C 0000000F
// _SaveMemPtr                    cseg     0000144B 0000000F
// _RestoreMemPtr                 cseg     0000145A 0000001C
// _SetMemRm0                     cseg     00001476 0000000F
// _ClearMemory                   cseg     00001485 0000001B
// _UpdateVar8                    cseg     000014A0 0000001D
// CmdShowMem                     cseg     000014BD 0000004D
// END memory allocations


void init_agi_heap(void);
u16 update_var8(void);
void *agi_malloc(u16 size);
void *get_memptr(void);
void set_memptr(void *ptr);
void save_memptr(void);
void restore_memptr(void);
void set_memrm0(void);
void clear_memory(void);

/* u8 *CmdShowMem(*u8); */

