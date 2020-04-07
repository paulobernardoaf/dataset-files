#include "ruby/config.h"
#if defined(__cplusplus)
#error not for C++
#endif
#define LIKELY(x) RB_LIKELY(x)
#define UNLIKELY(x) RB_UNLIKELY(x)
#define numberof(array) ((int)(sizeof(array) / sizeof((array)[0])))
#define roomof(x, y) (((x) + (y) - 1) / (y))
#define type_roomof(x, y) roomof(sizeof(x), sizeof(y))
#define ACCESS_ONCE(type,x) (*((volatile type *)&(x)))
#include "ruby/ruby.h"
#undef Check_Type
#undef RClass
#undef RCLASS_SUPER
#undef NEWOBJ_OF
#undef RB_NEWOBJ_OF
#undef RB_OBJ_WRITE
#undef RHASH_IFNONE
#undef RHASH_SIZE
#undef RSTRUCT_LEN
#undef RSTRUCT_PTR
#undef RSTRUCT_SET
#undef RSTRUCT_GET
#define rb_ary_new_from_args(...) rb_nonexistent_symbol(__VA_ARGS__)
#define rb_io_fptr_finalize(...) rb_nonexistent_symbol(__VA_ARGS__)
#define rb_fstring_cstr(...) rb_nonexistent_symbol(__VA_ARGS__)
#define rb_sym_intern_ascii_cstr(...) rb_nonexistent_symbol(__VA_ARGS__)
#define rb_funcallv(...) rb_nonexistent_symbol(__VA_ARGS__)
#define rb_method_basic_definition_p(...) rb_nonexistent_symbol(__VA_ARGS__)
void rb_obj_info_dump(VALUE obj);
void rb_obj_info_dump_loc(VALUE obj, const char *file, int line, const char *func);
void ruby_debug_breakpoint(void);
PRINTF_ARGS(void ruby_debug_printf(const char*, ...), 1, 2);
#define rp(obj) rb_obj_info_dump_loc((VALUE)(obj), __FILE__, __LINE__, RUBY_FUNCTION_NAME_STRING)
#define rp_m(msg, obj) do { fprintf(stderr, "%s", (msg)); rb_obj_info_dump((VALUE)obj); } while (0)
#define bp() ruby_debug_breakpoint()
