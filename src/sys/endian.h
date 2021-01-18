#ifndef SRC_SYS_ENDIAN
#define SRC_SYS_ENDIAN

#include <stdint.h>

extern uint16_t load_le_16(const void *data);
extern uint16_t load_be_16(const void *data);
extern void store_le_16(void *data, uint16_t value);
extern void store_be_16(void *data, uint16_t value);

#endif /* SRC_SYS_ENDIAN */
