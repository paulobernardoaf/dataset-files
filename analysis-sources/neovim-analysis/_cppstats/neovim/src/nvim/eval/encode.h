#include <stddef.h>
#include <msgpack.h>
#include "nvim/eval.h"
#include "nvim/garray.h"
#include "nvim/vim.h" 
int encode_vim_to_msgpack(msgpack_packer *const packer,
typval_T *const tv,
const char *const objname);
int encode_vim_to_echo(garray_T *const packer,
typval_T *const tv,
const char *const objname);
typedef struct {
const list_T *const list; 
const listitem_T *li; 
size_t offset; 
size_t li_length; 
} ListReaderState;
static inline ListReaderState encode_init_lrstate(const list_T *const list)
FUNC_ATTR_NONNULL_ALL
{
return (ListReaderState) {
.list = list,
.li = tv_list_first(list),
.offset = 0,
.li_length = (TV_LIST_ITEM_TV(tv_list_first(list))->vval.v_string == NULL
? 0
: STRLEN(TV_LIST_ITEM_TV(
tv_list_first(list))->vval.v_string)),
};
}
extern const char *const encode_special_var_names[];
#define SURROGATE_HI_START 0xD800
#define SURROGATE_HI_END 0xDBFF
#define SURROGATE_LO_START 0xDC00
#define SURROGATE_LO_END 0xDFFF
#define SURROGATE_FIRST_CHAR 0x10000
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "eval/encode.h.generated.h"
#endif
