








#include "ossl.h"

#define NewX509Store(klass) TypedData_Wrap_Struct((klass), &ossl_x509store_type, 0)

#define SetX509Store(obj, st) do { if (!(st)) { ossl_raise(rb_eRuntimeError, "STORE wasn't initialized!"); } RTYPEDDATA_DATA(obj) = (st); } while (0)





#define GetX509Store(obj, st) do { TypedData_Get_Struct((obj), X509_STORE, &ossl_x509store_type, (st)); if (!(st)) { ossl_raise(rb_eRuntimeError, "STORE wasn't initialized!"); } } while (0)






#define NewX509StCtx(klass) TypedData_Wrap_Struct((klass), &ossl_x509stctx_type, 0)

#define SetX509StCtx(obj, ctx) do { if (!(ctx)) { ossl_raise(rb_eRuntimeError, "STORE_CTX wasn't initialized!"); } RTYPEDDATA_DATA(obj) = (ctx); } while (0)





#define GetX509StCtx(obj, ctx) do { TypedData_Get_Struct((obj), X509_STORE_CTX, &ossl_x509stctx_type, (ctx)); if (!(ctx)) { ossl_raise(rb_eRuntimeError, "STORE_CTX is out of scope!"); } } while (0)









static int stctx_ex_verify_cb_idx, store_ex_verify_cb_idx;
static VALUE ossl_x509stctx_new(X509_STORE_CTX *);

struct ossl_verify_cb_args {
VALUE proc;
VALUE preverify_ok;
VALUE store_ctx;
};

static VALUE
call_verify_cb_proc(struct ossl_verify_cb_args *args)
{
return rb_funcall(args->proc, rb_intern("call"), 2,
args->preverify_ok, args->store_ctx);
}

int
ossl_verify_cb_call(VALUE proc, int ok, X509_STORE_CTX *ctx)
{
VALUE rctx, ret;
struct ossl_verify_cb_args args;
int state;

if (NIL_P(proc))
return ok;

ret = Qfalse;
rctx = rb_protect((VALUE(*)(VALUE))ossl_x509stctx_new, (VALUE)ctx, &state);
if (state) {
rb_set_errinfo(Qnil);
rb_warn("StoreContext initialization failure");
}
else {
args.proc = proc;
args.preverify_ok = ok ? Qtrue : Qfalse;
args.store_ctx = rctx;
ret = rb_protect((VALUE(*)(VALUE))call_verify_cb_proc, (VALUE)&args, &state);
if (state) {
rb_set_errinfo(Qnil);
rb_warn("exception in verify_callback is ignored");
}
RTYPEDDATA_DATA(rctx) = NULL;
}
if (ret == Qtrue) {
X509_STORE_CTX_set_error(ctx, X509_V_OK);
ok = 1;
}
else {
if (X509_STORE_CTX_get_error(ctx) == X509_V_OK)
X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_REJECTED);
ok = 0;
}

return ok;
}




VALUE cX509Store;
VALUE cX509StoreContext;
VALUE eX509StoreError;

static void
ossl_x509store_free(void *ptr)
{
X509_STORE_free(ptr);
}

static const rb_data_type_t ossl_x509store_type = {
"OpenSSL/X509/STORE",
{
0, ossl_x509store_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};




X509_STORE *
GetX509StorePtr(VALUE obj)
{
X509_STORE *store;

GetX509Store(obj, store);

return store;
}




static int
x509store_verify_cb(int ok, X509_STORE_CTX *ctx)
{
VALUE proc;

proc = (VALUE)X509_STORE_CTX_get_ex_data(ctx, stctx_ex_verify_cb_idx);
if (!proc)
proc = (VALUE)X509_STORE_get_ex_data(X509_STORE_CTX_get0_store(ctx),
store_ex_verify_cb_idx);
if (!proc)
return ok;

return ossl_verify_cb_call(proc, ok, ctx);
}

static VALUE
ossl_x509store_alloc(VALUE klass)
{
X509_STORE *store;
VALUE obj;

obj = NewX509Store(klass);
if((store = X509_STORE_new()) == NULL){
ossl_raise(eX509StoreError, NULL);
}
SetX509Store(obj, store);

return obj;
}




static VALUE
ossl_x509store_set_vfy_cb(VALUE self, VALUE cb)
{
X509_STORE *store;

GetX509Store(self, store);
X509_STORE_set_ex_data(store, store_ex_verify_cb_idx, (void *)cb);
rb_iv_set(self, "@verify_callback", cb);

return cb;
}








static VALUE
ossl_x509store_initialize(int argc, VALUE *argv, VALUE self)
{
X509_STORE *store;


GetX509Store(self, store);
#if !defined(HAVE_OPAQUE_OPENSSL)

store->ex_data.sk = NULL;
#endif
X509_STORE_set_verify_cb(store, x509store_verify_cb);
ossl_x509store_set_vfy_cb(self, Qnil);


rb_iv_set(self, "@error", Qnil);
rb_iv_set(self, "@error_string", Qnil);
rb_iv_set(self, "@chain", Qnil);
rb_iv_set(self, "@time", Qnil);

return self;
}








static VALUE
ossl_x509store_set_flags(VALUE self, VALUE flags)
{
X509_STORE *store;
long f = NUM2LONG(flags);

GetX509Store(self, store);
X509_STORE_set_flags(store, f);

return flags;
}



















static VALUE
ossl_x509store_set_purpose(VALUE self, VALUE purpose)
{
X509_STORE *store;
int p = NUM2INT(purpose);

GetX509Store(self, store);
X509_STORE_set_purpose(store, p);

return purpose;
}





static VALUE
ossl_x509store_set_trust(VALUE self, VALUE trust)
{
X509_STORE *store;
int t = NUM2INT(trust);

GetX509Store(self, store);
X509_STORE_set_trust(store, t);

return trust;
}







static VALUE
ossl_x509store_set_time(VALUE self, VALUE time)
{
rb_iv_set(self, "@time", time);
return time;
}









static VALUE
ossl_x509store_add_file(VALUE self, VALUE file)
{
X509_STORE *store;
X509_LOOKUP *lookup;
char *path = NULL;

if(file != Qnil){
path = StringValueCStr(file);
}
GetX509Store(self, store);
lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
if(lookup == NULL) ossl_raise(eX509StoreError, NULL);
if(X509_LOOKUP_load_file(lookup, path, X509_FILETYPE_PEM) != 1){
ossl_raise(eX509StoreError, NULL);
}
#if OPENSSL_VERSION_NUMBER < 0x10101000 || defined(LIBRESSL_VERSION_NUMBER)






ossl_clear_error();
#endif

return self;
}







static VALUE
ossl_x509store_add_path(VALUE self, VALUE dir)
{
X509_STORE *store;
X509_LOOKUP *lookup;
char *path = NULL;

if(dir != Qnil){
path = StringValueCStr(dir);
}
GetX509Store(self, store);
lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
if(lookup == NULL) ossl_raise(eX509StoreError, NULL);
if(X509_LOOKUP_add_dir(lookup, path, X509_FILETYPE_PEM) != 1){
ossl_raise(eX509StoreError, NULL);
}

return self;
}












static VALUE
ossl_x509store_set_default_paths(VALUE self)
{
X509_STORE *store;

GetX509Store(self, store);
if (X509_STORE_set_default_paths(store) != 1){
ossl_raise(eX509StoreError, NULL);
}

return Qnil;
}







static VALUE
ossl_x509store_add_cert(VALUE self, VALUE arg)
{
X509_STORE *store;
X509 *cert;

cert = GetX509CertPtr(arg); 
GetX509Store(self, store);
if (X509_STORE_add_cert(store, cert) != 1){
ossl_raise(eX509StoreError, NULL);
}

return self;
}







static VALUE
ossl_x509store_add_crl(VALUE self, VALUE arg)
{
X509_STORE *store;
X509_CRL *crl;

crl = GetX509CRLPtr(arg); 
GetX509Store(self, store);
if (X509_STORE_add_crl(store, crl) != 1){
ossl_raise(eX509StoreError, NULL);
}

return self;
}

static VALUE ossl_x509stctx_get_err(VALUE);
static VALUE ossl_x509stctx_get_err_string(VALUE);
static VALUE ossl_x509stctx_get_chain(VALUE);
















static VALUE
ossl_x509store_verify(int argc, VALUE *argv, VALUE self)
{
VALUE cert, chain;
VALUE ctx, proc, result;

rb_scan_args(argc, argv, "11", &cert, &chain);
ctx = rb_funcall(cX509StoreContext, rb_intern("new"), 3, self, cert, chain);
proc = rb_block_given_p() ? rb_block_proc() :
rb_iv_get(self, "@verify_callback");
rb_iv_set(ctx, "@verify_callback", proc);
result = rb_funcall(ctx, rb_intern("verify"), 0);

rb_iv_set(self, "@error", ossl_x509stctx_get_err(ctx));
rb_iv_set(self, "@error_string", ossl_x509stctx_get_err_string(ctx));
rb_iv_set(self, "@chain", ossl_x509stctx_get_chain(ctx));

return result;
}




static void ossl_x509stctx_free(void*);


static const rb_data_type_t ossl_x509stctx_type = {
"OpenSSL/X509/STORE_CTX",
{
0, ossl_x509stctx_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};




static void
ossl_x509stctx_free(void *ptr)
{
X509_STORE_CTX *ctx = ptr;
if (X509_STORE_CTX_get0_untrusted(ctx))
sk_X509_pop_free(X509_STORE_CTX_get0_untrusted(ctx), X509_free);
if (X509_STORE_CTX_get0_cert(ctx))
X509_free(X509_STORE_CTX_get0_cert(ctx));
X509_STORE_CTX_free(ctx);
}

static VALUE
ossl_x509stctx_alloc(VALUE klass)
{
X509_STORE_CTX *ctx;
VALUE obj;

obj = NewX509StCtx(klass);
if((ctx = X509_STORE_CTX_new()) == NULL){
ossl_raise(eX509StoreError, NULL);
}
SetX509StCtx(obj, ctx);

return obj;
}

static VALUE
ossl_x509stctx_new(X509_STORE_CTX *ctx)
{
VALUE obj;

obj = NewX509StCtx(cX509StoreContext);
SetX509StCtx(obj, ctx);

return obj;
}

static VALUE ossl_x509stctx_set_flags(VALUE, VALUE);
static VALUE ossl_x509stctx_set_purpose(VALUE, VALUE);
static VALUE ossl_x509stctx_set_trust(VALUE, VALUE);
static VALUE ossl_x509stctx_set_time(VALUE, VALUE);





static VALUE
ossl_x509stctx_initialize(int argc, VALUE *argv, VALUE self)
{
VALUE store, cert, chain, t;
X509_STORE_CTX *ctx;
X509_STORE *x509st;
X509 *x509 = NULL;
STACK_OF(X509) *x509s = NULL;

rb_scan_args(argc, argv, "12", &store, &cert, &chain);
GetX509StCtx(self, ctx);
GetX509Store(store, x509st);
if(!NIL_P(cert)) x509 = DupX509CertPtr(cert); 
if(!NIL_P(chain)) x509s = ossl_x509_ary2sk(chain);
if(X509_STORE_CTX_init(ctx, x509st, x509, x509s) != 1){
sk_X509_pop_free(x509s, X509_free);
ossl_raise(eX509StoreError, NULL);
}
if (!NIL_P(t = rb_iv_get(store, "@time")))
ossl_x509stctx_set_time(self, t);
rb_iv_set(self, "@verify_callback", rb_iv_get(store, "@verify_callback"));
rb_iv_set(self, "@cert", cert);

return self;
}





static VALUE
ossl_x509stctx_verify(VALUE self)
{
X509_STORE_CTX *ctx;

GetX509StCtx(self, ctx);
X509_STORE_CTX_set_ex_data(ctx, stctx_ex_verify_cb_idx,
(void *)rb_iv_get(self, "@verify_callback"));

switch (X509_verify_cert(ctx)) {
case 1:
return Qtrue;
case 0:
ossl_clear_error();
return Qfalse;
default:
ossl_raise(eX509CertError, NULL);
}
}





static VALUE
ossl_x509stctx_get_chain(VALUE self)
{
X509_STORE_CTX *ctx;
STACK_OF(X509) *chain;
X509 *x509;
int i, num;
VALUE ary;

GetX509StCtx(self, ctx);
if((chain = X509_STORE_CTX_get0_chain(ctx)) == NULL){
return Qnil;
}
if((num = sk_X509_num(chain)) < 0){
OSSL_Debug("certs in chain < 0???");
return rb_ary_new();
}
ary = rb_ary_new2(num);
for(i = 0; i < num; i++) {
x509 = sk_X509_value(chain, i);
rb_ary_push(ary, ossl_x509_new(x509));
}

return ary;
}





static VALUE
ossl_x509stctx_get_err(VALUE self)
{
X509_STORE_CTX *ctx;

GetX509StCtx(self, ctx);

return INT2NUM(X509_STORE_CTX_get_error(ctx));
}





static VALUE
ossl_x509stctx_set_error(VALUE self, VALUE err)
{
X509_STORE_CTX *ctx;

GetX509StCtx(self, ctx);
X509_STORE_CTX_set_error(ctx, NUM2INT(err));

return err;
}







static VALUE
ossl_x509stctx_get_err_string(VALUE self)
{
X509_STORE_CTX *ctx;
long err;

GetX509StCtx(self, ctx);
err = X509_STORE_CTX_get_error(ctx);

return rb_str_new2(X509_verify_cert_error_string(err));
}





static VALUE
ossl_x509stctx_get_err_depth(VALUE self)
{
X509_STORE_CTX *ctx;

GetX509StCtx(self, ctx);

return INT2NUM(X509_STORE_CTX_get_error_depth(ctx));
}





static VALUE
ossl_x509stctx_get_curr_cert(VALUE self)
{
X509_STORE_CTX *ctx;

GetX509StCtx(self, ctx);

return ossl_x509_new(X509_STORE_CTX_get_current_cert(ctx));
}





static VALUE
ossl_x509stctx_get_curr_crl(VALUE self)
{
X509_STORE_CTX *ctx;
X509_CRL *crl;

GetX509StCtx(self, ctx);
crl = X509_STORE_CTX_get0_current_crl(ctx);
if (!crl)
return Qnil;

return ossl_x509crl_new(crl);
}







static VALUE
ossl_x509stctx_set_flags(VALUE self, VALUE flags)
{
X509_STORE_CTX *store;
long f = NUM2LONG(flags);

GetX509StCtx(self, store);
X509_STORE_CTX_set_flags(store, f);

return flags;
}







static VALUE
ossl_x509stctx_set_purpose(VALUE self, VALUE purpose)
{
X509_STORE_CTX *store;
int p = NUM2INT(purpose);

GetX509StCtx(self, store);
X509_STORE_CTX_set_purpose(store, p);

return purpose;
}





static VALUE
ossl_x509stctx_set_trust(VALUE self, VALUE trust)
{
X509_STORE_CTX *store;
int t = NUM2INT(trust);

GetX509StCtx(self, store);
X509_STORE_CTX_set_trust(store, t);

return trust;
}







static VALUE
ossl_x509stctx_set_time(VALUE self, VALUE time)
{
X509_STORE_CTX *store;
long t;

t = NUM2LONG(rb_Integer(time));
GetX509StCtx(self, store);
X509_STORE_CTX_set_time(store, 0, t);

return time;
}




void
Init_ossl_x509store(void)
{
#undef rb_intern
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
mX509 = rb_define_module_under(mOSSL, "X509");
#endif


stctx_ex_verify_cb_idx = X509_STORE_CTX_get_ex_new_index(0, (void *)"stctx_ex_verify_cb_idx", 0, 0, 0);
if (stctx_ex_verify_cb_idx < 0)
ossl_raise(eOSSLError, "X509_STORE_CTX_get_ex_new_index");
store_ex_verify_cb_idx = X509_STORE_get_ex_new_index(0, (void *)"store_ex_verify_cb_idx", 0, 0, 0);
if (store_ex_verify_cb_idx < 0)
ossl_raise(eOSSLError, "X509_STORE_get_ex_new_index");

eX509StoreError = rb_define_class_under(mX509, "StoreError", eOSSLError);




































cX509Store = rb_define_class_under(mX509, "Store", rb_cObject);








rb_attr(cX509Store, rb_intern("verify_callback"), 1, 0, Qfalse);



rb_attr(cX509Store, rb_intern("error"), 1, 0, Qfalse);



rb_attr(cX509Store, rb_intern("error_string"), 1, 0, Qfalse);



rb_attr(cX509Store, rb_intern("chain"), 1, 0, Qfalse);
rb_define_alloc_func(cX509Store, ossl_x509store_alloc);
rb_define_method(cX509Store, "initialize", ossl_x509store_initialize, -1);
rb_undef_method(cX509Store, "initialize_copy");
rb_define_method(cX509Store, "verify_callback=", ossl_x509store_set_vfy_cb, 1);
rb_define_method(cX509Store, "flags=", ossl_x509store_set_flags, 1);
rb_define_method(cX509Store, "purpose=", ossl_x509store_set_purpose, 1);
rb_define_method(cX509Store, "trust=", ossl_x509store_set_trust, 1);
rb_define_method(cX509Store, "time=", ossl_x509store_set_time, 1);
rb_define_method(cX509Store, "add_path", ossl_x509store_add_path, 1);
rb_define_method(cX509Store, "add_file", ossl_x509store_add_file, 1);
rb_define_method(cX509Store, "set_default_paths", ossl_x509store_set_default_paths, 0);
rb_define_method(cX509Store, "add_cert", ossl_x509store_add_cert, 1);
rb_define_method(cX509Store, "add_crl", ossl_x509store_add_crl, 1);
rb_define_method(cX509Store, "verify", ossl_x509store_verify, -1);







cX509StoreContext = rb_define_class_under(mX509,"StoreContext", rb_cObject);
rb_define_alloc_func(cX509StoreContext, ossl_x509stctx_alloc);
rb_define_method(cX509StoreContext, "initialize", ossl_x509stctx_initialize, -1);
rb_undef_method(cX509StoreContext, "initialize_copy");
rb_define_method(cX509StoreContext, "verify", ossl_x509stctx_verify, 0);
rb_define_method(cX509StoreContext, "chain", ossl_x509stctx_get_chain,0);
rb_define_method(cX509StoreContext, "error", ossl_x509stctx_get_err, 0);
rb_define_method(cX509StoreContext, "error=", ossl_x509stctx_set_error, 1);
rb_define_method(cX509StoreContext, "error_string", ossl_x509stctx_get_err_string,0);
rb_define_method(cX509StoreContext, "error_depth", ossl_x509stctx_get_err_depth, 0);
rb_define_method(cX509StoreContext, "current_cert", ossl_x509stctx_get_curr_cert, 0);
rb_define_method(cX509StoreContext, "current_crl", ossl_x509stctx_get_curr_crl, 0);
rb_define_method(cX509StoreContext, "flags=", ossl_x509stctx_set_flags, 1);
rb_define_method(cX509StoreContext, "purpose=", ossl_x509stctx_set_purpose, 1);
rb_define_method(cX509StoreContext, "trust=", ossl_x509stctx_set_trust, 1);
rb_define_method(cX509StoreContext, "time=", ossl_x509stctx_set_time, 1);
}
