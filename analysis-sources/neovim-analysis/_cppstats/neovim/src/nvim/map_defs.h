#include "nvim/lib/khash.h"
typedef const char * cstr_t;
typedef void * ptr_t;
#define Map(T, U) Map_##T##_##U
#define PMap(T) Map(T, ptr_t)
