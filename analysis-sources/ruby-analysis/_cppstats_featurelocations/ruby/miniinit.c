










#include "ruby/ruby.h"
#include "ruby/encoding.h"


const char ruby_exec_prefix[] = "";
const char ruby_initial_load_paths[] = "";


VALUE
rb_locale_charmap(VALUE klass)
{

return Qnil;
}

int
rb_locale_charmap_index(void)
{
return -1;
}

int
Init_enc_set_filesystem_encoding(void)
{
return rb_enc_to_index(rb_default_external_encoding());
}

void rb_encdb_declare(const char *name);
int rb_encdb_alias(const char *alias, const char *orig);
void
Init_enc(void)
{
rb_encdb_declare("ASCII-8BIT");
rb_encdb_declare("US-ASCII");
rb_encdb_declare("UTF-8");
rb_encdb_alias("BINARY", "ASCII-8BIT");
rb_encdb_alias("ASCII", "US-ASCII");
}

#include "mini_builtin.c"
