#if !defined(INTERNAL_CLASS_H)
#define INTERNAL_CLASS_H









#include "id_table.h" 
#include "internal/gc.h" 
#include "internal/serial.h" 
#include "internal/stdbool.h" 
#include "ruby/intern.h" 
#include "ruby/ruby.h" 

#if defined(RClass)
#undef RClass 
#endif

#if defined(RCLASS_SUPER)
#undef RCLASS_SUPER
#endif

struct rb_deprecated_classext_struct {
char conflict[sizeof(VALUE) * 3];
};

struct rb_subclass_entry {
VALUE klass;
struct rb_subclass_entry *next;
};

struct rb_classext_struct {
struct st_table *iv_index_tbl;
struct st_table *iv_tbl;
#if SIZEOF_SERIAL_T == SIZEOF_VALUE 
struct rb_id_table *m_tbl;
#endif
struct rb_id_table *const_tbl;
struct rb_id_table *callable_m_tbl;
struct rb_id_table *cc_tbl; 
struct rb_subclass_entry *subclasses;
struct rb_subclass_entry **parent_subclasses;





struct rb_subclass_entry **module_subclasses;
#if SIZEOF_SERIAL_T != SIZEOF_VALUE 
rb_serial_t class_serial;
#endif
const VALUE origin_;
const VALUE refined_class;
rb_alloc_func_t allocator;
const VALUE includer;
};

struct RClass {
struct RBasic basic;
VALUE super;
struct rb_classext_struct *ptr;
#if SIZEOF_SERIAL_T == SIZEOF_VALUE

rb_serial_t class_serial;
#else

struct rb_id_table *m_tbl;
#endif
};

typedef struct rb_subclass_entry rb_subclass_entry_t;
typedef struct rb_classext_struct rb_classext_t;

#define RCLASS_EXT(c) (RCLASS(c)->ptr)
#define RCLASS_IV_TBL(c) (RCLASS_EXT(c)->iv_tbl)
#define RCLASS_CONST_TBL(c) (RCLASS_EXT(c)->const_tbl)
#if SIZEOF_SERIAL_T == SIZEOF_VALUE
#define RCLASS_M_TBL(c) (RCLASS_EXT(c)->m_tbl)
#else
#define RCLASS_M_TBL(c) (RCLASS(c)->m_tbl)
#endif
#define RCLASS_CALLABLE_M_TBL(c) (RCLASS_EXT(c)->callable_m_tbl)
#define RCLASS_CC_TBL(c) (RCLASS_EXT(c)->cc_tbl)
#define RCLASS_IV_INDEX_TBL(c) (RCLASS_EXT(c)->iv_index_tbl)
#define RCLASS_ORIGIN(c) (RCLASS_EXT(c)->origin_)
#define RCLASS_REFINED_CLASS(c) (RCLASS_EXT(c)->refined_class)
#if SIZEOF_SERIAL_T == SIZEOF_VALUE
#define RCLASS_SERIAL(c) (RCLASS(c)->class_serial)
#else
#define RCLASS_SERIAL(c) (RCLASS_EXT(c)->class_serial)
#endif
#define RCLASS_INCLUDER(c) (RCLASS_EXT(c)->includer)

#define RCLASS_CLONED FL_USER6
#define RICLASS_IS_ORIGIN FL_USER5
#define RCLASS_REFINED_BY_ANY FL_USER7


void rb_class_subclass_add(VALUE super, VALUE klass);
void rb_class_remove_from_super_subclasses(VALUE);
int rb_singleton_class_internal_p(VALUE sklass);
VALUE rb_class_boot(VALUE);
VALUE rb_make_metaclass(VALUE, VALUE);
VALUE rb_include_class_new(VALUE, VALUE);
void rb_class_foreach_subclass(VALUE klass, void (*f)(VALUE, VALUE), VALUE);
void rb_class_detach_subclasses(VALUE);
void rb_class_detach_module_subclasses(VALUE);
void rb_class_remove_from_module_subclasses(VALUE);
VALUE rb_obj_methods(int argc, const VALUE *argv, VALUE obj);
VALUE rb_obj_protected_methods(int argc, const VALUE *argv, VALUE obj);
VALUE rb_obj_private_methods(int argc, const VALUE *argv, VALUE obj);
VALUE rb_obj_public_methods(int argc, const VALUE *argv, VALUE obj);
VALUE rb_special_singleton_class(VALUE);
VALUE rb_singleton_class_clone_and_attach(VALUE obj, VALUE attach);
VALUE rb_singleton_class_get(VALUE obj);
int rb_class_has_methods(VALUE c);
void rb_undef_methods_from(VALUE klass, VALUE super);
static inline void RCLASS_SET_ORIGIN(VALUE klass, VALUE origin);
static inline VALUE RCLASS_SUPER(VALUE klass);
static inline VALUE RCLASS_SET_SUPER(VALUE klass, VALUE super);
static inline void RCLASS_SET_INCLUDER(VALUE iclass, VALUE klass);

MJIT_SYMBOL_EXPORT_BEGIN
VALUE rb_class_inherited(VALUE, VALUE);
VALUE rb_keyword_error_new(const char *, VALUE);
MJIT_SYMBOL_EXPORT_END

static inline void
RCLASS_SET_ORIGIN(VALUE klass, VALUE origin)
{
RB_OBJ_WRITE(klass, &RCLASS_ORIGIN(klass), origin);
if (klass != origin) FL_SET(origin, RICLASS_IS_ORIGIN);
}

static inline void
RCLASS_SET_INCLUDER(VALUE iclass, VALUE klass)
{
RB_OBJ_WRITE(iclass, &RCLASS_INCLUDER(iclass), klass);
}

static inline VALUE
RCLASS_SUPER(VALUE klass)
{
return RCLASS(klass)->super;
}

static inline VALUE
RCLASS_SET_SUPER(VALUE klass, VALUE super)
{
if (super) {
rb_class_remove_from_super_subclasses(klass);
rb_class_subclass_add(super, klass);
}
RB_OBJ_WRITE(klass, &RCLASS(klass)->super, super);
return super;
}

#endif 
