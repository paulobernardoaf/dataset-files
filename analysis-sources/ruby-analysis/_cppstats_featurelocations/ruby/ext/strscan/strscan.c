









#include "ruby/ruby.h"
#include "ruby/re.h"
#include "ruby/encoding.h"

#if defined(RUBY_EXTCONF_H)
#include RUBY_EXTCONF_H
#endif

#if defined(HAVE_ONIG_REGION_MEMSIZE)
extern size_t onig_region_memsize(const struct re_registers *regs);
#endif

#include <stdbool.h>

#define STRSCAN_VERSION "1.0.3"





static VALUE StringScanner;
static VALUE ScanError;
static ID id_byteslice;

struct strscanner
{

unsigned long flags;
#define FLAG_MATCHED (1 << 0)


VALUE str;


long prev; 
long curr; 


struct re_registers regs;


VALUE regex;


bool fixed_anchor_p;
};

#define MATCHED_P(s) ((s)->flags & FLAG_MATCHED)
#define MATCHED(s) (s)->flags |= FLAG_MATCHED
#define CLEAR_MATCH_STATUS(s) (s)->flags &= ~FLAG_MATCHED

#define S_PBEG(s) (RSTRING_PTR((s)->str))
#define S_LEN(s) (RSTRING_LEN((s)->str))
#define S_PEND(s) (S_PBEG(s) + S_LEN(s))
#define CURPTR(s) (S_PBEG(s) + (s)->curr)
#define S_RESTLEN(s) (S_LEN(s) - (s)->curr)

#define EOS_P(s) ((s)->curr >= RSTRING_LEN(p->str))

#define GET_SCANNER(obj,var) do {(var) = check_strscan(obj);if (NIL_P((var)->str)) rb_raise(rb_eArgError, "uninitialized StringScanner object");} while (0)








static inline long minl _((const long n, const long x));
static VALUE extract_range _((struct strscanner *p, long beg_i, long end_i));
static VALUE extract_beg_len _((struct strscanner *p, long beg_i, long len));

static struct strscanner *check_strscan _((VALUE obj));
static void strscan_mark _((void *p));
static void strscan_free _((void *p));
static size_t strscan_memsize _((const void *p));
static VALUE strscan_s_allocate _((VALUE klass));
static VALUE strscan_initialize _((int argc, VALUE *argv, VALUE self));
static VALUE strscan_init_copy _((VALUE vself, VALUE vorig));

static VALUE strscan_s_mustc _((VALUE self));
static VALUE strscan_terminate _((VALUE self));
static VALUE strscan_clear _((VALUE self));
static VALUE strscan_get_string _((VALUE self));
static VALUE strscan_set_string _((VALUE self, VALUE str));
static VALUE strscan_concat _((VALUE self, VALUE str));
static VALUE strscan_get_pos _((VALUE self));
static VALUE strscan_set_pos _((VALUE self, VALUE pos));
static VALUE strscan_do_scan _((VALUE self, VALUE regex,
int succptr, int getstr, int headonly));
static VALUE strscan_scan _((VALUE self, VALUE re));
static VALUE strscan_match_p _((VALUE self, VALUE re));
static VALUE strscan_skip _((VALUE self, VALUE re));
static VALUE strscan_check _((VALUE self, VALUE re));
static VALUE strscan_scan_full _((VALUE self, VALUE re,
VALUE succp, VALUE getp));
static VALUE strscan_scan_until _((VALUE self, VALUE re));
static VALUE strscan_skip_until _((VALUE self, VALUE re));
static VALUE strscan_check_until _((VALUE self, VALUE re));
static VALUE strscan_search_full _((VALUE self, VALUE re,
VALUE succp, VALUE getp));
static void adjust_registers_to_matched _((struct strscanner *p));
static VALUE strscan_getch _((VALUE self));
static VALUE strscan_get_byte _((VALUE self));
static VALUE strscan_getbyte _((VALUE self));
static VALUE strscan_peek _((VALUE self, VALUE len));
static VALUE strscan_peep _((VALUE self, VALUE len));
static VALUE strscan_unscan _((VALUE self));
static VALUE strscan_bol_p _((VALUE self));
static VALUE strscan_eos_p _((VALUE self));
static VALUE strscan_empty_p _((VALUE self));
static VALUE strscan_rest_p _((VALUE self));
static VALUE strscan_matched_p _((VALUE self));
static VALUE strscan_matched _((VALUE self));
static VALUE strscan_matched_size _((VALUE self));
static VALUE strscan_aref _((VALUE self, VALUE idx));
static VALUE strscan_pre_match _((VALUE self));
static VALUE strscan_post_match _((VALUE self));
static VALUE strscan_rest _((VALUE self));
static VALUE strscan_rest_size _((VALUE self));

static VALUE strscan_inspect _((VALUE self));
static VALUE inspect1 _((struct strscanner *p));
static VALUE inspect2 _((struct strscanner *p));





static VALUE
str_new(struct strscanner *p, const char *ptr, long len)
{
VALUE str = rb_str_new(ptr, len);
rb_enc_copy(str, p->str);
return str;
}

static inline long
minl(const long x, const long y)
{
return (x < y) ? x : y;
}

static VALUE
extract_range(struct strscanner *p, long beg_i, long end_i)
{
if (beg_i > S_LEN(p)) return Qnil;
end_i = minl(end_i, S_LEN(p));
return str_new(p, S_PBEG(p) + beg_i, end_i - beg_i);
}

static VALUE
extract_beg_len(struct strscanner *p, long beg_i, long len)
{
if (beg_i > S_LEN(p)) return Qnil;
len = minl(len, S_LEN(p) - beg_i);
return str_new(p, S_PBEG(p) + beg_i, len);
}





static void
strscan_mark(void *ptr)
{
struct strscanner *p = ptr;
rb_gc_mark(p->str);
}

static void
strscan_free(void *ptr)
{
struct strscanner *p = ptr;
onig_region_free(&(p->regs), 0);
ruby_xfree(p);
}

static size_t
strscan_memsize(const void *ptr)
{
const struct strscanner *p = ptr;
size_t size = sizeof(*p) - sizeof(p->regs);
#if defined(HAVE_ONIG_REGION_MEMSIZE)
size += onig_region_memsize(&p->regs);
#endif
return size;
}

static const rb_data_type_t strscanner_type = {
"StringScanner",
{strscan_mark, strscan_free, strscan_memsize},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE
strscan_s_allocate(VALUE klass)
{
struct strscanner *p;
VALUE obj = TypedData_Make_Struct(klass, struct strscanner, &strscanner_type, p);

CLEAR_MATCH_STATUS(p);
onig_region_init(&(p->regs));
p->str = Qnil;
return obj;
}













static VALUE
strscan_initialize(int argc, VALUE *argv, VALUE self)
{
struct strscanner *p;
VALUE str, options;

p = check_strscan(self);
rb_scan_args(argc, argv, "11", &str, &options);
options = rb_check_hash_type(options);
if (!NIL_P(options)) {
VALUE fixed_anchor;
ID keyword_ids[1];
keyword_ids[0] = rb_intern("fixed_anchor");
rb_get_kwargs(options, keyword_ids, 0, 1, &fixed_anchor);
if (fixed_anchor == Qundef) {
p->fixed_anchor_p = false;
}
else {
p->fixed_anchor_p = RTEST(fixed_anchor);
}
}
else {
p->fixed_anchor_p = false;
}
StringValue(str);
p->str = str;

return self;
}

static struct strscanner *
check_strscan(VALUE obj)
{
return rb_check_typeddata(obj, &strscanner_type);
}








static VALUE
strscan_init_copy(VALUE vself, VALUE vorig)
{
struct strscanner *self, *orig;

self = check_strscan(vself);
orig = check_strscan(vorig);
if (self != orig) {
self->flags = orig->flags;
self->str = orig->str;
self->prev = orig->prev;
self->curr = orig->curr;
if (rb_reg_region_copy(&self->regs, &orig->regs))
rb_memerror();
RB_GC_GUARD(vorig);
}

return vself;
}










static VALUE
strscan_s_mustc(VALUE self)
{
return self;
}




static VALUE
strscan_reset(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
p->curr = 0;
CLEAR_MATCH_STATUS(p);
return self;
}








static VALUE
strscan_terminate(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
p->curr = S_LEN(p);
CLEAR_MATCH_STATUS(p);
return self;
}





static VALUE
strscan_clear(VALUE self)
{
rb_warning("StringScanner#clear is obsolete; use #terminate instead");
return strscan_terminate(self);
}




static VALUE
strscan_get_string(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
return p->str;
}







static VALUE
strscan_set_string(VALUE self, VALUE str)
{
struct strscanner *p = check_strscan(self);

StringValue(str);
p->str = str;
p->curr = 0;
CLEAR_MATCH_STATUS(p);
return str;
}















static VALUE
strscan_concat(VALUE self, VALUE str)
{
struct strscanner *p;

GET_SCANNER(self, p);
StringValue(str);
rb_str_append(p->str, str);
return self;
}















static VALUE
strscan_get_pos(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
return INT2FIX(p->curr);
}














static VALUE
strscan_get_charpos(VALUE self)
{
struct strscanner *p;
VALUE substr;

GET_SCANNER(self, p);

substr = rb_funcall(p->str, id_byteslice, 2, INT2FIX(0), INT2NUM(p->curr));

return rb_str_length(substr);
}










static VALUE
strscan_set_pos(VALUE self, VALUE v)
{
struct strscanner *p;
long i;

GET_SCANNER(self, p);
i = NUM2INT(v);
if (i < 0) i += S_LEN(p);
if (i < 0) rb_raise(rb_eRangeError, "index out of range");
if (i > S_LEN(p)) rb_raise(rb_eRangeError, "index out of range");
p->curr = i;
return INT2NUM(i);
}

static inline UChar *
match_target(struct strscanner *p)
{
if (p->fixed_anchor_p) {
return (UChar *)S_PBEG(p);
}
else
{
return (UChar *)CURPTR(p);
}
}

static inline void
set_registers(struct strscanner *p, size_t length)
{
const int at = 0;
OnigRegion *regs = &(p->regs);
onig_region_clear(regs);
if (onig_region_set(regs, at, 0, 0)) return;
if (p->fixed_anchor_p) {
regs->beg[at] = p->curr;
regs->end[at] = p->curr + length;
}
else
{
regs->end[at] = length;
}
}

static inline void
succ(struct strscanner *p)
{
if (p->fixed_anchor_p) {
p->curr = p->regs.end[0];
}
else
{
p->curr += p->regs.end[0];
}
}

static inline long
last_match_length(struct strscanner *p)
{
if (p->fixed_anchor_p) {
return p->regs.end[0] - p->prev;
}
else
{
return p->regs.end[0];
}
}

static inline long
adjust_register_position(struct strscanner *p, long position)
{
if (p->fixed_anchor_p) {
return position;
}
else {
return p->prev + position;
}
}

static VALUE
strscan_do_scan(VALUE self, VALUE pattern, int succptr, int getstr, int headonly)
{
struct strscanner *p;

if (headonly) {
if (!RB_TYPE_P(pattern, T_REGEXP)) {
StringValue(pattern);
}
}
else {
Check_Type(pattern, T_REGEXP);
}
GET_SCANNER(self, p);

CLEAR_MATCH_STATUS(p);
if (S_RESTLEN(p) < 0) {
return Qnil;
}

if (RB_TYPE_P(pattern, T_REGEXP)) {
regex_t *rb_reg_prepare_re(VALUE re, VALUE str);
regex_t *re;
long ret;
int tmpreg;

p->regex = pattern;
re = rb_reg_prepare_re(pattern, p->str);
tmpreg = re != RREGEXP_PTR(pattern);
if (!tmpreg) RREGEXP(pattern)->usecnt++;

if (headonly) {
ret = onig_match(re,
match_target(p),
(UChar* )(CURPTR(p) + S_RESTLEN(p)),
(UChar* )CURPTR(p),
&(p->regs),
ONIG_OPTION_NONE);
}
else {
ret = onig_search(re,
match_target(p),
(UChar* )(CURPTR(p) + S_RESTLEN(p)),
(UChar* )CURPTR(p),
(UChar* )(CURPTR(p) + S_RESTLEN(p)),
&(p->regs),
ONIG_OPTION_NONE);
}
if (!tmpreg) RREGEXP(pattern)->usecnt--;
if (tmpreg) {
if (RREGEXP(pattern)->usecnt) {
onig_free(re);
}
else {
onig_free(RREGEXP_PTR(pattern));
RREGEXP_PTR(pattern) = re;
}
}

if (ret == -2) rb_raise(ScanError, "regexp buffer overflow");
if (ret < 0) {

return Qnil;
}
}
else {
rb_enc_check(p->str, pattern);
if (S_RESTLEN(p) < RSTRING_LEN(pattern)) {
return Qnil;
}
if (memcmp(CURPTR(p), RSTRING_PTR(pattern), RSTRING_LEN(pattern)) != 0) {
return Qnil;
}
set_registers(p, RSTRING_LEN(pattern));
}

MATCHED(p);
p->prev = p->curr;

if (succptr) {
succ(p);
}
{
const long length = last_match_length(p);
if (getstr) {
return extract_beg_len(p, p->prev, length);
}
else {
return INT2FIX(length);
}
}
}

















static VALUE
strscan_scan(VALUE self, VALUE re)
{
return strscan_do_scan(self, re, 1, 1, 1);
}













static VALUE
strscan_match_p(VALUE self, VALUE re)
{
return strscan_do_scan(self, re, 0, 0, 1);
}



















static VALUE
strscan_skip(VALUE self, VALUE re)
{
return strscan_do_scan(self, re, 1, 0, 1);
}
















static VALUE
strscan_check(VALUE self, VALUE re)
{
return strscan_do_scan(self, re, 0, 1, 1);
}











static VALUE
strscan_scan_full(VALUE self, VALUE re, VALUE s, VALUE f)
{
return strscan_do_scan(self, re, RTEST(s), RTEST(f), 1);
}













static VALUE
strscan_scan_until(VALUE self, VALUE re)
{
return strscan_do_scan(self, re, 1, 1, 0);
}














static VALUE
strscan_exist_p(VALUE self, VALUE re)
{
return strscan_do_scan(self, re, 0, 0, 0);
}

















static VALUE
strscan_skip_until(VALUE self, VALUE re)
{
return strscan_do_scan(self, re, 1, 0, 0);
}














static VALUE
strscan_check_until(VALUE self, VALUE re)
{
return strscan_do_scan(self, re, 0, 1, 0);
}










static VALUE
strscan_search_full(VALUE self, VALUE re, VALUE s, VALUE f)
{
return strscan_do_scan(self, re, RTEST(s), RTEST(f), 0);
}

static void
adjust_registers_to_matched(struct strscanner *p)
{
onig_region_clear(&(p->regs));
if (p->fixed_anchor_p) {
onig_region_set(&(p->regs), 0, (int)p->prev, (int)p->curr);
}
else {
onig_region_set(&(p->regs), 0, 0, (int)(p->curr - p->prev));
}
}















static VALUE
strscan_getch(VALUE self)
{
struct strscanner *p;
long len;

GET_SCANNER(self, p);
CLEAR_MATCH_STATUS(p);
if (EOS_P(p))
return Qnil;

len = rb_enc_mbclen(CURPTR(p), S_PEND(p), rb_enc_get(p->str));
len = minl(len, S_RESTLEN(p));
p->prev = p->curr;
p->curr += len;
MATCHED(p);
adjust_registers_to_matched(p);
return extract_range(p,
adjust_register_position(p, p->regs.beg[0]),
adjust_register_position(p, p->regs.end[0]));
}

















static VALUE
strscan_get_byte(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
CLEAR_MATCH_STATUS(p);
if (EOS_P(p))
return Qnil;

p->prev = p->curr;
p->curr++;
MATCHED(p);
adjust_registers_to_matched(p);
return extract_range(p,
adjust_register_position(p, p->regs.beg[0]),
adjust_register_position(p, p->regs.end[0]));
}





static VALUE
strscan_getbyte(VALUE self)
{
rb_warning("StringScanner#getbyte is obsolete; use #get_byte instead");
return strscan_get_byte(self);
}












static VALUE
strscan_peek(VALUE self, VALUE vlen)
{
struct strscanner *p;
long len;

GET_SCANNER(self, p);

len = NUM2LONG(vlen);
if (EOS_P(p))
return str_new(p, "", 0);

len = minl(len, S_RESTLEN(p));
return extract_beg_len(p, p->curr, len);
}





static VALUE
strscan_peep(VALUE self, VALUE vlen)
{
rb_warning("StringScanner#peep is obsolete; use #peek instead");
return strscan_peek(self, vlen);
}












static VALUE
strscan_unscan(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
if (! MATCHED_P(p))
rb_raise(ScanError, "unscan failed: previous match record not exist");
p->curr = p->prev;
CLEAR_MATCH_STATUS(p);
return self;
}













static VALUE
strscan_bol_p(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
if (CURPTR(p) > S_PEND(p)) return Qnil;
if (p->curr == 0) return Qtrue;
return (*(CURPTR(p) - 1) == '\n') ? Qtrue : Qfalse;
}











static VALUE
strscan_eos_p(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
return EOS_P(p) ? Qtrue : Qfalse;
}





static VALUE
strscan_empty_p(VALUE self)
{
rb_warning("StringScanner#empty? is obsolete; use #eos? instead");
return strscan_eos_p(self);
}









static VALUE
strscan_rest_p(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
return EOS_P(p) ? Qfalse : Qtrue;
}










static VALUE
strscan_matched_p(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
return MATCHED_P(p) ? Qtrue : Qfalse;
}








static VALUE
strscan_matched(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
if (! MATCHED_P(p)) return Qnil;
return extract_range(p,
adjust_register_position(p, p->regs.beg[0]),
adjust_register_position(p, p->regs.end[0]));
}











static VALUE
strscan_matched_size(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
if (! MATCHED_P(p)) return Qnil;
return INT2NUM(p->regs.end[0] - p->regs.beg[0]);
}

static int
name_to_backref_number(struct re_registers *regs, VALUE regexp, const char* name, const char* name_end, rb_encoding *enc)
{
int num;

num = onig_name_to_backref_number(RREGEXP_PTR(regexp),
(const unsigned char* )name, (const unsigned char* )name_end, regs);
if (num >= 1) {
return num;
}
else {
rb_enc_raise(enc, rb_eIndexError, "undefined group name reference: %.*s",
rb_long2int(name_end - name), name);
}

UNREACHABLE;
}



























static VALUE
strscan_aref(VALUE self, VALUE idx)
{
const char *name;
struct strscanner *p;
long i;

GET_SCANNER(self, p);
if (! MATCHED_P(p)) return Qnil;

switch (TYPE(idx)) {
case T_SYMBOL:
idx = rb_sym2str(idx);

case T_STRING:
if (!p->regex) return Qnil;
RSTRING_GETMEM(idx, name, i);
i = name_to_backref_number(&(p->regs), p->regex, name, name + i, rb_enc_get(idx));
break;
default:
i = NUM2LONG(idx);
}

if (i < 0)
i += p->regs.num_regs;
if (i < 0) return Qnil;
if (i >= p->regs.num_regs) return Qnil;
if (p->regs.beg[i] == -1) return Qnil;

return extract_range(p,
adjust_register_position(p, p->regs.beg[i]),
adjust_register_position(p, p->regs.end[i]));
}











static VALUE
strscan_size(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
if (! MATCHED_P(p)) return Qnil;
return INT2FIX(p->regs.num_regs);
}













static VALUE
strscan_captures(VALUE self)
{
struct strscanner *p;
int i, num_regs;
VALUE new_ary;

GET_SCANNER(self, p);
if (! MATCHED_P(p)) return Qnil;

num_regs = p->regs.num_regs;
new_ary = rb_ary_new2(num_regs);

for (i = 1; i < num_regs; i++) {
VALUE str = extract_range(p,
adjust_register_position(p, p->regs.beg[i]),
adjust_register_position(p, p->regs.end[i]));
rb_ary_push(new_ary, str);
}

return new_ary;
}















static VALUE
strscan_values_at(int argc, VALUE *argv, VALUE self)
{
struct strscanner *p;
long i;
VALUE new_ary;

GET_SCANNER(self, p);
if (! MATCHED_P(p)) return Qnil;

new_ary = rb_ary_new2(argc);
for (i = 0; i<argc; i++) {
rb_ary_push(new_ary, strscan_aref(self, argv[i]));
}

return new_ary;
}










static VALUE
strscan_pre_match(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
if (! MATCHED_P(p)) return Qnil;
return extract_range(p,
0,
adjust_register_position(p, p->regs.beg[0]));
}










static VALUE
strscan_post_match(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
if (! MATCHED_P(p)) return Qnil;
return extract_range(p,
adjust_register_position(p, p->regs.end[0]),
S_LEN(p));
}





static VALUE
strscan_rest(VALUE self)
{
struct strscanner *p;

GET_SCANNER(self, p);
if (EOS_P(p)) {
return str_new(p, "", 0);
}
return extract_range(p, p->curr, S_LEN(p));
}




static VALUE
strscan_rest_size(VALUE self)
{
struct strscanner *p;
long i;

GET_SCANNER(self, p);
if (EOS_P(p)) {
return INT2FIX(0);
}
i = S_RESTLEN(p);
return INT2FIX(i);
}





static VALUE
strscan_restsize(VALUE self)
{
rb_warning("StringScanner#restsize is obsolete; use #rest_size instead");
return strscan_rest_size(self);
}

#define INSPECT_LENGTH 5












static VALUE
strscan_inspect(VALUE self)
{
struct strscanner *p;
VALUE a, b;

p = check_strscan(self);
if (NIL_P(p->str)) {
a = rb_sprintf("#<%"PRIsVALUE" (uninitialized)>", rb_obj_class(self));
return a;
}
if (EOS_P(p)) {
a = rb_sprintf("#<%"PRIsVALUE" fin>", rb_obj_class(self));
return a;
}
if (p->curr == 0) {
b = inspect2(p);
a = rb_sprintf("#<%"PRIsVALUE" %ld/%ld @ %"PRIsVALUE">",
rb_obj_class(self),
p->curr, S_LEN(p),
b);
return a;
}
a = inspect1(p);
b = inspect2(p);
a = rb_sprintf("#<%"PRIsVALUE" %ld/%ld %"PRIsVALUE" @ %"PRIsVALUE">",
rb_obj_class(self),
p->curr, S_LEN(p),
a, b);
return a;
}

static VALUE
inspect1(struct strscanner *p)
{
VALUE str;
long len;

if (p->curr == 0) return rb_str_new2("");
if (p->curr > INSPECT_LENGTH) {
str = rb_str_new_cstr("...");
len = INSPECT_LENGTH;
}
else {
str = rb_str_new(0, 0);
len = p->curr;
}
rb_str_cat(str, CURPTR(p) - len, len);
return rb_str_dump(str);
}

static VALUE
inspect2(struct strscanner *p)
{
VALUE str;
long len;

if (EOS_P(p)) return rb_str_new2("");
len = S_RESTLEN(p);
if (len > INSPECT_LENGTH) {
str = rb_str_new(CURPTR(p), INSPECT_LENGTH);
rb_str_cat2(str, "...");
}
else {
str = rb_str_new(CURPTR(p), len);
}
return rb_str_dump(str);
}










static VALUE
strscan_fixed_anchor_p(VALUE self)
{
struct strscanner *p;
p = check_strscan(self);
return p->fixed_anchor_p ? Qtrue : Qfalse;
}












































































































void
Init_strscan(void)
{
#undef rb_intern
ID id_scanerr = rb_intern("ScanError");
VALUE tmp;

id_byteslice = rb_intern("byteslice");

StringScanner = rb_define_class("StringScanner", rb_cObject);
ScanError = rb_define_class_under(StringScanner, "Error", rb_eStandardError);
if (!rb_const_defined(rb_cObject, id_scanerr)) {
rb_const_set(rb_cObject, id_scanerr, ScanError);
}
tmp = rb_str_new2(STRSCAN_VERSION);
rb_obj_freeze(tmp);
rb_const_set(StringScanner, rb_intern("Version"), tmp);
tmp = rb_str_new2("$Id$");
rb_obj_freeze(tmp);
rb_const_set(StringScanner, rb_intern("Id"), tmp);

rb_define_alloc_func(StringScanner, strscan_s_allocate);
rb_define_private_method(StringScanner, "initialize", strscan_initialize, -1);
rb_define_private_method(StringScanner, "initialize_copy", strscan_init_copy, 1);
rb_define_singleton_method(StringScanner, "must_C_version", strscan_s_mustc, 0);
rb_define_method(StringScanner, "reset", strscan_reset, 0);
rb_define_method(StringScanner, "terminate", strscan_terminate, 0);
rb_define_method(StringScanner, "clear", strscan_clear, 0);
rb_define_method(StringScanner, "string", strscan_get_string, 0);
rb_define_method(StringScanner, "string=", strscan_set_string, 1);
rb_define_method(StringScanner, "concat", strscan_concat, 1);
rb_define_method(StringScanner, "<<", strscan_concat, 1);
rb_define_method(StringScanner, "pos", strscan_get_pos, 0);
rb_define_method(StringScanner, "pos=", strscan_set_pos, 1);
rb_define_method(StringScanner, "charpos", strscan_get_charpos, 0);
rb_define_method(StringScanner, "pointer", strscan_get_pos, 0);
rb_define_method(StringScanner, "pointer=", strscan_set_pos, 1);

rb_define_method(StringScanner, "scan", strscan_scan, 1);
rb_define_method(StringScanner, "skip", strscan_skip, 1);
rb_define_method(StringScanner, "match?", strscan_match_p, 1);
rb_define_method(StringScanner, "check", strscan_check, 1);
rb_define_method(StringScanner, "scan_full", strscan_scan_full, 3);

rb_define_method(StringScanner, "scan_until", strscan_scan_until, 1);
rb_define_method(StringScanner, "skip_until", strscan_skip_until, 1);
rb_define_method(StringScanner, "exist?", strscan_exist_p, 1);
rb_define_method(StringScanner, "check_until", strscan_check_until, 1);
rb_define_method(StringScanner, "search_full", strscan_search_full, 3);

rb_define_method(StringScanner, "getch", strscan_getch, 0);
rb_define_method(StringScanner, "get_byte", strscan_get_byte, 0);
rb_define_method(StringScanner, "getbyte", strscan_getbyte, 0);
rb_define_method(StringScanner, "peek", strscan_peek, 1);
rb_define_method(StringScanner, "peep", strscan_peep, 1);

rb_define_method(StringScanner, "unscan", strscan_unscan, 0);

rb_define_method(StringScanner, "beginning_of_line?", strscan_bol_p, 0);
rb_alias(StringScanner, rb_intern("bol?"), rb_intern("beginning_of_line?"));
rb_define_method(StringScanner, "eos?", strscan_eos_p, 0);
rb_define_method(StringScanner, "empty?", strscan_empty_p, 0);
rb_define_method(StringScanner, "rest?", strscan_rest_p, 0);

rb_define_method(StringScanner, "matched?", strscan_matched_p, 0);
rb_define_method(StringScanner, "matched", strscan_matched, 0);
rb_define_method(StringScanner, "matched_size", strscan_matched_size, 0);
rb_define_method(StringScanner, "[]", strscan_aref, 1);
rb_define_method(StringScanner, "pre_match", strscan_pre_match, 0);
rb_define_method(StringScanner, "post_match", strscan_post_match, 0);
rb_define_method(StringScanner, "size", strscan_size, 0);
rb_define_method(StringScanner, "captures", strscan_captures, 0);
rb_define_method(StringScanner, "values_at", strscan_values_at, -1);

rb_define_method(StringScanner, "rest", strscan_rest, 0);
rb_define_method(StringScanner, "rest_size", strscan_rest_size, 0);
rb_define_method(StringScanner, "restsize", strscan_restsize, 0);

rb_define_method(StringScanner, "inspect", strscan_inspect, 0);

rb_define_method(StringScanner, "fixed_anchor?", strscan_fixed_anchor_p, 0);
}
