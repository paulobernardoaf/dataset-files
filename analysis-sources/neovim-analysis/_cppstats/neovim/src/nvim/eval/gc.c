#include "nvim/eval/typval.h"
#include "nvim/eval/gc.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "eval/gc.c.generated.h"
#endif
dict_T *gc_first_dict = NULL;
list_T *gc_first_list = NULL;
