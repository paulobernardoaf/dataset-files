#include "ruby/ruby.h"
#include "id_table.h"
typedef enum {
CONST_DEPRECATED = 0x100,
CONST_VISIBILITY_MASK = 0xff,
CONST_PUBLIC = 0x00,
CONST_PRIVATE,
CONST_VISIBILITY_MAX
} rb_const_flag_t;
#define RB_CONST_PRIVATE_P(ce) (((ce)->flag & CONST_VISIBILITY_MASK) == CONST_PRIVATE)
#define RB_CONST_PUBLIC_P(ce) (((ce)->flag & CONST_VISIBILITY_MASK) == CONST_PUBLIC)
#define RB_CONST_DEPRECATED_P(ce) ((ce)->flag & CONST_DEPRECATED)
typedef struct rb_const_entry_struct {
rb_const_flag_t flag;
int line;
VALUE value; 
VALUE file; 
} rb_const_entry_t;
VALUE rb_mod_private_constant(int argc, const VALUE *argv, VALUE obj);
VALUE rb_mod_public_constant(int argc, const VALUE *argv, VALUE obj);
VALUE rb_mod_deprecate_constant(int argc, const VALUE *argv, VALUE obj);
void rb_free_const_table(struct rb_id_table *tbl);
VALUE rb_const_source_location(VALUE, ID);
MJIT_SYMBOL_EXPORT_BEGIN
int rb_autoloading_value(VALUE mod, ID id, VALUE *value, rb_const_flag_t *flag);
rb_const_entry_t *rb_const_lookup(VALUE klass, ID id);
VALUE rb_public_const_get_at(VALUE klass, ID id);
VALUE rb_public_const_get_from(VALUE klass, ID id);
int rb_public_const_defined_from(VALUE klass, ID id);
VALUE rb_const_source_location_at(VALUE, ID);
MJIT_SYMBOL_EXPORT_END
