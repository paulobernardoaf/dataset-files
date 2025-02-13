#include "ruby.h"
#define init(n) {void Init_##n(VALUE klass); Init_##n(klass);}
static VALUE
sym_find(VALUE dummy, VALUE sym)
{
return rb_check_symbol(&sym);
}
static VALUE
sym_pinneddown_p(VALUE dummy, VALUE sym)
{
ID id = rb_check_id(&sym);
if (!id) return Qnil;
#if defined(ULL2NUM)
return ULL2NUM(id);
#else
return ULONG2NUM(id);
#endif
}
static VALUE
sym_iv_get(VALUE dummy, VALUE obj, VALUE name)
{
const char *n = StringValueCStr(name);
return rb_iv_get(obj, n);
}
void
Init_symbol(void)
{
VALUE mBug = rb_define_module("Bug");
VALUE klass = rb_define_class_under(mBug, "Symbol", rb_cSymbol);
rb_define_singleton_method(klass, "find", sym_find, 1);
rb_define_singleton_method(klass, "pinneddown?", sym_pinneddown_p, 1);
rb_define_singleton_method(klass, "iv_get", sym_iv_get, 2);
TEST_INIT_FUNCS(init);
}
