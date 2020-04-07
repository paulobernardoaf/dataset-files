
#if !defined(ARCH_H)
#define ARCH_H

#include <stdint.h>




typedef struct gdb_reg {
char name[32]; 
uint64_t offset; 
uint64_t size; 
} gdb_reg_t;

gdb_reg_t *arch_parse_reg_profile(const char * reg_profile);

#endif
