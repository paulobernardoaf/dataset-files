#include "nvim/types.h"
#include "nvim/pos.h"
#include "nvim/buffer_defs.h"
#include "nvim/eval/typval.h"
#include "nvim/option_defs.h"
#define CH_FOLD(c) utf_fold((sizeof(c) == sizeof(char)) ?((int)(uint8_t)(c)) :((int)(c)))
typedef enum {
STR2NR_DEC = 0,
STR2NR_BIN = (1 << 0), 
STR2NR_OCT = (1 << 1), 
STR2NR_HEX = (1 << 2), 
STR2NR_FORCE = (1 << 3),
STR2NR_ALL = STR2NR_BIN | STR2NR_OCT | STR2NR_HEX,
} ChStr2NrFlags;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "charset.h.generated.h"
#endif
static inline bool vim_isbreak(int c)
REAL_FATTR_CONST
REAL_FATTR_ALWAYS_INLINE;
static inline bool vim_isbreak(int c)
{
return breakat_flags[(char_u)c];
}
