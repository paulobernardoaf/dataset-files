#if !defined(NVIM_SHADA_H)
#define NVIM_SHADA_H

#include <msgpack.h>


typedef enum {
kShaDaWantInfo = 1, 
kShaDaWantMarks = 2, 
kShaDaForceit = 4, 
kShaDaGetOldfiles = 8, 
kShaDaMissingError = 16, 
} ShaDaReadFileFlags;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "shada.h.generated.h"
#endif
#endif 
