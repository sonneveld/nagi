#ifndef NAGI_SYS_ENDIAN_H
#define NAGI_SYS_ENDIAN_H

extern u16 load_le_16(const void *data);
extern u16 load_be_16(const void *data);
extern void store_le_16(void *data, u16 value);
extern void store_be_16(void *data, u16 value);

#endif /* NAGI_SYS_ENDIAN_H */
