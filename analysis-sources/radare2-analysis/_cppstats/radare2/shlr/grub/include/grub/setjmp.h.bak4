

















#if !defined(GRUB_SETJMP_HEADER)
#define GRUB_SETJMP_HEADER 1

#if defined(GRUB_UTIL) && !defined(GRUBOF)
#include <setjmp.h>
typedef jmp_buf grub_jmp_buf;
#define grub_setjmp setjmp
#define grub_longjmp longjmp
#else


#include <grub/cpu/setjmp.h>
#endif

#endif 
