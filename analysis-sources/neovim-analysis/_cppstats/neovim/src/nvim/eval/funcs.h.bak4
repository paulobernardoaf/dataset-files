#if !defined(NVIM_EVAL_FUNCS_H)
#define NVIM_EVAL_FUNCS_H

#include "nvim/buffer_defs.h"
#include "nvim/eval/typval.h"

typedef void (*FunPtr)(void);


typedef void (*VimLFunc)(typval_T *args, typval_T *rvar, FunPtr data);


typedef struct fst {
char *name; 
uint8_t min_argc; 
uint8_t max_argc; 
VimLFunc func; 
FunPtr data; 
} VimLFuncDef;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "eval/funcs.h.generated.h"
#endif
#endif 
