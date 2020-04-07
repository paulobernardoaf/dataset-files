#include "ruby.h"
#include <gdbm.h>
#include <fcntl.h>
#include <errno.h>
static VALUE rb_cGDBM, rb_eGDBMError, rb_eGDBMFatalError;
#if SIZEOF_LONG > SIZEOF_INT
#define TOO_LONG(n) ((long)(+(int)(n)) != (long)(n))
#else
#define TOO_LONG(n) 0
#endif
#define RUBY_GDBM_RW_BIT 0x20000000
#define MY_BLOCK_SIZE (2048)
#define MY_FATAL_FUNC rb_gdbm_fatal
NORETURN(static void rb_gdbm_fatal(const char *msg));
NORETURN(static void closed_dbm(void));
static void
rb_gdbm_fatal(const char *msg)
{
rb_raise(rb_eGDBMFatalError, "%s", msg);
}
struct dbmdata {
int di_size;
GDBM_FILE di_dbm;
};
static void
closed_dbm(void)
{
rb_raise(rb_eRuntimeError, "closed GDBM file");
}
#define GetDBM(obj, dbmp) do {TypedData_Get_Struct((obj), struct dbmdata, &dbm_type, (dbmp));if ((dbmp)->di_dbm == 0) closed_dbm();} while (0)
#define GetDBM2(obj, dbmp, dbm) do {GetDBM((obj), (dbmp));(dbm) = (dbmp)->di_dbm;} while (0)
static void
free_dbm(void *ptr)
{
struct dbmdata *dbmp = ptr;
if (dbmp->di_dbm)
gdbm_close(dbmp->di_dbm);
xfree(dbmp);
}
static size_t
memsize_dbm(const void *ptr)
{
const struct dbmdata *dbmp = ptr;
size_t size = sizeof(*dbmp);
if (dbmp->di_dbm)
size += DBM_SIZEOF_DBM;
return size;
}
static const rb_data_type_t dbm_type = {
"gdbm",
{0, free_dbm, memsize_dbm,},
0, 0,
RUBY_TYPED_FREE_IMMEDIATELY,
};
static VALUE
fgdbm_close(VALUE obj)
{
struct dbmdata *dbmp;
GetDBM(obj, dbmp);
gdbm_close(dbmp->di_dbm);
dbmp->di_dbm = 0;
return Qnil;
}
static VALUE
fgdbm_closed(VALUE obj)
{
struct dbmdata *dbmp;
TypedData_Get_Struct(obj, struct dbmdata, &dbm_type, dbmp);
if (dbmp->di_dbm == 0)
return Qtrue;
return Qfalse;
}
static VALUE
fgdbm_s_alloc(VALUE klass)
{
struct dbmdata *dbmp;
return TypedData_Make_Struct(klass, struct dbmdata, &dbm_type, dbmp);
}
static VALUE
fgdbm_initialize(int argc, VALUE *argv, VALUE obj)
{
VALUE file, vmode, vflags;
GDBM_FILE dbm;
struct dbmdata *dbmp;
int mode, flags = 0;
TypedData_Get_Struct(obj, struct dbmdata, &dbm_type, dbmp);
if (rb_scan_args(argc, argv, "12", &file, &vmode, &vflags) == 1) {
mode = 0666; 
}
else if (NIL_P(vmode)) {
mode = -1; 
}
else {
mode = NUM2INT(vmode);
}
if (!NIL_P(vflags))
flags = NUM2INT(vflags);
FilePathValue(file);
#if defined(GDBM_CLOEXEC)
flags |= GDBM_CLOEXEC;
#endif
if (flags & RUBY_GDBM_RW_BIT) {
flags &= ~RUBY_GDBM_RW_BIT;
dbm = gdbm_open(RSTRING_PTR(file), MY_BLOCK_SIZE,
flags, mode, MY_FATAL_FUNC);
}
else {
dbm = 0;
if (mode >= 0)
dbm = gdbm_open(RSTRING_PTR(file), MY_BLOCK_SIZE,
GDBM_WRCREAT|flags, mode, MY_FATAL_FUNC);
if (!dbm)
dbm = gdbm_open(RSTRING_PTR(file), MY_BLOCK_SIZE,
GDBM_WRITER|flags, 0, MY_FATAL_FUNC);
if (!dbm)
dbm = gdbm_open(RSTRING_PTR(file), MY_BLOCK_SIZE,
GDBM_READER|flags, 0, MY_FATAL_FUNC);
}
if (dbm) {
rb_fd_fix_cloexec(gdbm_fdesc(dbm));
}
if (!dbm) {
if (mode == -1) return Qnil;
if (gdbm_errno == GDBM_FILE_OPEN_ERROR ||
gdbm_errno == GDBM_CANT_BE_READER ||
gdbm_errno == GDBM_CANT_BE_WRITER)
rb_sys_fail_str(file);
else
rb_raise(rb_eGDBMError, "%s", gdbm_strerror(gdbm_errno));
}
if (dbmp->di_dbm)
gdbm_close(dbmp->di_dbm);
dbmp->di_dbm = dbm;
dbmp->di_size = -1;
return obj;
}
static VALUE
fgdbm_s_open(int argc, VALUE *argv, VALUE klass)
{
VALUE obj = fgdbm_s_alloc(klass);
if (NIL_P(fgdbm_initialize(argc, argv, obj))) {
return Qnil;
}
if (rb_block_given_p()) {
return rb_ensure(rb_yield, obj, fgdbm_close, obj);
}
return obj;
}
static VALUE
rb_gdbm_fetch(GDBM_FILE dbm, datum key)
{
datum val;
VALUE str;
val = gdbm_fetch(dbm, key);
if (val.dptr == 0)
return Qnil;
str = rb_str_new(val.dptr, val.dsize);
free(val.dptr);
return str;
}
static VALUE
rb_gdbm_fetch2(GDBM_FILE dbm, VALUE keystr)
{
datum key;
long len;
ExportStringValue(keystr);
len = RSTRING_LEN(keystr);
if (TOO_LONG(len)) return Qnil;
key.dptr = RSTRING_PTR(keystr);
key.dsize = (int)len;
return rb_gdbm_fetch(dbm, key);
}
static VALUE
rb_gdbm_fetch3(VALUE obj, VALUE keystr)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
GetDBM2(obj, dbmp, dbm);
return rb_gdbm_fetch2(dbm, keystr);
}
static VALUE
rb_gdbm_firstkey(GDBM_FILE dbm)
{
datum key;
VALUE str;
key = gdbm_firstkey(dbm);
if (key.dptr == 0)
return Qnil;
str = rb_str_new(key.dptr, key.dsize);
free(key.dptr);
return str;
}
static VALUE
rb_gdbm_nextkey(GDBM_FILE dbm, VALUE keystr)
{
datum key, key2;
VALUE str;
long len;
len = RSTRING_LEN(keystr);
if (TOO_LONG(len)) return Qnil;
key.dptr = RSTRING_PTR(keystr);
key.dsize = (int)len;
key2 = gdbm_nextkey(dbm, key);
if (key2.dptr == 0)
return Qnil;
str = rb_str_new(key2.dptr, key2.dsize);
free(key2.dptr);
return str;
}
static VALUE
fgdbm_fetch(VALUE obj, VALUE keystr, VALUE ifnone)
{
VALUE valstr;
valstr = rb_gdbm_fetch3(obj, keystr);
if (NIL_P(valstr)) {
if (ifnone == Qnil && rb_block_given_p())
return rb_yield(keystr);
return ifnone;
}
return valstr;
}
static VALUE
fgdbm_aref(VALUE obj, VALUE keystr)
{
return rb_gdbm_fetch3(obj, keystr);
}
static VALUE
fgdbm_fetch_m(int argc, VALUE *argv, VALUE obj)
{
VALUE keystr, valstr, ifnone;
rb_scan_args(argc, argv, "11", &keystr, &ifnone);
valstr = fgdbm_fetch(obj, keystr, ifnone);
if (argc == 1 && !rb_block_given_p() && NIL_P(valstr))
rb_raise(rb_eIndexError, "key not found");
return valstr;
}
static VALUE
fgdbm_key(VALUE obj, VALUE valstr)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr, valstr2;
ExportStringValue(valstr);
GetDBM2(obj, dbmp, dbm);
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
valstr2 = rb_gdbm_fetch2(dbm, keystr);
if (!NIL_P(valstr2) &&
(int)RSTRING_LEN(valstr) == (int)RSTRING_LEN(valstr2) &&
memcmp(RSTRING_PTR(valstr), RSTRING_PTR(valstr2),
(int)RSTRING_LEN(valstr)) == 0) {
return keystr;
}
}
return Qnil;
}
static VALUE
fgdbm_index(VALUE obj, VALUE value)
{
rb_warn("GDBM#index is deprecated; use GDBM#key");
return fgdbm_key(obj, value);
}
static VALUE
fgdbm_select(VALUE obj)
{
VALUE new = rb_ary_new();
GDBM_FILE dbm;
struct dbmdata *dbmp;
VALUE keystr;
GetDBM2(obj, dbmp, dbm);
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
VALUE assoc = rb_assoc_new(keystr, rb_gdbm_fetch2(dbm, keystr));
VALUE v = rb_yield(assoc);
if (RTEST(v)) {
rb_ary_push(new, assoc);
}
GetDBM2(obj, dbmp, dbm);
}
return new;
}
static VALUE
fgdbm_values_at(int argc, VALUE *argv, VALUE obj)
{
VALUE new = rb_ary_new2(argc);
int i;
for (i=0; i<argc; i++) {
rb_ary_push(new, rb_gdbm_fetch3(obj, argv[i]));
}
return new;
}
static void
rb_gdbm_modify(VALUE obj)
{
if (OBJ_FROZEN(obj)) rb_error_frozen("GDBM");
}
static VALUE
rb_gdbm_delete(VALUE obj, VALUE keystr)
{
datum key;
struct dbmdata *dbmp;
GDBM_FILE dbm;
long len;
rb_gdbm_modify(obj);
ExportStringValue(keystr);
len = RSTRING_LEN(keystr);
if (TOO_LONG(len)) return Qnil;
key.dptr = RSTRING_PTR(keystr);
key.dsize = (int)len;
GetDBM2(obj, dbmp, dbm);
if (!gdbm_exists(dbm, key)) {
return Qnil;
}
if (gdbm_delete(dbm, key)) {
dbmp->di_size = -1;
rb_raise(rb_eGDBMError, "%s", gdbm_strerror(gdbm_errno));
}
else if (dbmp->di_size >= 0) {
dbmp->di_size--;
}
return obj;
}
static VALUE
fgdbm_delete(VALUE obj, VALUE keystr)
{
VALUE valstr;
valstr = fgdbm_fetch(obj, keystr, Qnil);
rb_gdbm_delete(obj, keystr);
return valstr;
}
static VALUE
fgdbm_shift(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr, valstr;
rb_gdbm_modify(obj);
GetDBM2(obj, dbmp, dbm);
keystr = rb_gdbm_firstkey(dbm);
if (NIL_P(keystr)) return Qnil;
valstr = rb_gdbm_fetch2(dbm, keystr);
rb_gdbm_delete(obj, keystr);
return rb_assoc_new(keystr, valstr);
}
static VALUE
fgdbm_delete_if(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr, valstr;
VALUE ret, ary = rb_ary_tmp_new(0);
long i;
int status = 0, n;
rb_gdbm_modify(obj);
GetDBM2(obj, dbmp, dbm);
n = dbmp->di_size;
dbmp->di_size = -1;
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
OBJ_FREEZE(keystr);
valstr = rb_gdbm_fetch2(dbm, keystr);
ret = rb_protect(rb_yield, rb_assoc_new(rb_str_dup(keystr), valstr), &status);
if (status != 0) break;
if (RTEST(ret)) rb_ary_push(ary, keystr);
GetDBM2(obj, dbmp, dbm);
}
for (i = 0; i < RARRAY_LEN(ary); i++)
rb_gdbm_delete(obj, RARRAY_AREF(ary, i));
if (status) rb_jump_tag(status);
if (n > 0) dbmp->di_size = n - (int)RARRAY_LEN(ary);
rb_ary_clear(ary);
return obj;
}
static VALUE
fgdbm_clear(VALUE obj)
{
datum key, nextkey;
struct dbmdata *dbmp;
GDBM_FILE dbm;
rb_gdbm_modify(obj);
GetDBM2(obj, dbmp, dbm);
dbmp->di_size = -1;
#if 0
while (key = gdbm_firstkey(dbm), key.dptr) {
if (gdbm_delete(dbm, key)) {
free(key.dptr);
rb_raise(rb_eGDBMError, "%s", gdbm_strerror(gdbm_errno));
}
free(key.dptr);
}
#else
while (key = gdbm_firstkey(dbm), key.dptr) {
for (; key.dptr; key = nextkey) {
nextkey = gdbm_nextkey(dbm, key);
if (gdbm_delete(dbm, key)) {
free(key.dptr);
if (nextkey.dptr) free(nextkey.dptr);
rb_raise(rb_eGDBMError, "%s", gdbm_strerror(gdbm_errno));
}
free(key.dptr);
}
}
#endif
dbmp->di_size = 0;
return obj;
}
static VALUE
fgdbm_invert(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr, valstr;
VALUE hash = rb_hash_new();
GetDBM2(obj, dbmp, dbm);
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
valstr = rb_gdbm_fetch2(dbm, keystr);
rb_hash_aset(hash, valstr, keystr);
}
return hash;
}
static VALUE
fgdbm_store(VALUE obj, VALUE keystr, VALUE valstr)
{
datum key, val;
struct dbmdata *dbmp;
GDBM_FILE dbm;
rb_gdbm_modify(obj);
ExportStringValue(keystr);
ExportStringValue(valstr);
key.dptr = RSTRING_PTR(keystr);
key.dsize = RSTRING_LENINT(keystr);
val.dptr = RSTRING_PTR(valstr);
val.dsize = RSTRING_LENINT(valstr);
GetDBM2(obj, dbmp, dbm);
dbmp->di_size = -1;
if (gdbm_store(dbm, key, val, GDBM_REPLACE)) {
if (errno == EPERM) rb_sys_fail(0);
rb_raise(rb_eGDBMError, "%s", gdbm_strerror(gdbm_errno));
}
return valstr;
}
static VALUE
update_i(RB_BLOCK_CALL_FUNC_ARGLIST(pair, dbm))
{
const VALUE *ptr;
Check_Type(pair, T_ARRAY);
if (RARRAY_LEN(pair) < 2) {
rb_raise(rb_eArgError, "pair must be [key, value]");
}
ptr = RARRAY_CONST_PTR(pair);
fgdbm_store(dbm, ptr[0], ptr[1]);
return Qnil;
}
static VALUE
fgdbm_update(VALUE obj, VALUE other)
{
rb_block_call(other, rb_intern("each_pair"), 0, 0, update_i, obj);
return obj;
}
static VALUE
fgdbm_replace(VALUE obj, VALUE other)
{
fgdbm_clear(obj);
rb_block_call(other, rb_intern("each_pair"), 0, 0, update_i, obj);
return obj;
}
static VALUE
fgdbm_length(VALUE obj)
{
datum key, nextkey;
struct dbmdata *dbmp;
GDBM_FILE dbm;
int i = 0;
GetDBM2(obj, dbmp, dbm);
if (dbmp->di_size > 0) return INT2FIX(dbmp->di_size);
for (key = gdbm_firstkey(dbm); key.dptr; key = nextkey) {
nextkey = gdbm_nextkey(dbm, key);
free(key.dptr);
i++;
}
dbmp->di_size = i;
return INT2FIX(i);
}
static VALUE
fgdbm_empty_p(VALUE obj)
{
datum key;
struct dbmdata *dbmp;
GDBM_FILE dbm;
GetDBM(obj, dbmp);
if (dbmp->di_size < 0) {
dbm = dbmp->di_dbm;
key = gdbm_firstkey(dbm);
if (key.dptr) {
free(key.dptr);
return Qfalse;
}
return Qtrue;
}
if (dbmp->di_size == 0) return Qtrue;
return Qfalse;
}
static VALUE
fgdbm_each_value(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr;
RETURN_ENUMERATOR(obj, 0, 0);
GetDBM2(obj, dbmp, dbm);
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
rb_yield(rb_gdbm_fetch2(dbm, keystr));
GetDBM2(obj, dbmp, dbm);
}
return obj;
}
static VALUE
fgdbm_each_key(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr;
RETURN_ENUMERATOR(obj, 0, 0);
GetDBM2(obj, dbmp, dbm);
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
rb_yield(keystr);
GetDBM2(obj, dbmp, dbm);
}
return obj;
}
static VALUE
fgdbm_each_pair(VALUE obj)
{
GDBM_FILE dbm;
struct dbmdata *dbmp;
VALUE keystr;
RETURN_ENUMERATOR(obj, 0, 0);
GetDBM2(obj, dbmp, dbm);
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
rb_yield(rb_assoc_new(keystr, rb_gdbm_fetch2(dbm, keystr)));
GetDBM2(obj, dbmp, dbm);
}
return obj;
}
static VALUE
fgdbm_keys(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr, ary;
GetDBM2(obj, dbmp, dbm);
ary = rb_ary_new();
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
rb_ary_push(ary, keystr);
}
return ary;
}
static VALUE
fgdbm_values(VALUE obj)
{
datum key, nextkey;
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE valstr, ary;
GetDBM2(obj, dbmp, dbm);
ary = rb_ary_new();
for (key = gdbm_firstkey(dbm); key.dptr; key = nextkey) {
nextkey = gdbm_nextkey(dbm, key);
valstr = rb_gdbm_fetch(dbm, key);
free(key.dptr);
rb_ary_push(ary, valstr);
}
return ary;
}
static VALUE
fgdbm_has_key(VALUE obj, VALUE keystr)
{
datum key;
struct dbmdata *dbmp;
GDBM_FILE dbm;
long len;
ExportStringValue(keystr);
len = RSTRING_LENINT(keystr);
if (TOO_LONG(len)) return Qfalse;
key.dptr = RSTRING_PTR(keystr);
key.dsize = (int)len;
GetDBM2(obj, dbmp, dbm);
if (gdbm_exists(dbm, key))
return Qtrue;
return Qfalse;
}
static VALUE
fgdbm_has_value(VALUE obj, VALUE valstr)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr, valstr2;
ExportStringValue(valstr);
GetDBM2(obj, dbmp, dbm);
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
valstr2 = rb_gdbm_fetch2(dbm, keystr);
if (!NIL_P(valstr2) &&
(int)RSTRING_LEN(valstr) == (int)RSTRING_LEN(valstr2) &&
memcmp(RSTRING_PTR(valstr), RSTRING_PTR(valstr2),
(int)RSTRING_LEN(valstr)) == 0) {
return Qtrue;
}
}
return Qfalse;
}
static VALUE
fgdbm_to_a(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr, ary;
GetDBM2(obj, dbmp, dbm);
ary = rb_ary_new();
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
rb_ary_push(ary, rb_assoc_new(keystr, rb_gdbm_fetch2(dbm, keystr)));
}
return ary;
}
static VALUE
fgdbm_reorganize(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
rb_gdbm_modify(obj);
GetDBM2(obj, dbmp, dbm);
gdbm_reorganize(dbm);
rb_fd_fix_cloexec(gdbm_fdesc(dbm));
return obj;
}
static VALUE
fgdbm_sync(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
rb_gdbm_modify(obj);
GetDBM2(obj, dbmp, dbm);
gdbm_sync(dbm);
return obj;
}
static VALUE
fgdbm_set_cachesize(VALUE obj, VALUE val)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
int optval;
GetDBM2(obj, dbmp, dbm);
optval = FIX2INT(val);
if (gdbm_setopt(dbm, GDBM_CACHESIZE, &optval, sizeof(optval)) == -1) {
rb_raise(rb_eGDBMError, "%s", gdbm_strerror(gdbm_errno));
}
return val;
}
static VALUE
fgdbm_set_fastmode(VALUE obj, VALUE val)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
int optval;
GetDBM2(obj, dbmp, dbm);
optval = 0;
if (RTEST(val))
optval = 1;
if (gdbm_setopt(dbm, GDBM_FASTMODE, &optval, sizeof(optval)) == -1) {
rb_raise(rb_eGDBMError, "%s", gdbm_strerror(gdbm_errno));
}
return val;
}
static VALUE
fgdbm_set_syncmode(VALUE obj, VALUE val)
{
#if !defined(GDBM_SYNCMODE)
fgdbm_set_fastmode(obj, RTEST(val) ? Qfalse : Qtrue);
return val;
#else
struct dbmdata *dbmp;
GDBM_FILE dbm;
int optval;
GetDBM2(obj, dbmp, dbm);
optval = 0;
if (RTEST(val))
optval = 1;
if (gdbm_setopt(dbm, GDBM_FASTMODE, &optval, sizeof(optval)) == -1) {
rb_raise(rb_eGDBMError, "%s", gdbm_strerror(gdbm_errno));
}
return val;
#endif
}
static VALUE
fgdbm_to_hash(VALUE obj)
{
struct dbmdata *dbmp;
GDBM_FILE dbm;
VALUE keystr, hash;
GetDBM2(obj, dbmp, dbm);
hash = rb_hash_new();
for (keystr = rb_gdbm_firstkey(dbm); RTEST(keystr);
keystr = rb_gdbm_nextkey(dbm, keystr)) {
rb_hash_aset(hash, keystr, rb_gdbm_fetch2(dbm, keystr));
}
return hash;
}
static VALUE
fgdbm_reject(VALUE obj)
{
return rb_hash_delete_if(fgdbm_to_hash(obj));
}
void
Init_gdbm(void)
{
rb_cGDBM = rb_define_class("GDBM", rb_cObject);
rb_eGDBMError = rb_define_class("GDBMError", rb_eStandardError);
rb_eGDBMFatalError = rb_define_class("GDBMFatalError", rb_eException);
rb_include_module(rb_cGDBM, rb_mEnumerable);
rb_define_alloc_func(rb_cGDBM, fgdbm_s_alloc);
rb_define_singleton_method(rb_cGDBM, "open", fgdbm_s_open, -1);
rb_define_method(rb_cGDBM, "initialize", fgdbm_initialize, -1);
rb_define_method(rb_cGDBM, "close", fgdbm_close, 0);
rb_define_method(rb_cGDBM, "closed?", fgdbm_closed, 0);
rb_define_method(rb_cGDBM, "[]", fgdbm_aref, 1);
rb_define_method(rb_cGDBM, "fetch", fgdbm_fetch_m, -1);
rb_define_method(rb_cGDBM, "[]=", fgdbm_store, 2);
rb_define_method(rb_cGDBM, "store", fgdbm_store, 2);
rb_define_method(rb_cGDBM, "index", fgdbm_index, 1);
rb_define_method(rb_cGDBM, "key", fgdbm_key, 1);
rb_define_method(rb_cGDBM, "select", fgdbm_select, 0);
rb_define_method(rb_cGDBM, "values_at", fgdbm_values_at, -1);
rb_define_method(rb_cGDBM, "length", fgdbm_length, 0);
rb_define_method(rb_cGDBM, "size", fgdbm_length, 0);
rb_define_method(rb_cGDBM, "empty?", fgdbm_empty_p, 0);
rb_define_method(rb_cGDBM, "each", fgdbm_each_pair, 0);
rb_define_method(rb_cGDBM, "each_value", fgdbm_each_value, 0);
rb_define_method(rb_cGDBM, "each_key", fgdbm_each_key, 0);
rb_define_method(rb_cGDBM, "each_pair", fgdbm_each_pair, 0);
rb_define_method(rb_cGDBM, "keys", fgdbm_keys, 0);
rb_define_method(rb_cGDBM, "values", fgdbm_values, 0);
rb_define_method(rb_cGDBM, "shift", fgdbm_shift, 0);
rb_define_method(rb_cGDBM, "delete", fgdbm_delete, 1);
rb_define_method(rb_cGDBM, "delete_if", fgdbm_delete_if, 0);
rb_define_method(rb_cGDBM, "reject!", fgdbm_delete_if, 0);
rb_define_method(rb_cGDBM, "reject", fgdbm_reject, 0);
rb_define_method(rb_cGDBM, "clear", fgdbm_clear, 0);
rb_define_method(rb_cGDBM, "invert", fgdbm_invert, 0);
rb_define_method(rb_cGDBM, "update", fgdbm_update, 1);
rb_define_method(rb_cGDBM, "replace", fgdbm_replace, 1);
rb_define_method(rb_cGDBM, "reorganize", fgdbm_reorganize, 0);
rb_define_method(rb_cGDBM, "sync", fgdbm_sync, 0);
rb_define_method(rb_cGDBM, "cachesize=", fgdbm_set_cachesize, 1);
rb_define_method(rb_cGDBM, "fastmode=", fgdbm_set_fastmode, 1);
rb_define_method(rb_cGDBM, "syncmode=", fgdbm_set_syncmode, 1);
rb_define_method(rb_cGDBM, "include?", fgdbm_has_key, 1);
rb_define_method(rb_cGDBM, "has_key?", fgdbm_has_key, 1);
rb_define_method(rb_cGDBM, "member?", fgdbm_has_key, 1);
rb_define_method(rb_cGDBM, "has_value?", fgdbm_has_value, 1);
rb_define_method(rb_cGDBM, "key?", fgdbm_has_key, 1);
rb_define_method(rb_cGDBM, "value?", fgdbm_has_value, 1);
rb_define_method(rb_cGDBM, "to_a", fgdbm_to_a, 0);
rb_define_method(rb_cGDBM, "to_hash", fgdbm_to_hash, 0);
rb_define_const(rb_cGDBM, "READER", INT2FIX(GDBM_READER|RUBY_GDBM_RW_BIT));
rb_define_const(rb_cGDBM, "WRITER", INT2FIX(GDBM_WRITER|RUBY_GDBM_RW_BIT));
rb_define_const(rb_cGDBM, "WRCREAT", INT2FIX(GDBM_WRCREAT|RUBY_GDBM_RW_BIT));
rb_define_const(rb_cGDBM, "NEWDB", INT2FIX(GDBM_NEWDB|RUBY_GDBM_RW_BIT));
rb_define_const(rb_cGDBM, "FAST", INT2FIX(GDBM_FAST));
#if defined(GDBM_SYNC)
rb_define_const(rb_cGDBM, "SYNC", INT2FIX(GDBM_SYNC));
#endif
#if defined(GDBM_NOLOCK)
rb_define_const(rb_cGDBM, "NOLOCK", INT2FIX(GDBM_NOLOCK));
#endif
rb_define_const(rb_cGDBM, "VERSION", rb_str_new2(gdbm_version));
}
