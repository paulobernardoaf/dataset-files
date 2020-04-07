










#include "ruby.h"

#if defined(HAVE_CDEFS_H)
#include <cdefs.h>
#endif
#if defined(HAVE_SYS_CDEFS_H)
#include <sys/cdefs.h>
#endif
#include DBM_HDR
#include <fcntl.h>
#include <errno.h>

#define DSIZE_TYPE TYPEOF_DATUM_DSIZE
#if SIZEOF_DATUM_DSIZE > SIZEOF_INT
#define RSTRING_DSIZE(s) RSTRING_LEN(s)
#define TOO_LONG(n) ((void)(n),0)
#else
#define RSTRING_DSIZE(s) RSTRING_LENINT(s)
#define TOO_LONG(n) ((long)(+(DSIZE_TYPE)(n)) != (n))
#endif

static VALUE rb_cDBM, rb_eDBMError;

#define RUBY_DBM_RW_BIT 0x20000000

struct dbmdata {
long di_size;
DBM *di_dbm;
};

NORETURN(static void closed_dbm(void));

static void
closed_dbm(void)
{
rb_raise(rb_eDBMError, "closed DBM file");
}

#define GetDBM(obj, dbmp) do {TypedData_Get_Struct((obj), struct dbmdata, &dbm_type, (dbmp));if ((dbmp)->di_dbm == 0) closed_dbm();} while (0)




#define GetDBM2(obj, dbmp, dbm) do {GetDBM((obj), (dbmp));(dbm) = (dbmp)->di_dbm;} while (0)




static void
free_dbm(void *ptr)
{
struct dbmdata *dbmp = ptr;
if (dbmp->di_dbm)
dbm_close(dbmp->di_dbm);
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
"dbm",
{0, free_dbm, memsize_dbm,},
0, 0,
RUBY_TYPED_FREE_IMMEDIATELY,
};







static VALUE
fdbm_close(VALUE obj)
{
struct dbmdata *dbmp;

GetDBM(obj, dbmp);
dbm_close(dbmp->di_dbm);
dbmp->di_dbm = 0;

return Qnil;
}







static VALUE
fdbm_closed(VALUE obj)
{
struct dbmdata *dbmp;

TypedData_Get_Struct(obj, struct dbmdata, &dbm_type, dbmp);
if (dbmp->di_dbm == 0)
return Qtrue;

return Qfalse;
}

static VALUE
fdbm_alloc(VALUE klass)
{
struct dbmdata *dbmp;

return TypedData_Make_Struct(klass, struct dbmdata, &dbm_type, dbmp);
}














static VALUE
fdbm_initialize(int argc, VALUE *argv, VALUE obj)
{
VALUE file, vmode, vflags;
DBM *dbm;
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





#if !defined(O_CLOEXEC)
#define O_CLOEXEC 0
#endif

if (flags & RUBY_DBM_RW_BIT) {
flags &= ~RUBY_DBM_RW_BIT;
dbm = dbm_open(RSTRING_PTR(file), flags|O_CLOEXEC, mode);
}
else {
dbm = 0;
if (mode >= 0) {
dbm = dbm_open(RSTRING_PTR(file), O_RDWR|O_CREAT|O_CLOEXEC, mode);
}
if (!dbm) {
dbm = dbm_open(RSTRING_PTR(file), O_RDWR|O_CLOEXEC, 0);
}
if (!dbm) {
dbm = dbm_open(RSTRING_PTR(file), O_RDONLY|O_CLOEXEC, 0);
}
}

if (dbm) {


















#if defined(HAVE_DBM_PAGFNO)
rb_fd_fix_cloexec(dbm_pagfno(dbm));
#endif
#if defined(HAVE_DBM_DIRFNO)
rb_fd_fix_cloexec(dbm_dirfno(dbm));
#endif

#if defined(RUBYDBM_DB_HEADER) && defined(HAVE_TYPE_DBC)


((DBC*)dbm)->dbp->set_errfile(((DBC*)dbm)->dbp, NULL);
#endif
}

if (!dbm) {
if (mode == -1) return Qnil;
rb_sys_fail_str(file);
}

if (dbmp->di_dbm)
dbm_close(dbmp->di_dbm);
dbmp->di_dbm = dbm;
dbmp->di_size = -1;

return obj;
}









static VALUE
fdbm_s_open(int argc, VALUE *argv, VALUE klass)
{
VALUE obj = fdbm_alloc(klass);

if (NIL_P(fdbm_initialize(argc, argv, obj))) {
return Qnil;
}

if (rb_block_given_p()) {
return rb_ensure(rb_yield, obj, fdbm_close, obj);
}

return obj;
}

static VALUE
fdbm_fetch(VALUE obj, VALUE keystr, VALUE ifnone)
{
datum key, value;
struct dbmdata *dbmp;
DBM *dbm;
long len;

ExportStringValue(keystr);
len = RSTRING_LEN(keystr);
if (TOO_LONG(len)) goto not_found;
key.dptr = RSTRING_PTR(keystr);
key.dsize = (DSIZE_TYPE)len;

GetDBM2(obj, dbmp, dbm);
value = dbm_fetch(dbm, key);
if (value.dptr == 0) {
not_found:
if (NIL_P(ifnone) && rb_block_given_p()) {
keystr = rb_str_dup(keystr);
return rb_yield(keystr);
}
return ifnone;
}
return rb_str_new(value.dptr, value.dsize);
}








static VALUE
fdbm_aref(VALUE obj, VALUE keystr)
{
return fdbm_fetch(obj, keystr, Qnil);
}









static VALUE
fdbm_fetch_m(int argc, VALUE *argv, VALUE obj)
{
VALUE keystr, valstr, ifnone;

rb_scan_args(argc, argv, "11", &keystr, &ifnone);
valstr = fdbm_fetch(obj, keystr, ifnone);
if (argc == 1 && !rb_block_given_p() && NIL_P(valstr))
rb_raise(rb_eIndexError, "key not found");

return valstr;
}







static VALUE
fdbm_key(VALUE obj, VALUE valstr)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;
long len;

ExportStringValue(valstr);
len = RSTRING_LEN(valstr);
if (TOO_LONG(len)) return Qnil;

GetDBM2(obj, dbmp, dbm);
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
val = dbm_fetch(dbm, key);
if ((long)val.dsize == RSTRING_LEN(valstr) &&
memcmp(val.dptr, RSTRING_PTR(valstr), val.dsize) == 0) {
return rb_str_new(key.dptr, key.dsize);
}
}
return Qnil;
}


static VALUE
fdbm_index(VALUE hash, VALUE value)
{
rb_warn("DBM#index is deprecated; use DBM#key");
return fdbm_key(hash, value);
}








static VALUE
fdbm_select(VALUE obj)
{
VALUE new = rb_ary_new();
datum key, val;
DBM *dbm;
struct dbmdata *dbmp;

GetDBM2(obj, dbmp, dbm);
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
VALUE assoc, v;
val = dbm_fetch(dbm, key);
assoc = rb_assoc_new(rb_str_new(key.dptr, key.dsize),
rb_str_new(val.dptr, val.dsize));
v = rb_yield(assoc);
if (RTEST(v)) {
rb_ary_push(new, assoc);
}
GetDBM2(obj, dbmp, dbm);
}

return new;
}







static VALUE
fdbm_values_at(int argc, VALUE *argv, VALUE obj)
{
VALUE new = rb_ary_new2(argc);
int i;

for (i=0; i<argc; i++) {
rb_ary_push(new, fdbm_fetch(obj, argv[i], Qnil));
}

return new;
}

static void
fdbm_modify(VALUE obj)
{
if (OBJ_FROZEN(obj)) rb_error_frozen("DBM");
}







static VALUE
fdbm_delete(VALUE obj, VALUE keystr)
{
datum key, value;
struct dbmdata *dbmp;
DBM *dbm;
VALUE valstr;
long len;

fdbm_modify(obj);
ExportStringValue(keystr);
len = RSTRING_LEN(keystr);
if (TOO_LONG(len)) goto not_found;
key.dptr = RSTRING_PTR(keystr);
key.dsize = (DSIZE_TYPE)len;

GetDBM2(obj, dbmp, dbm);

value = dbm_fetch(dbm, key);
if (value.dptr == 0) {
not_found:
if (rb_block_given_p()) return rb_yield(keystr);
return Qnil;
}


valstr = rb_str_new(value.dptr, value.dsize);

if (dbm_delete(dbm, key)) {
dbmp->di_size = -1;
rb_raise(rb_eDBMError, "dbm_delete failed");
}
else if (dbmp->di_size >= 0) {
dbmp->di_size--;
}
return valstr;
}









static VALUE
fdbm_shift(VALUE obj)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;
VALUE keystr, valstr;

fdbm_modify(obj);
GetDBM2(obj, dbmp, dbm);
dbmp->di_size = -1;

key = dbm_firstkey(dbm);
if (!key.dptr) return Qnil;
val = dbm_fetch(dbm, key);
keystr = rb_str_new(key.dptr, key.dsize);
valstr = rb_str_new(val.dptr, val.dsize);
dbm_delete(dbm, key);

return rb_assoc_new(keystr, valstr);
}









static VALUE
fdbm_delete_if(VALUE obj)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;
VALUE keystr, valstr;
VALUE ret, ary = rb_ary_tmp_new(0);
int status = 0;
long i, n;

fdbm_modify(obj);
GetDBM2(obj, dbmp, dbm);
n = dbmp->di_size;
dbmp->di_size = -1;

for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
val = dbm_fetch(dbm, key);
keystr = rb_str_new(key.dptr, key.dsize);
OBJ_FREEZE(keystr);
valstr = rb_str_new(val.dptr, val.dsize);
ret = rb_protect(rb_yield, rb_assoc_new(rb_str_dup(keystr), valstr), &status);
if (status != 0) break;
if (RTEST(ret)) rb_ary_push(ary, keystr);
GetDBM2(obj, dbmp, dbm);
}

for (i = 0; i < RARRAY_LEN(ary); i++) {
keystr = RARRAY_AREF(ary, i);
key.dptr = RSTRING_PTR(keystr);
key.dsize = (DSIZE_TYPE)RSTRING_LEN(keystr);
if (dbm_delete(dbm, key)) {
rb_raise(rb_eDBMError, "dbm_delete failed");
}
}
if (status) rb_jump_tag(status);
if (n > 0) dbmp->di_size = n - RARRAY_LEN(ary);
rb_ary_clear(ary);

return obj;
}







static VALUE
fdbm_clear(VALUE obj)
{
datum key;
struct dbmdata *dbmp;
DBM *dbm;

fdbm_modify(obj);
GetDBM2(obj, dbmp, dbm);
dbmp->di_size = -1;
while (key = dbm_firstkey(dbm), key.dptr) {
if (dbm_delete(dbm, key)) {
rb_raise(rb_eDBMError, "dbm_delete failed");
}
}
dbmp->di_size = 0;

return obj;
}








static VALUE
fdbm_invert(VALUE obj)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;
VALUE keystr, valstr;
VALUE hash = rb_hash_new();

GetDBM2(obj, dbmp, dbm);
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
val = dbm_fetch(dbm, key);
keystr = rb_str_new(key.dptr, key.dsize);
valstr = rb_str_new(val.dptr, val.dsize);
rb_hash_aset(hash, valstr, keystr);
}
return hash;
}

static VALUE fdbm_store(VALUE,VALUE,VALUE);

static VALUE
update_i(RB_BLOCK_CALL_FUNC_ARGLIST(pair, dbm))
{
const VALUE *ptr;
Check_Type(pair, T_ARRAY);
if (RARRAY_LEN(pair) < 2) {
rb_raise(rb_eArgError, "pair must be [key, value]");
}
ptr = RARRAY_CONST_PTR(pair);
fdbm_store(dbm, ptr[0], ptr[1]);
return Qnil;
}









static VALUE
fdbm_update(VALUE obj, VALUE other)
{
rb_block_call(other, rb_intern("each_pair"), 0, 0, update_i, obj);
return obj;
}









static VALUE
fdbm_replace(VALUE obj, VALUE other)
{
fdbm_clear(obj);
rb_block_call(other, rb_intern("each_pair"), 0, 0, update_i, obj);
return obj;
}









static VALUE
fdbm_store(VALUE obj, VALUE keystr, VALUE valstr)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;

fdbm_modify(obj);
keystr = rb_obj_as_string(keystr);
valstr = rb_obj_as_string(valstr);

key.dptr = RSTRING_PTR(keystr);
key.dsize = RSTRING_DSIZE(keystr);

val.dptr = RSTRING_PTR(valstr);
val.dsize = RSTRING_DSIZE(valstr);

GetDBM2(obj, dbmp, dbm);
dbmp->di_size = -1;
if (dbm_store(dbm, key, val, DBM_REPLACE)) {
dbm_clearerr(dbm);
if (errno == EPERM) rb_sys_fail(0);
rb_raise(rb_eDBMError, "dbm_store failed");
}

return valstr;
}








static VALUE
fdbm_length(VALUE obj)
{
datum key;
struct dbmdata *dbmp;
DBM *dbm;
int i = 0;

GetDBM2(obj, dbmp, dbm);
if (dbmp->di_size > 0) return INT2FIX(dbmp->di_size);

for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
i++;
}
dbmp->di_size = i;

return INT2FIX(i);
}







static VALUE
fdbm_empty_p(VALUE obj)
{
datum key;
struct dbmdata *dbmp;
DBM *dbm;

GetDBM2(obj, dbmp, dbm);
if (dbmp->di_size < 0) {
dbm = dbmp->di_dbm;

for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
return Qfalse;
}
}
else {
if (dbmp->di_size)
return Qfalse;
}
return Qtrue;
}







static VALUE
fdbm_each_value(VALUE obj)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;

RETURN_ENUMERATOR(obj, 0, 0);

GetDBM2(obj, dbmp, dbm);
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
val = dbm_fetch(dbm, key);
rb_yield(rb_str_new(val.dptr, val.dsize));
GetDBM2(obj, dbmp, dbm);
}
return obj;
}







static VALUE
fdbm_each_key(VALUE obj)
{
datum key;
struct dbmdata *dbmp;
DBM *dbm;

RETURN_ENUMERATOR(obj, 0, 0);

GetDBM2(obj, dbmp, dbm);
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
rb_yield(rb_str_new(key.dptr, key.dsize));
GetDBM2(obj, dbmp, dbm);
}
return obj;
}








static VALUE
fdbm_each_pair(VALUE obj)
{
datum key, val;
DBM *dbm;
struct dbmdata *dbmp;
VALUE keystr, valstr;

RETURN_ENUMERATOR(obj, 0, 0);

GetDBM2(obj, dbmp, dbm);

for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
val = dbm_fetch(dbm, key);
keystr = rb_str_new(key.dptr, key.dsize);
valstr = rb_str_new(val.dptr, val.dsize);
rb_yield(rb_assoc_new(keystr, valstr));
GetDBM2(obj, dbmp, dbm);
}

return obj;
}







static VALUE
fdbm_keys(VALUE obj)
{
datum key;
struct dbmdata *dbmp;
DBM *dbm;
VALUE ary;

GetDBM2(obj, dbmp, dbm);

ary = rb_ary_new();
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
rb_ary_push(ary, rb_str_new(key.dptr, key.dsize));
}

return ary;
}







static VALUE
fdbm_values(VALUE obj)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;
VALUE ary;

GetDBM2(obj, dbmp, dbm);
ary = rb_ary_new();
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
val = dbm_fetch(dbm, key);
rb_ary_push(ary, rb_str_new(val.dptr, val.dsize));
}

return ary;
}










static VALUE
fdbm_has_key(VALUE obj, VALUE keystr)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;
long len;

ExportStringValue(keystr);
len = RSTRING_LEN(keystr);
if (TOO_LONG(len)) return Qfalse;
key.dptr = RSTRING_PTR(keystr);
key.dsize = (DSIZE_TYPE)len;

GetDBM2(obj, dbmp, dbm);
val = dbm_fetch(dbm, key);
if (val.dptr) return Qtrue;
return Qfalse;
}









static VALUE
fdbm_has_value(VALUE obj, VALUE valstr)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;
long len;

ExportStringValue(valstr);
len = RSTRING_LEN(valstr);
if (TOO_LONG(len)) return Qfalse;
val.dptr = RSTRING_PTR(valstr);
val.dsize = (DSIZE_TYPE)len;

GetDBM2(obj, dbmp, dbm);
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
val = dbm_fetch(dbm, key);
if ((DSIZE_TYPE)val.dsize == (DSIZE_TYPE)RSTRING_LEN(valstr) &&
memcmp(val.dptr, RSTRING_PTR(valstr), val.dsize) == 0)
return Qtrue;
}
return Qfalse;
}








static VALUE
fdbm_to_a(VALUE obj)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;
VALUE ary;

GetDBM2(obj, dbmp, dbm);
ary = rb_ary_new();
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
val = dbm_fetch(dbm, key);
rb_ary_push(ary, rb_assoc_new(rb_str_new(key.dptr, key.dsize),
rb_str_new(val.dptr, val.dsize)));
}

return ary;
}








static VALUE
fdbm_to_hash(VALUE obj)
{
datum key, val;
struct dbmdata *dbmp;
DBM *dbm;
VALUE hash;

GetDBM2(obj, dbmp, dbm);
hash = rb_hash_new();
for (key = dbm_firstkey(dbm); key.dptr; key = dbm_nextkey(dbm)) {
val = dbm_fetch(dbm, key);
rb_hash_aset(hash, rb_str_new(key.dptr, key.dsize),
rb_str_new(val.dptr, val.dsize));
}

return hash;
}








static VALUE
fdbm_reject(VALUE obj)
{
return rb_hash_delete_if(fdbm_to_hash(obj));
}






























































void
Init_dbm(void)
{
rb_cDBM = rb_define_class("DBM", rb_cObject);



rb_eDBMError = rb_define_class("DBMError", rb_eStandardError);
rb_include_module(rb_cDBM, rb_mEnumerable);

rb_define_alloc_func(rb_cDBM, fdbm_alloc);
rb_define_singleton_method(rb_cDBM, "open", fdbm_s_open, -1);

rb_define_method(rb_cDBM, "initialize", fdbm_initialize, -1);
rb_define_method(rb_cDBM, "close", fdbm_close, 0);
rb_define_method(rb_cDBM, "closed?", fdbm_closed, 0);
rb_define_method(rb_cDBM, "[]", fdbm_aref, 1);
rb_define_method(rb_cDBM, "fetch", fdbm_fetch_m, -1);
rb_define_method(rb_cDBM, "[]=", fdbm_store, 2);
rb_define_method(rb_cDBM, "store", fdbm_store, 2);
rb_define_method(rb_cDBM, "index", fdbm_index, 1);
rb_define_method(rb_cDBM, "key", fdbm_key, 1);
rb_define_method(rb_cDBM, "select", fdbm_select, 0);
rb_define_method(rb_cDBM, "values_at", fdbm_values_at, -1);
rb_define_method(rb_cDBM, "length", fdbm_length, 0);
rb_define_method(rb_cDBM, "size", fdbm_length, 0);
rb_define_method(rb_cDBM, "empty?", fdbm_empty_p, 0);
rb_define_method(rb_cDBM, "each", fdbm_each_pair, 0);
rb_define_method(rb_cDBM, "each_value", fdbm_each_value, 0);
rb_define_method(rb_cDBM, "each_key", fdbm_each_key, 0);
rb_define_method(rb_cDBM, "each_pair", fdbm_each_pair, 0);
rb_define_method(rb_cDBM, "keys", fdbm_keys, 0);
rb_define_method(rb_cDBM, "values", fdbm_values, 0);
rb_define_method(rb_cDBM, "shift", fdbm_shift, 0);
rb_define_method(rb_cDBM, "delete", fdbm_delete, 1);
rb_define_method(rb_cDBM, "delete_if", fdbm_delete_if, 0);
rb_define_method(rb_cDBM, "reject!", fdbm_delete_if, 0);
rb_define_method(rb_cDBM, "reject", fdbm_reject, 0);
rb_define_method(rb_cDBM, "clear", fdbm_clear, 0);
rb_define_method(rb_cDBM, "invert", fdbm_invert, 0);
rb_define_method(rb_cDBM, "update", fdbm_update, 1);
rb_define_method(rb_cDBM, "replace", fdbm_replace, 1);

rb_define_method(rb_cDBM, "include?", fdbm_has_key, 1);
rb_define_method(rb_cDBM, "has_key?", fdbm_has_key, 1);
rb_define_method(rb_cDBM, "member?", fdbm_has_key, 1);
rb_define_method(rb_cDBM, "has_value?", fdbm_has_value, 1);
rb_define_method(rb_cDBM, "key?", fdbm_has_key, 1);
rb_define_method(rb_cDBM, "value?", fdbm_has_value, 1);

rb_define_method(rb_cDBM, "to_a", fdbm_to_a, 0);
rb_define_method(rb_cDBM, "to_hash", fdbm_to_hash, 0);


rb_define_const(rb_cDBM, "READER", INT2FIX(O_RDONLY|RUBY_DBM_RW_BIT));


rb_define_const(rb_cDBM, "WRITER", INT2FIX(O_RDWR|RUBY_DBM_RW_BIT));




rb_define_const(rb_cDBM, "WRCREAT", INT2FIX(O_RDWR|O_CREAT|RUBY_DBM_RW_BIT));





rb_define_const(rb_cDBM, "NEWDB", INT2FIX(O_RDWR|O_CREAT|O_TRUNC|RUBY_DBM_RW_BIT));

{
VALUE version;
#if defined(_DBM_IOERR)
version = rb_str_new2("ndbm (4.3BSD)");
#elif defined(RUBYDBM_GDBM_HEADER)
#if defined(HAVE_DECLARED_LIBVAR_GDBM_VERSION)

version = rb_str_new2(gdbm_version);
#elif defined(HAVE_UNDECLARED_LIBVAR_GDBM_VERSION)


RUBY_EXTERN char *gdbm_version;
version = rb_str_new2(gdbm_version);
#else
version = rb_str_new2("GDBM (unknown)");
#endif
#elif defined(RUBYDBM_DB_HEADER)
#if defined(HAVE_DB_VERSION)

version = rb_str_new2(db_version(NULL, NULL, NULL));
#else
version = rb_str_new2("Berkeley DB (unknown)");
#endif
#elif defined(_RELIC_H)
#if defined(HAVE_DPVERSION)
version = rb_sprintf("QDBM %s", dpversion);
#else
version = rb_str_new2("QDBM (unknown)");
#endif
#else
version = rb_str_new2("ndbm (unknown)");
#endif












rb_define_const(rb_cDBM, "VERSION", version);
}
}
