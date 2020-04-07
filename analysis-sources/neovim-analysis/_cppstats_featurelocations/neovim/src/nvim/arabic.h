#if !defined(NVIM_ARABIC_H)
#define NVIM_ARABIC_H

#include <stdbool.h>


static inline bool arabic_char(int c)
{

return c >= 0x0621 && c <= 0x0670;
}

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "arabic.h.generated.h"
#endif
#endif 
