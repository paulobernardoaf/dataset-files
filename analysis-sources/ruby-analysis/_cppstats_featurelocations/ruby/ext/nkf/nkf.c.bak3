








#define RUBY_NKF_REVISION "$Revision$"
#define RUBY_NKF_VERSION NKF_VERSION " (" NKF_RELEASE_DATE ")"

#include "ruby/ruby.h"
#include "ruby/encoding.h"




#undef getc
#undef ungetc
#define getc(f) (input_ctr>=i_len?-1:input[input_ctr++])
#define ungetc(c,f) input_ctr--

#define INCSIZE 32
#undef putchar
#undef TRUE
#undef FALSE
#define putchar(c) rb_nkf_putchar(c)



static unsigned char *output;
static unsigned char *input;
static int input_ctr;
static int i_len;
static int output_ctr;
static int o_len;
static int incsize;

static VALUE result;

static int
rb_nkf_putchar(unsigned int c)
{
if (output_ctr >= o_len) {
o_len += incsize;
rb_str_resize(result, o_len);
incsize *= 2;
output = (unsigned char *)RSTRING_PTR(result);
}
output[output_ctr++] = c;

return c;
}




#define PERL_XS 1
#include "nkf-utf8/config.h"
#include "nkf-utf8/utf8tbl.c"
#include "nkf-utf8/nkf.c"

rb_encoding* rb_nkf_enc_get(const char *name)
{
int idx = rb_enc_find_index(name);
if (idx < 0) {
nkf_encoding *nkf_enc = nkf_enc_find(name);
idx = rb_enc_find_index(nkf_enc_name(nkf_enc_to_base_encoding(nkf_enc)));
if (idx < 0) {
idx = rb_define_dummy_encoding(name);
}
}
return rb_enc_from_index(idx);
}

int nkf_split_options(const char *arg)
{
int count = 0;
unsigned char option[256];
int i = 0, j = 0;
int is_escaped = FALSE;
int is_single_quoted = FALSE;
int is_double_quoted = FALSE;
for(i = 0; arg[i]; i++){
if(j == 255){
return -1;
}else if(is_single_quoted){
if(arg[i] == '\''){
is_single_quoted = FALSE;
}else{
option[j++] = arg[i];
}
}else if(is_escaped){
is_escaped = FALSE;
option[j++] = arg[i];
}else if(arg[i] == '\\'){
is_escaped = TRUE;
}else if(is_double_quoted){
if(arg[i] == '"'){
is_double_quoted = FALSE;
}else{
option[j++] = arg[i];
}
}else if(arg[i] == '\''){
is_single_quoted = TRUE;
}else if(arg[i] == '"'){
is_double_quoted = TRUE;
}else if(arg[i] == ' '){
option[j] = '\0';
options(option);
j = 0;
}else{
option[j++] = arg[i];
}
}
if(j){
option[j] = '\0';
options(option);
}
return count;
}












static VALUE
rb_nkf_convert(VALUE obj, VALUE opt, VALUE src)
{
VALUE tmp;
reinit();
nkf_split_options(StringValueCStr(opt));
if (!output_encoding) rb_raise(rb_eArgError, "no output encoding given");

switch (nkf_enc_to_index(output_encoding)) {
case UTF_8_BOM: output_encoding = nkf_enc_from_index(UTF_8); break;
case UTF_16BE_BOM: output_encoding = nkf_enc_from_index(UTF_16BE); break;
case UTF_16LE_BOM: output_encoding = nkf_enc_from_index(UTF_16LE); break;
case UTF_32BE_BOM: output_encoding = nkf_enc_from_index(UTF_32BE); break;
case UTF_32LE_BOM: output_encoding = nkf_enc_from_index(UTF_32LE); break;
}
output_bom_f = FALSE;

incsize = INCSIZE;

input_ctr = 0;
input = (unsigned char *)StringValuePtr(src);
i_len = RSTRING_LENINT(src);
tmp = rb_str_new(0, i_len*3 + 10);

output_ctr = 0;
output = (unsigned char *)RSTRING_PTR(tmp);
o_len = RSTRING_LENINT(tmp);
*output = '\0';


result = tmp;
kanji_convert(NULL);
result = Qnil;


rb_str_set_len(tmp, output_ctr);

if (mimeout_f)
rb_enc_associate(tmp, rb_usascii_encoding());
else
rb_enc_associate(tmp, rb_nkf_enc_get(nkf_enc_name(output_encoding)));

return tmp;
}










static VALUE
rb_nkf_guess(VALUE obj, VALUE src)
{
reinit();

input_ctr = 0;
input = (unsigned char *)StringValuePtr(src);
i_len = RSTRING_LENINT(src);

guess_f = TRUE;
kanji_convert( NULL );
guess_f = FALSE;

return rb_enc_from_encoding(rb_nkf_enc_get(get_guessed_code()));
}
















































































































































































































































































void
Init_nkf(void)
{
VALUE mNKF = rb_define_module("NKF");

rb_define_module_function(mNKF, "nkf", rb_nkf_convert, 2);
rb_define_module_function(mNKF, "guess", rb_nkf_guess, 1);
rb_define_alias(rb_singleton_class(mNKF), "guess", "guess");

rb_define_const(mNKF, "AUTO", Qnil);
rb_define_const(mNKF, "NOCONV", Qnil);
rb_define_const(mNKF, "UNKNOWN", Qnil);
rb_define_const(mNKF, "BINARY", rb_enc_from_encoding(rb_nkf_enc_get("BINARY")));
rb_define_const(mNKF, "ASCII", rb_enc_from_encoding(rb_nkf_enc_get("US-ASCII")));
rb_define_const(mNKF, "JIS", rb_enc_from_encoding(rb_nkf_enc_get("ISO-2022-JP")));
rb_define_const(mNKF, "EUC", rb_enc_from_encoding(rb_nkf_enc_get("EUC-JP")));
rb_define_const(mNKF, "SJIS", rb_enc_from_encoding(rb_nkf_enc_get("Shift_JIS")));
rb_define_const(mNKF, "UTF8", rb_enc_from_encoding(rb_utf8_encoding()));
rb_define_const(mNKF, "UTF16", rb_enc_from_encoding(rb_nkf_enc_get("UTF-16BE")));
rb_define_const(mNKF, "UTF32", rb_enc_from_encoding(rb_nkf_enc_get("UTF-32BE")));


rb_define_const(mNKF, "VERSION", rb_str_new2(RUBY_NKF_VERSION));

rb_define_const(mNKF, "NKF_VERSION", rb_str_new2(NKF_VERSION));

rb_define_const(mNKF, "NKF_RELEASE_DATE", rb_str_new2(NKF_RELEASE_DATE));
}
