#include "ruby.h"
#if defined(__cplusplus)
extern "C" {
#endif
void Init_module_under_autoload_spec(void) {
VALUE specs = rb_const_get(rb_cObject, rb_intern("CApiModuleSpecs"));
rb_define_module_under(specs, "ModuleUnderAutoload");
rb_define_module_under(specs, "RubyUnderAutoload");
}
#if defined(__cplusplus)
}
#endif
