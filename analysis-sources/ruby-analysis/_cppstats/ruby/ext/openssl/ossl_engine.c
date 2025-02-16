#include "ossl.h"
#if !defined(OPENSSL_NO_ENGINE)
#define NewEngine(klass) TypedData_Wrap_Struct((klass), &ossl_engine_type, 0)
#define SetEngine(obj, engine) do { if (!(engine)) { ossl_raise(rb_eRuntimeError, "ENGINE wasn't initialized."); } RTYPEDDATA_DATA(obj) = (engine); } while(0)
#define GetEngine(obj, engine) do { TypedData_Get_Struct((obj), ENGINE, &ossl_engine_type, (engine)); if (!(engine)) { ossl_raise(rb_eRuntimeError, "ENGINE wasn't initialized."); } } while (0)
VALUE cEngine;
VALUE eEngineError;
#if !defined(LIBRESSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER >= 0x10100000
#define OSSL_ENGINE_LOAD_IF_MATCH(engine_name, x) do{if(!strcmp(#engine_name, RSTRING_PTR(name))){if (OPENSSL_init_crypto(OPENSSL_INIT_ENGINE_##x, NULL))return Qtrue;elseossl_raise(eEngineError, "OPENSSL_init_crypto"); }}while(0)
#else
#define OSSL_ENGINE_LOAD_IF_MATCH(engine_name, x) do{if(!strcmp(#engine_name, RSTRING_PTR(name))){ENGINE_load_##engine_name();return Qtrue;}}while(0)
#endif
static void
ossl_engine_free(void *engine)
{
ENGINE_free(engine);
}
static const rb_data_type_t ossl_engine_type = {
"OpenSSL/Engine",
{
0, ossl_engine_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};
static VALUE
ossl_engine_s_load(int argc, VALUE *argv, VALUE klass)
{
VALUE name;
rb_scan_args(argc, argv, "01", &name);
if(NIL_P(name)){
ENGINE_load_builtin_engines();
return Qtrue;
}
StringValueCStr(name);
#if HAVE_ENGINE_LOAD_DYNAMIC
OSSL_ENGINE_LOAD_IF_MATCH(dynamic, DYNAMIC);
#endif
#if !defined(OPENSSL_NO_STATIC_ENGINE)
#if HAVE_ENGINE_LOAD_4758CCA
OSSL_ENGINE_LOAD_IF_MATCH(4758cca, 4758CCA);
#endif
#if HAVE_ENGINE_LOAD_AEP
OSSL_ENGINE_LOAD_IF_MATCH(aep, AEP);
#endif
#if HAVE_ENGINE_LOAD_ATALLA
OSSL_ENGINE_LOAD_IF_MATCH(atalla, ATALLA);
#endif
#if HAVE_ENGINE_LOAD_CHIL
OSSL_ENGINE_LOAD_IF_MATCH(chil, CHIL);
#endif
#if HAVE_ENGINE_LOAD_CSWIFT
OSSL_ENGINE_LOAD_IF_MATCH(cswift, CSWIFT);
#endif
#if HAVE_ENGINE_LOAD_NURON
OSSL_ENGINE_LOAD_IF_MATCH(nuron, NURON);
#endif
#if HAVE_ENGINE_LOAD_SUREWARE
OSSL_ENGINE_LOAD_IF_MATCH(sureware, SUREWARE);
#endif
#if HAVE_ENGINE_LOAD_UBSEC
OSSL_ENGINE_LOAD_IF_MATCH(ubsec, UBSEC);
#endif
#if HAVE_ENGINE_LOAD_PADLOCK
OSSL_ENGINE_LOAD_IF_MATCH(padlock, PADLOCK);
#endif
#if HAVE_ENGINE_LOAD_CAPI
OSSL_ENGINE_LOAD_IF_MATCH(capi, CAPI);
#endif
#if HAVE_ENGINE_LOAD_GMP
OSSL_ENGINE_LOAD_IF_MATCH(gmp, GMP);
#endif
#if HAVE_ENGINE_LOAD_GOST
OSSL_ENGINE_LOAD_IF_MATCH(gost, GOST);
#endif
#endif
#if HAVE_ENGINE_LOAD_CRYPTODEV
OSSL_ENGINE_LOAD_IF_MATCH(cryptodev, CRYPTODEV);
#endif
OSSL_ENGINE_LOAD_IF_MATCH(openssl, OPENSSL);
rb_warning("no such builtin loader for `%"PRIsVALUE"'", name);
return Qnil;
}
static VALUE
ossl_engine_s_cleanup(VALUE self)
{
#if defined(LIBRESSL_VERSION_NUMBER) || OPENSSL_VERSION_NUMBER < 0x10100000
ENGINE_cleanup();
#endif
return Qnil;
}
static VALUE
ossl_engine_s_engines(VALUE klass)
{
ENGINE *e;
VALUE ary, obj;
ary = rb_ary_new();
for(e = ENGINE_get_first(); e; e = ENGINE_get_next(e)){
obj = NewEngine(klass);
ENGINE_up_ref(e);
SetEngine(obj, e);
rb_ary_push(ary, obj);
}
return ary;
}
static VALUE
ossl_engine_s_by_id(VALUE klass, VALUE id)
{
ENGINE *e;
VALUE obj;
StringValueCStr(id);
ossl_engine_s_load(1, &id, klass);
obj = NewEngine(klass);
if(!(e = ENGINE_by_id(RSTRING_PTR(id))))
ossl_raise(eEngineError, NULL);
SetEngine(obj, e);
if(rb_block_given_p()) rb_yield(obj);
if(!ENGINE_init(e))
ossl_raise(eEngineError, NULL);
ENGINE_ctrl(e, ENGINE_CTRL_SET_PASSWORD_CALLBACK,
0, NULL, (void(*)(void))ossl_pem_passwd_cb);
ossl_clear_error();
return obj;
}
static VALUE
ossl_engine_get_id(VALUE self)
{
ENGINE *e;
GetEngine(self, e);
return rb_str_new2(ENGINE_get_id(e));
}
static VALUE
ossl_engine_get_name(VALUE self)
{
ENGINE *e;
GetEngine(self, e);
return rb_str_new2(ENGINE_get_name(e));
}
static VALUE
ossl_engine_finish(VALUE self)
{
ENGINE *e;
GetEngine(self, e);
if(!ENGINE_finish(e)) ossl_raise(eEngineError, NULL);
return Qnil;
}
static VALUE
ossl_engine_get_cipher(VALUE self, VALUE name)
{
ENGINE *e;
const EVP_CIPHER *ciph, *tmp;
int nid;
tmp = EVP_get_cipherbyname(StringValueCStr(name));
if(!tmp) ossl_raise(eEngineError, "no such cipher `%"PRIsVALUE"'", name);
nid = EVP_CIPHER_nid(tmp);
GetEngine(self, e);
ciph = ENGINE_get_cipher(e, nid);
if(!ciph) ossl_raise(eEngineError, NULL);
return ossl_cipher_new(ciph);
}
static VALUE
ossl_engine_get_digest(VALUE self, VALUE name)
{
ENGINE *e;
const EVP_MD *md, *tmp;
int nid;
tmp = EVP_get_digestbyname(StringValueCStr(name));
if(!tmp) ossl_raise(eEngineError, "no such digest `%"PRIsVALUE"'", name);
nid = EVP_MD_nid(tmp);
GetEngine(self, e);
md = ENGINE_get_digest(e, nid);
if(!md) ossl_raise(eEngineError, NULL);
return ossl_digest_new(md);
}
static VALUE
ossl_engine_load_privkey(int argc, VALUE *argv, VALUE self)
{
ENGINE *e;
EVP_PKEY *pkey;
VALUE id, data, obj;
char *sid, *sdata;
rb_scan_args(argc, argv, "02", &id, &data);
sid = NIL_P(id) ? NULL : StringValueCStr(id);
sdata = NIL_P(data) ? NULL : StringValueCStr(data);
GetEngine(self, e);
pkey = ENGINE_load_private_key(e, sid, NULL, sdata);
if (!pkey) ossl_raise(eEngineError, NULL);
obj = ossl_pkey_new(pkey);
OSSL_PKEY_SET_PRIVATE(obj);
return obj;
}
static VALUE
ossl_engine_load_pubkey(int argc, VALUE *argv, VALUE self)
{
ENGINE *e;
EVP_PKEY *pkey;
VALUE id, data;
char *sid, *sdata;
rb_scan_args(argc, argv, "02", &id, &data);
sid = NIL_P(id) ? NULL : StringValueCStr(id);
sdata = NIL_P(data) ? NULL : StringValueCStr(data);
GetEngine(self, e);
pkey = ENGINE_load_public_key(e, sid, NULL, sdata);
if (!pkey) ossl_raise(eEngineError, NULL);
return ossl_pkey_new(pkey);
}
static VALUE
ossl_engine_set_default(VALUE self, VALUE flag)
{
ENGINE *e;
int f = NUM2INT(flag);
GetEngine(self, e);
ENGINE_set_default(e, f);
return Qtrue;
}
static VALUE
ossl_engine_ctrl_cmd(int argc, VALUE *argv, VALUE self)
{
ENGINE *e;
VALUE cmd, val;
int ret;
GetEngine(self, e);
rb_scan_args(argc, argv, "11", &cmd, &val);
ret = ENGINE_ctrl_cmd_string(e, StringValueCStr(cmd),
NIL_P(val) ? NULL : StringValueCStr(val), 0);
if (!ret) ossl_raise(eEngineError, NULL);
return self;
}
static VALUE
ossl_engine_cmd_flag_to_name(int flag)
{
switch(flag){
case ENGINE_CMD_FLAG_NUMERIC: return rb_str_new2("NUMERIC");
case ENGINE_CMD_FLAG_STRING: return rb_str_new2("STRING");
case ENGINE_CMD_FLAG_NO_INPUT: return rb_str_new2("NO_INPUT");
case ENGINE_CMD_FLAG_INTERNAL: return rb_str_new2("INTERNAL");
default: return rb_str_new2("UNKNOWN");
}
}
static VALUE
ossl_engine_get_cmds(VALUE self)
{
ENGINE *e;
const ENGINE_CMD_DEFN *defn, *p;
VALUE ary, tmp;
GetEngine(self, e);
ary = rb_ary_new();
if ((defn = ENGINE_get_cmd_defns(e)) != NULL){
for (p = defn; p->cmd_num > 0; p++){
tmp = rb_ary_new();
rb_ary_push(tmp, rb_str_new2(p->cmd_name));
rb_ary_push(tmp, rb_str_new2(p->cmd_desc));
rb_ary_push(tmp, ossl_engine_cmd_flag_to_name(p->cmd_flags));
rb_ary_push(ary, tmp);
}
}
return ary;
}
static VALUE
ossl_engine_inspect(VALUE self)
{
ENGINE *e;
GetEngine(self, e);
return rb_sprintf("#<%"PRIsVALUE" id=\"%s\" name=\"%s\">",
rb_obj_class(self), ENGINE_get_id(e), ENGINE_get_name(e));
}
#define DefEngineConst(x) rb_define_const(cEngine, #x, INT2NUM(ENGINE_##x))
void
Init_ossl_engine(void)
{
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
#endif
cEngine = rb_define_class_under(mOSSL, "Engine", rb_cObject);
eEngineError = rb_define_class_under(cEngine, "EngineError", eOSSLError);
rb_undef_alloc_func(cEngine);
rb_define_singleton_method(cEngine, "load", ossl_engine_s_load, -1);
rb_define_singleton_method(cEngine, "cleanup", ossl_engine_s_cleanup, 0);
rb_define_singleton_method(cEngine, "engines", ossl_engine_s_engines, 0);
rb_define_singleton_method(cEngine, "by_id", ossl_engine_s_by_id, 1);
rb_define_method(cEngine, "id", ossl_engine_get_id, 0);
rb_define_method(cEngine, "name", ossl_engine_get_name, 0);
rb_define_method(cEngine, "finish", ossl_engine_finish, 0);
rb_define_method(cEngine, "cipher", ossl_engine_get_cipher, 1);
rb_define_method(cEngine, "digest", ossl_engine_get_digest, 1);
rb_define_method(cEngine, "load_private_key", ossl_engine_load_privkey, -1);
rb_define_method(cEngine, "load_public_key", ossl_engine_load_pubkey, -1);
rb_define_method(cEngine, "set_default", ossl_engine_set_default, 1);
rb_define_method(cEngine, "ctrl_cmd", ossl_engine_ctrl_cmd, -1);
rb_define_method(cEngine, "cmds", ossl_engine_get_cmds, 0);
rb_define_method(cEngine, "inspect", ossl_engine_inspect, 0);
DefEngineConst(METHOD_RSA);
DefEngineConst(METHOD_DSA);
DefEngineConst(METHOD_DH);
DefEngineConst(METHOD_RAND);
#if defined(ENGINE_METHOD_BN_MOD_EXP)
DefEngineConst(METHOD_BN_MOD_EXP);
#endif
#if defined(ENGINE_METHOD_BN_MOD_EXP_CRT)
DefEngineConst(METHOD_BN_MOD_EXP_CRT);
#endif
DefEngineConst(METHOD_CIPHERS);
DefEngineConst(METHOD_DIGESTS);
DefEngineConst(METHOD_ALL);
DefEngineConst(METHOD_NONE);
}
#else
void
Init_ossl_engine(void)
{
}
#endif
