#define DUPMAX 255
#define INTFINITY (DUPMAX + 1)
#define NC (CHAR_MAX - CHAR_MIN + 1)
#define STRLCPY(x,y,z) { strncpy ((x),(y),(z)); (x)[(z)? (z) - 1: 0] = 0; }
#if !defined(REDEBUG)
#if !defined(NDEBUG)
#define NDEBUG 
#endif
#endif
#include <assert.h>
#if defined(USEBCOPY)
#define memmove(d, s, c) bcopy(s, d, c)
#endif
#define ut8 unsigned char
