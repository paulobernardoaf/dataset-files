#include "ossl.h"
#define numberof(ary) (int)(sizeof(ary)/sizeof((ary)[0]))
#if defined(_WIN32)
#define TO_SOCKET(s) _get_osfhandle(s)
#else
#define TO_SOCKET(s) (s)
#endif
#define GetSSLCTX(obj, ctx) do { TypedData_Get_Struct((obj), SSL_CTX, &ossl_sslctx_type, (ctx)); } while (0)
VALUE mSSL;
static VALUE mSSLExtConfig;
static VALUE eSSLError;
VALUE cSSLContext;
VALUE cSSLSocket;
static VALUE eSSLErrorWaitReadable;
static VALUE eSSLErrorWaitWritable;
static ID id_call, ID_callback_state, id_tmp_dh_callback, id_tmp_ecdh_callback,
id_npn_protocols_encoded;
static VALUE sym_exception, sym_wait_readable, sym_wait_writable;
static ID id_i_cert_store, id_i_ca_file, id_i_ca_path, id_i_verify_mode,
id_i_verify_depth, id_i_verify_callback, id_i_client_ca,
id_i_renegotiation_cb, id_i_cert, id_i_key, id_i_extra_chain_cert,
id_i_client_cert_cb, id_i_tmp_ecdh_callback, id_i_timeout,
id_i_session_id_context, id_i_session_get_cb, id_i_session_new_cb,
id_i_session_remove_cb, id_i_npn_select_cb, id_i_npn_protocols,
id_i_alpn_select_cb, id_i_alpn_protocols, id_i_servername_cb,
id_i_verify_hostname;
static ID id_i_io, id_i_context, id_i_hostname;
static int ossl_ssl_ex_vcb_idx;
static int ossl_ssl_ex_ptr_idx;
static int ossl_sslctx_ex_ptr_idx;
#if !defined(HAVE_X509_STORE_UP_REF)
static int ossl_sslctx_ex_store_p;
#endif
static void
ossl_sslctx_free(void *ptr)
{
SSL_CTX *ctx = ptr;
#if !defined(HAVE_X509_STORE_UP_REF)
if (ctx && SSL_CTX_get_ex_data(ctx, ossl_sslctx_ex_store_p))
ctx->cert_store = NULL;
#endif
SSL_CTX_free(ctx);
}
static const rb_data_type_t ossl_sslctx_type = {
"OpenSSL/SSL/CTX",
{
0, ossl_sslctx_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};
static VALUE
ossl_sslctx_s_alloc(VALUE klass)
{
SSL_CTX *ctx;
long mode = 0 |
SSL_MODE_ENABLE_PARTIAL_WRITE |
SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER |
SSL_MODE_RELEASE_BUFFERS;
VALUE obj;
obj = TypedData_Wrap_Struct(klass, &ossl_sslctx_type, 0);
#if OPENSSL_VERSION_NUMBER >= 0x10100000 && !defined(LIBRESSL_VERSION_NUMBER)
ctx = SSL_CTX_new(TLS_method());
#else
ctx = SSL_CTX_new(SSLv23_method());
#endif
if (!ctx) {
ossl_raise(eSSLError, "SSL_CTX_new");
}
SSL_CTX_set_mode(ctx, mode);
RTYPEDDATA_DATA(obj) = ctx;
SSL_CTX_set_ex_data(ctx, ossl_sslctx_ex_ptr_idx, (void *)obj);
#if !defined(OPENSSL_NO_EC) && defined(HAVE_SSL_CTX_SET_ECDH_AUTO)
if (!SSL_CTX_set_ecdh_auto(ctx, 1))
ossl_raise(eSSLError, "SSL_CTX_set_ecdh_auto");
#endif
return obj;
}
static int
parse_proto_version(VALUE str)
{
int i;
static const struct {
const char *name;
int version;
} map[] = {
{ "SSL2", SSL2_VERSION },
{ "SSL3", SSL3_VERSION },
{ "TLS1", TLS1_VERSION },
{ "TLS1_1", TLS1_1_VERSION },
{ "TLS1_2", TLS1_2_VERSION },
#if defined(TLS1_3_VERSION)
{ "TLS1_3", TLS1_3_VERSION },
#endif
};
if (NIL_P(str))
return 0;
if (RB_INTEGER_TYPE_P(str))
return NUM2INT(str);
if (SYMBOL_P(str))
str = rb_sym2str(str);
StringValue(str);
for (i = 0; i < numberof(map); i++)
if (!strncmp(map[i].name, RSTRING_PTR(str), RSTRING_LEN(str)))
return map[i].version;
rb_raise(rb_eArgError, "unrecognized version %+"PRIsVALUE, str);
}
static VALUE
ossl_sslctx_set_minmax_proto_version(VALUE self, VALUE min_v, VALUE max_v)
{
SSL_CTX *ctx;
int min, max;
GetSSLCTX(self, ctx);
min = parse_proto_version(min_v);
max = parse_proto_version(max_v);
#if defined(HAVE_SSL_CTX_SET_MIN_PROTO_VERSION)
if (!SSL_CTX_set_min_proto_version(ctx, min))
ossl_raise(eSSLError, "SSL_CTX_set_min_proto_version");
if (!SSL_CTX_set_max_proto_version(ctx, max))
ossl_raise(eSSLError, "SSL_CTX_set_max_proto_version");
#else
{
unsigned long sum = 0, opts = 0;
int i;
static const struct {
int ver;
unsigned long opts;
} options_map[] = {
{ SSL2_VERSION, SSL_OP_NO_SSLv2 },
{ SSL3_VERSION, SSL_OP_NO_SSLv3 },
{ TLS1_VERSION, SSL_OP_NO_TLSv1 },
{ TLS1_1_VERSION, SSL_OP_NO_TLSv1_1 },
{ TLS1_2_VERSION, SSL_OP_NO_TLSv1_2 },
#if defined(TLS1_3_VERSION)
{ TLS1_3_VERSION, SSL_OP_NO_TLSv1_3 },
#endif
};
for (i = 0; i < numberof(options_map); i++) {
sum |= options_map[i].opts;
if ((min && min > options_map[i].ver) ||
(max && max < options_map[i].ver)) {
opts |= options_map[i].opts;
}
}
SSL_CTX_clear_options(ctx, sum);
SSL_CTX_set_options(ctx, opts);
}
#endif
return Qnil;
}
static VALUE
ossl_call_client_cert_cb(VALUE obj)
{
VALUE ctx_obj, cb, ary, cert, key;
ctx_obj = rb_attr_get(obj, id_i_context);
cb = rb_attr_get(ctx_obj, id_i_client_cert_cb);
if (NIL_P(cb))
return Qnil;
ary = rb_funcallv(cb, id_call, 1, &obj);
Check_Type(ary, T_ARRAY);
GetX509CertPtr(cert = rb_ary_entry(ary, 0));
GetPrivPKeyPtr(key = rb_ary_entry(ary, 1));
return rb_ary_new3(2, cert, key);
}
static int
ossl_client_cert_cb(SSL *ssl, X509 **x509, EVP_PKEY **pkey)
{
VALUE obj, ret;
obj = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
ret = rb_protect(ossl_call_client_cert_cb, obj, NULL);
if (NIL_P(ret))
return 0;
*x509 = DupX509CertPtr(RARRAY_AREF(ret, 0));
*pkey = DupPKeyPtr(RARRAY_AREF(ret, 1));
return 1;
}
#if !defined(OPENSSL_NO_DH) || !defined(OPENSSL_NO_EC) && defined(HAVE_SSL_CTX_SET_TMP_ECDH_CALLBACK)
struct tmp_dh_callback_args {
VALUE ssl_obj;
ID id;
int type;
int is_export;
int keylength;
};
static EVP_PKEY *
ossl_call_tmp_dh_callback(struct tmp_dh_callback_args *args)
{
VALUE cb, dh;
EVP_PKEY *pkey;
cb = rb_funcall(args->ssl_obj, args->id, 0);
if (NIL_P(cb))
return NULL;
dh = rb_funcall(cb, id_call, 3, args->ssl_obj, INT2NUM(args->is_export),
INT2NUM(args->keylength));
pkey = GetPKeyPtr(dh);
if (EVP_PKEY_base_id(pkey) != args->type)
return NULL;
return pkey;
}
#endif
#if !defined(OPENSSL_NO_DH)
static DH *
ossl_tmp_dh_callback(SSL *ssl, int is_export, int keylength)
{
VALUE rb_ssl;
EVP_PKEY *pkey;
struct tmp_dh_callback_args args;
int state;
rb_ssl = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
args.ssl_obj = rb_ssl;
args.id = id_tmp_dh_callback;
args.is_export = is_export;
args.keylength = keylength;
args.type = EVP_PKEY_DH;
pkey = (EVP_PKEY *)rb_protect((VALUE (*)(VALUE))ossl_call_tmp_dh_callback,
(VALUE)&args, &state);
if (state) {
rb_ivar_set(rb_ssl, ID_callback_state, INT2NUM(state));
return NULL;
}
if (!pkey)
return NULL;
return EVP_PKEY_get0_DH(pkey);
}
#endif 
#if !defined(OPENSSL_NO_EC) && defined(HAVE_SSL_CTX_SET_TMP_ECDH_CALLBACK)
static EC_KEY *
ossl_tmp_ecdh_callback(SSL *ssl, int is_export, int keylength)
{
VALUE rb_ssl;
EVP_PKEY *pkey;
struct tmp_dh_callback_args args;
int state;
rb_ssl = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
args.ssl_obj = rb_ssl;
args.id = id_tmp_ecdh_callback;
args.is_export = is_export;
args.keylength = keylength;
args.type = EVP_PKEY_EC;
pkey = (EVP_PKEY *)rb_protect((VALUE (*)(VALUE))ossl_call_tmp_dh_callback,
(VALUE)&args, &state);
if (state) {
rb_ivar_set(rb_ssl, ID_callback_state, INT2NUM(state));
return NULL;
}
if (!pkey)
return NULL;
return EVP_PKEY_get0_EC_KEY(pkey);
}
#endif
static VALUE
call_verify_certificate_identity(VALUE ctx_v)
{
X509_STORE_CTX *ctx = (X509_STORE_CTX *)ctx_v;
SSL *ssl;
VALUE ssl_obj, hostname, cert_obj;
ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
ssl_obj = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
hostname = rb_attr_get(ssl_obj, id_i_hostname);
if (!RTEST(hostname)) {
rb_warning("verify_hostname requires hostname to be set");
return Qtrue;
}
cert_obj = ossl_x509_new(X509_STORE_CTX_get_current_cert(ctx));
return rb_funcall(mSSL, rb_intern("verify_certificate_identity"), 2,
cert_obj, hostname);
}
static int
ossl_ssl_verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
VALUE cb, ssl_obj, sslctx_obj, verify_hostname, ret;
SSL *ssl;
int status;
ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
cb = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_vcb_idx);
ssl_obj = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
sslctx_obj = rb_attr_get(ssl_obj, id_i_context);
verify_hostname = rb_attr_get(sslctx_obj, id_i_verify_hostname);
if (preverify_ok && RTEST(verify_hostname) && !SSL_is_server(ssl) &&
!X509_STORE_CTX_get_error_depth(ctx)) {
ret = rb_protect(call_verify_certificate_identity, (VALUE)ctx, &status);
if (status) {
rb_ivar_set(ssl_obj, ID_callback_state, INT2NUM(status));
return 0;
}
if (ret != Qtrue) {
preverify_ok = 0;
#if defined(X509_V_ERR_HOSTNAME_MISMATCH)
X509_STORE_CTX_set_error(ctx, X509_V_ERR_HOSTNAME_MISMATCH);
#else
X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_REJECTED);
#endif
}
}
return ossl_verify_cb_call(cb, preverify_ok, ctx);
}
static VALUE
ossl_call_session_get_cb(VALUE ary)
{
VALUE ssl_obj, cb;
Check_Type(ary, T_ARRAY);
ssl_obj = rb_ary_entry(ary, 0);
cb = rb_funcall(ssl_obj, rb_intern("session_get_cb"), 0);
if (NIL_P(cb)) return Qnil;
return rb_funcallv(cb, id_call, 1, &ary);
}
static SSL_SESSION *
#if (!defined(LIBRESSL_VERSION_NUMBER) ? OPENSSL_VERSION_NUMBER >= 0x10100000 : LIBRESSL_VERSION_NUMBER >= 0x2080000f)
ossl_sslctx_session_get_cb(SSL *ssl, const unsigned char *buf, int len, int *copy)
#else
ossl_sslctx_session_get_cb(SSL *ssl, unsigned char *buf, int len, int *copy)
#endif
{
VALUE ary, ssl_obj, ret_obj;
SSL_SESSION *sess;
int state = 0;
OSSL_Debug("SSL SESSION get callback entered");
ssl_obj = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
ary = rb_ary_new2(2);
rb_ary_push(ary, ssl_obj);
rb_ary_push(ary, rb_str_new((const char *)buf, len));
ret_obj = rb_protect(ossl_call_session_get_cb, ary, &state);
if (state) {
rb_ivar_set(ssl_obj, ID_callback_state, INT2NUM(state));
return NULL;
}
if (!rb_obj_is_instance_of(ret_obj, cSSLSession))
return NULL;
GetSSLSession(ret_obj, sess);
*copy = 1;
return sess;
}
static VALUE
ossl_call_session_new_cb(VALUE ary)
{
VALUE ssl_obj, cb;
Check_Type(ary, T_ARRAY);
ssl_obj = rb_ary_entry(ary, 0);
cb = rb_funcall(ssl_obj, rb_intern("session_new_cb"), 0);
if (NIL_P(cb)) return Qnil;
return rb_funcallv(cb, id_call, 1, &ary);
}
static int
ossl_sslctx_session_new_cb(SSL *ssl, SSL_SESSION *sess)
{
VALUE ary, ssl_obj, sess_obj;
int state = 0;
OSSL_Debug("SSL SESSION new callback entered");
ssl_obj = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
sess_obj = rb_obj_alloc(cSSLSession);
SSL_SESSION_up_ref(sess);
DATA_PTR(sess_obj) = sess;
ary = rb_ary_new2(2);
rb_ary_push(ary, ssl_obj);
rb_ary_push(ary, sess_obj);
rb_protect(ossl_call_session_new_cb, ary, &state);
if (state) {
rb_ivar_set(ssl_obj, ID_callback_state, INT2NUM(state));
}
return 0;
}
static VALUE
ossl_call_session_remove_cb(VALUE ary)
{
VALUE sslctx_obj, cb;
Check_Type(ary, T_ARRAY);
sslctx_obj = rb_ary_entry(ary, 0);
cb = rb_attr_get(sslctx_obj, id_i_session_remove_cb);
if (NIL_P(cb)) return Qnil;
return rb_funcallv(cb, id_call, 1, &ary);
}
static void
ossl_sslctx_session_remove_cb(SSL_CTX *ctx, SSL_SESSION *sess)
{
VALUE ary, sslctx_obj, sess_obj;
int state = 0;
if (rb_during_gc())
return;
OSSL_Debug("SSL SESSION remove callback entered");
sslctx_obj = (VALUE)SSL_CTX_get_ex_data(ctx, ossl_sslctx_ex_ptr_idx);
sess_obj = rb_obj_alloc(cSSLSession);
SSL_SESSION_up_ref(sess);
DATA_PTR(sess_obj) = sess;
ary = rb_ary_new2(2);
rb_ary_push(ary, sslctx_obj);
rb_ary_push(ary, sess_obj);
rb_protect(ossl_call_session_remove_cb, ary, &state);
if (state) {
}
}
static VALUE
ossl_sslctx_add_extra_chain_cert_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, arg))
{
X509 *x509;
SSL_CTX *ctx;
GetSSLCTX(arg, ctx);
x509 = DupX509CertPtr(i);
if(!SSL_CTX_add_extra_chain_cert(ctx, x509)){
ossl_raise(eSSLError, NULL);
}
return i;
}
static VALUE ossl_sslctx_setup(VALUE self);
static VALUE
ossl_call_servername_cb(VALUE ary)
{
VALUE ssl_obj, sslctx_obj, cb, ret_obj;
Check_Type(ary, T_ARRAY);
ssl_obj = rb_ary_entry(ary, 0);
sslctx_obj = rb_attr_get(ssl_obj, id_i_context);
cb = rb_attr_get(sslctx_obj, id_i_servername_cb);
if (NIL_P(cb)) return Qnil;
ret_obj = rb_funcallv(cb, id_call, 1, &ary);
if (rb_obj_is_kind_of(ret_obj, cSSLContext)) {
SSL *ssl;
SSL_CTX *ctx2;
ossl_sslctx_setup(ret_obj);
GetSSL(ssl_obj, ssl);
GetSSLCTX(ret_obj, ctx2);
SSL_set_SSL_CTX(ssl, ctx2);
rb_ivar_set(ssl_obj, id_i_context, ret_obj);
} else if (!NIL_P(ret_obj)) {
ossl_raise(rb_eArgError, "servername_cb must return an "
"OpenSSL::SSL::SSLContext object or nil");
}
return ret_obj;
}
static int
ssl_servername_cb(SSL *ssl, int *ad, void *arg)
{
VALUE ary, ssl_obj;
int state = 0;
const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
if (!servername)
return SSL_TLSEXT_ERR_OK;
ssl_obj = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
ary = rb_ary_new2(2);
rb_ary_push(ary, ssl_obj);
rb_ary_push(ary, rb_str_new2(servername));
rb_protect(ossl_call_servername_cb, ary, &state);
if (state) {
rb_ivar_set(ssl_obj, ID_callback_state, INT2NUM(state));
return SSL_TLSEXT_ERR_ALERT_FATAL;
}
return SSL_TLSEXT_ERR_OK;
}
static void
ssl_renegotiation_cb(const SSL *ssl)
{
VALUE ssl_obj, sslctx_obj, cb;
ssl_obj = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
sslctx_obj = rb_attr_get(ssl_obj, id_i_context);
cb = rb_attr_get(sslctx_obj, id_i_renegotiation_cb);
if (NIL_P(cb)) return;
rb_funcallv(cb, id_call, 1, &ssl_obj);
}
#if !defined(OPENSSL_NO_NEXTPROTONEG) || defined(HAVE_SSL_CTX_SET_ALPN_SELECT_CB)
static VALUE
ssl_npn_encode_protocol_i(RB_BLOCK_CALL_FUNC_ARGLIST(cur, encoded))
{
int len = RSTRING_LENINT(cur);
char len_byte;
if (len < 1 || len > 255)
ossl_raise(eSSLError, "Advertised protocol must have length 1..255");
len_byte = len;
rb_str_buf_cat(encoded, &len_byte, 1);
rb_str_buf_cat(encoded, RSTRING_PTR(cur), len);
return Qnil;
}
static VALUE
ssl_encode_npn_protocols(VALUE protocols)
{
VALUE encoded = rb_str_new(NULL, 0);
rb_iterate(rb_each, protocols, ssl_npn_encode_protocol_i, encoded);
return encoded;
}
struct npn_select_cb_common_args {
VALUE cb;
const unsigned char *in;
unsigned inlen;
};
static VALUE
npn_select_cb_common_i(VALUE tmp)
{
struct npn_select_cb_common_args *args = (void *)tmp;
const unsigned char *in = args->in, *in_end = in + args->inlen;
unsigned char l;
long len;
VALUE selected, protocols = rb_ary_new();
while (in < in_end) {
l = *in++;
rb_ary_push(protocols, rb_str_new((const char *)in, l));
in += l;
}
selected = rb_funcallv(args->cb, id_call, 1, &protocols);
StringValue(selected);
len = RSTRING_LEN(selected);
if (len < 1 || len >= 256) {
ossl_raise(eSSLError, "Selected protocol name must have length 1..255");
}
return selected;
}
static int
ssl_npn_select_cb_common(SSL *ssl, VALUE cb, const unsigned char **out,
unsigned char *outlen, const unsigned char *in,
unsigned int inlen)
{
VALUE selected;
int status;
struct npn_select_cb_common_args args;
args.cb = cb;
args.in = in;
args.inlen = inlen;
selected = rb_protect(npn_select_cb_common_i, (VALUE)&args, &status);
if (status) {
VALUE ssl_obj = (VALUE)SSL_get_ex_data(ssl, ossl_ssl_ex_ptr_idx);
rb_ivar_set(ssl_obj, ID_callback_state, INT2NUM(status));
return SSL_TLSEXT_ERR_ALERT_FATAL;
}
*out = (unsigned char *)RSTRING_PTR(selected);
*outlen = (unsigned char)RSTRING_LEN(selected);
return SSL_TLSEXT_ERR_OK;
}
#endif
#if !defined(OPENSSL_NO_NEXTPROTONEG)
static int
ssl_npn_advertise_cb(SSL *ssl, const unsigned char **out, unsigned int *outlen,
void *arg)
{
VALUE protocols = (VALUE)arg;
*out = (const unsigned char *) RSTRING_PTR(protocols);
*outlen = RSTRING_LENINT(protocols);
return SSL_TLSEXT_ERR_OK;
}
static int
ssl_npn_select_cb(SSL *ssl, unsigned char **out, unsigned char *outlen,
const unsigned char *in, unsigned int inlen, void *arg)
{
VALUE sslctx_obj, cb;
sslctx_obj = (VALUE) arg;
cb = rb_attr_get(sslctx_obj, id_i_npn_select_cb);
return ssl_npn_select_cb_common(ssl, cb, (const unsigned char **)out,
outlen, in, inlen);
}
#endif
#if defined(HAVE_SSL_CTX_SET_ALPN_SELECT_CB)
static int
ssl_alpn_select_cb(SSL *ssl, const unsigned char **out, unsigned char *outlen,
const unsigned char *in, unsigned int inlen, void *arg)
{
VALUE sslctx_obj, cb;
sslctx_obj = (VALUE) arg;
cb = rb_attr_get(sslctx_obj, id_i_alpn_select_cb);
return ssl_npn_select_cb_common(ssl, cb, out, outlen, in, inlen);
}
#endif
static void
ssl_info_cb(const SSL *ssl, int where, int val)
{
int is_server = SSL_is_server((SSL *)ssl);
if (is_server && where & SSL_CB_HANDSHAKE_START) {
ssl_renegotiation_cb(ssl);
}
}
static VALUE
ossl_sslctx_get_options(VALUE self)
{
SSL_CTX *ctx;
GetSSLCTX(self, ctx);
return ULONG2NUM((unsigned long)SSL_CTX_get_options(ctx));
}
static VALUE
ossl_sslctx_set_options(VALUE self, VALUE options)
{
SSL_CTX *ctx;
rb_check_frozen(self);
GetSSLCTX(self, ctx);
SSL_CTX_clear_options(ctx, SSL_CTX_get_options(ctx));
if (NIL_P(options)) {
SSL_CTX_set_options(ctx, SSL_OP_ALL);
} else {
SSL_CTX_set_options(ctx, NUM2ULONG(options));
}
return self;
}
static VALUE
ossl_sslctx_setup(VALUE self)
{
SSL_CTX *ctx;
X509 *cert = NULL, *client_ca = NULL;
EVP_PKEY *key = NULL;
char *ca_path = NULL, *ca_file = NULL;
int verify_mode;
long i;
VALUE val;
if(OBJ_FROZEN(self)) return Qnil;
GetSSLCTX(self, ctx);
#if !defined(OPENSSL_NO_DH)
SSL_CTX_set_tmp_dh_callback(ctx, ossl_tmp_dh_callback);
#endif
#if !defined(OPENSSL_NO_EC)
if (RTEST(rb_attr_get(self, id_i_tmp_ecdh_callback))) {
#if defined(HAVE_SSL_CTX_SET_TMP_ECDH_CALLBACK)
rb_warn("#tmp_ecdh_callback= is deprecated; use #ecdh_curves= instead");
SSL_CTX_set_tmp_ecdh_callback(ctx, ossl_tmp_ecdh_callback);
#if defined(HAVE_SSL_CTX_SET_ECDH_AUTO)
if (!SSL_CTX_set_ecdh_auto(ctx, 0))
ossl_raise(eSSLError, "SSL_CTX_set_ecdh_auto");
#endif
#else
ossl_raise(eSSLError, "OpenSSL does not support tmp_ecdh_callback; "
"use #ecdh_curves= instead");
#endif
}
#endif 
#if defined(HAVE_SSL_CTX_SET_POST_HANDSHAKE_AUTH)
SSL_CTX_set_post_handshake_auth(ctx, 1);
#endif
val = rb_attr_get(self, id_i_cert_store);
if (!NIL_P(val)) {
X509_STORE *store = GetX509StorePtr(val); 
SSL_CTX_set_cert_store(ctx, store);
#if !defined(HAVE_X509_STORE_UP_REF)
SSL_CTX_set_ex_data(ctx, ossl_sslctx_ex_store_p, ctx);
#else 
X509_STORE_up_ref(store);
#endif
}
val = rb_attr_get(self, id_i_extra_chain_cert);
if(!NIL_P(val)){
rb_block_call(val, rb_intern("each"), 0, 0, ossl_sslctx_add_extra_chain_cert_i, self);
}
val = rb_attr_get(self, id_i_cert);
cert = NIL_P(val) ? NULL : GetX509CertPtr(val); 
val = rb_attr_get(self, id_i_key);
key = NIL_P(val) ? NULL : GetPrivPKeyPtr(val); 
if (cert && key) {
if (!SSL_CTX_use_certificate(ctx, cert)) {
ossl_raise(eSSLError, "SSL_CTX_use_certificate");
}
if (!SSL_CTX_use_PrivateKey(ctx, key)) {
ossl_raise(eSSLError, "SSL_CTX_use_PrivateKey");
}
if (!SSL_CTX_check_private_key(ctx)) {
ossl_raise(eSSLError, "SSL_CTX_check_private_key");
}
}
val = rb_attr_get(self, id_i_client_ca);
if(!NIL_P(val)){
if (RB_TYPE_P(val, T_ARRAY)) {
for(i = 0; i < RARRAY_LEN(val); i++){
client_ca = GetX509CertPtr(RARRAY_AREF(val, i));
if (!SSL_CTX_add_client_CA(ctx, client_ca)){
ossl_raise(eSSLError, "SSL_CTX_add_client_CA");
}
}
}
else{
client_ca = GetX509CertPtr(val); 
if (!SSL_CTX_add_client_CA(ctx, client_ca)){
ossl_raise(eSSLError, "SSL_CTX_add_client_CA");
}
}
}
val = rb_attr_get(self, id_i_ca_file);
ca_file = NIL_P(val) ? NULL : StringValueCStr(val);
val = rb_attr_get(self, id_i_ca_path);
ca_path = NIL_P(val) ? NULL : StringValueCStr(val);
if(ca_file || ca_path){
if (!SSL_CTX_load_verify_locations(ctx, ca_file, ca_path))
rb_warning("can't set verify locations");
}
val = rb_attr_get(self, id_i_verify_mode);
verify_mode = NIL_P(val) ? SSL_VERIFY_NONE : NUM2INT(val);
SSL_CTX_set_verify(ctx, verify_mode, ossl_ssl_verify_callback);
if (RTEST(rb_attr_get(self, id_i_client_cert_cb)))
SSL_CTX_set_client_cert_cb(ctx, ossl_client_cert_cb);
val = rb_attr_get(self, id_i_timeout);
if(!NIL_P(val)) SSL_CTX_set_timeout(ctx, NUM2LONG(val));
val = rb_attr_get(self, id_i_verify_depth);
if(!NIL_P(val)) SSL_CTX_set_verify_depth(ctx, NUM2INT(val));
#if !defined(OPENSSL_NO_NEXTPROTONEG)
val = rb_attr_get(self, id_i_npn_protocols);
if (!NIL_P(val)) {
VALUE encoded = ssl_encode_npn_protocols(val);
rb_ivar_set(self, id_npn_protocols_encoded, encoded);
SSL_CTX_set_next_protos_advertised_cb(ctx, ssl_npn_advertise_cb, (void *)encoded);
OSSL_Debug("SSL NPN advertise callback added");
}
if (RTEST(rb_attr_get(self, id_i_npn_select_cb))) {
SSL_CTX_set_next_proto_select_cb(ctx, ssl_npn_select_cb, (void *) self);
OSSL_Debug("SSL NPN select callback added");
}
#endif
#if defined(HAVE_SSL_CTX_SET_ALPN_SELECT_CB)
val = rb_attr_get(self, id_i_alpn_protocols);
if (!NIL_P(val)) {
VALUE rprotos = ssl_encode_npn_protocols(val);
if (SSL_CTX_set_alpn_protos(ctx, (unsigned char *)RSTRING_PTR(rprotos),
RSTRING_LENINT(rprotos)))
ossl_raise(eSSLError, "SSL_CTX_set_alpn_protos");
OSSL_Debug("SSL ALPN values added");
}
if (RTEST(rb_attr_get(self, id_i_alpn_select_cb))) {
SSL_CTX_set_alpn_select_cb(ctx, ssl_alpn_select_cb, (void *) self);
OSSL_Debug("SSL ALPN select callback added");
}
#endif
rb_obj_freeze(self);
val = rb_attr_get(self, id_i_session_id_context);
if (!NIL_P(val)){
StringValue(val);
if (!SSL_CTX_set_session_id_context(ctx, (unsigned char *)RSTRING_PTR(val),
RSTRING_LENINT(val))){
ossl_raise(eSSLError, "SSL_CTX_set_session_id_context");
}
}
if (RTEST(rb_attr_get(self, id_i_session_get_cb))) {
SSL_CTX_sess_set_get_cb(ctx, ossl_sslctx_session_get_cb);
OSSL_Debug("SSL SESSION get callback added");
}
if (RTEST(rb_attr_get(self, id_i_session_new_cb))) {
SSL_CTX_sess_set_new_cb(ctx, ossl_sslctx_session_new_cb);
OSSL_Debug("SSL SESSION new callback added");
}
if (RTEST(rb_attr_get(self, id_i_session_remove_cb))) {
SSL_CTX_sess_set_remove_cb(ctx, ossl_sslctx_session_remove_cb);
OSSL_Debug("SSL SESSION remove callback added");
}
val = rb_attr_get(self, id_i_servername_cb);
if (!NIL_P(val)) {
SSL_CTX_set_tlsext_servername_callback(ctx, ssl_servername_cb);
OSSL_Debug("SSL TLSEXT servername callback added");
}
return Qtrue;
}
static VALUE
ossl_ssl_cipher_to_ary(const SSL_CIPHER *cipher)
{
VALUE ary;
int bits, alg_bits;
ary = rb_ary_new2(4);
rb_ary_push(ary, rb_str_new2(SSL_CIPHER_get_name(cipher)));
rb_ary_push(ary, rb_str_new2(SSL_CIPHER_get_version(cipher)));
bits = SSL_CIPHER_get_bits(cipher, &alg_bits);
rb_ary_push(ary, INT2NUM(bits));
rb_ary_push(ary, INT2NUM(alg_bits));
return ary;
}
static VALUE
ossl_sslctx_get_ciphers(VALUE self)
{
SSL_CTX *ctx;
STACK_OF(SSL_CIPHER) *ciphers;
const SSL_CIPHER *cipher;
VALUE ary;
int i, num;
GetSSLCTX(self, ctx);
ciphers = SSL_CTX_get_ciphers(ctx);
if (!ciphers)
return rb_ary_new();
num = sk_SSL_CIPHER_num(ciphers);
ary = rb_ary_new2(num);
for(i = 0; i < num; i++){
cipher = sk_SSL_CIPHER_value(ciphers, i);
rb_ary_push(ary, ossl_ssl_cipher_to_ary(cipher));
}
return ary;
}
static VALUE
ossl_sslctx_set_ciphers(VALUE self, VALUE v)
{
SSL_CTX *ctx;
VALUE str, elem;
int i;
rb_check_frozen(self);
if (NIL_P(v))
return v;
else if (RB_TYPE_P(v, T_ARRAY)) {
str = rb_str_new(0, 0);
for (i = 0; i < RARRAY_LEN(v); i++) {
elem = rb_ary_entry(v, i);
if (RB_TYPE_P(elem, T_ARRAY)) elem = rb_ary_entry(elem, 0);
elem = rb_String(elem);
rb_str_append(str, elem);
if (i < RARRAY_LEN(v)-1) rb_str_cat2(str, ":");
}
} else {
str = v;
StringValue(str);
}
GetSSLCTX(self, ctx);
if (!SSL_CTX_set_cipher_list(ctx, StringValueCStr(str))) {
ossl_raise(eSSLError, "SSL_CTX_set_cipher_list");
}
return v;
}
#if !defined(OPENSSL_NO_EC)
static VALUE
ossl_sslctx_set_ecdh_curves(VALUE self, VALUE arg)
{
SSL_CTX *ctx;
rb_check_frozen(self);
GetSSLCTX(self, ctx);
StringValueCStr(arg);
#if defined(HAVE_SSL_CTX_SET1_CURVES_LIST)
if (!SSL_CTX_set1_curves_list(ctx, RSTRING_PTR(arg)))
ossl_raise(eSSLError, NULL);
#else
{
VALUE curve, splitted;
EC_KEY *ec;
int nid;
splitted = rb_str_split(arg, ":");
if (!RARRAY_LEN(splitted))
ossl_raise(eSSLError, "invalid input format");
curve = RARRAY_AREF(splitted, 0);
StringValueCStr(curve);
nid = EC_curve_nist2nid(RSTRING_PTR(curve));
if (nid == NID_undef)
nid = OBJ_txt2nid(RSTRING_PTR(curve));
if (nid == NID_undef)
ossl_raise(eSSLError, "unknown curve name");
ec = EC_KEY_new_by_curve_name(nid);
if (!ec)
ossl_raise(eSSLError, NULL);
EC_KEY_set_asn1_flag(ec, OPENSSL_EC_NAMED_CURVE);
if (!SSL_CTX_set_tmp_ecdh(ctx, ec)) {
EC_KEY_free(ec);
ossl_raise(eSSLError, "SSL_CTX_set_tmp_ecdh");
}
EC_KEY_free(ec);
#if defined(HAVE_SSL_CTX_SET_ECDH_AUTO)
if (!SSL_CTX_set_ecdh_auto(ctx, 0))
ossl_raise(eSSLError, "SSL_CTX_set_ecdh_auto");
#endif
}
#endif
return arg;
}
#else
#define ossl_sslctx_set_ecdh_curves rb_f_notimplement
#endif
static VALUE
ossl_sslctx_get_security_level(VALUE self)
{
SSL_CTX *ctx;
GetSSLCTX(self, ctx);
#if defined(HAVE_SSL_CTX_GET_SECURITY_LEVEL)
return INT2NUM(SSL_CTX_get_security_level(ctx));
#else
(void)ctx;
return INT2FIX(0);
#endif
}
static VALUE
ossl_sslctx_set_security_level(VALUE self, VALUE value)
{
SSL_CTX *ctx;
rb_check_frozen(self);
GetSSLCTX(self, ctx);
#if defined(HAVE_SSL_CTX_GET_SECURITY_LEVEL)
SSL_CTX_set_security_level(ctx, NUM2INT(value));
#else
(void)ctx;
if (NUM2INT(value) != 0)
ossl_raise(rb_eNotImpError, "setting security level to other than 0 is "
"not supported in this version of OpenSSL");
#endif
return value;
}
#if defined(SSL_MODE_SEND_FALLBACK_SCSV)
static VALUE
ossl_sslctx_enable_fallback_scsv(VALUE self)
{
SSL_CTX *ctx;
GetSSLCTX(self, ctx);
SSL_CTX_set_mode(ctx, SSL_MODE_SEND_FALLBACK_SCSV);
return Qnil;
}
#endif
static VALUE
ossl_sslctx_add_certificate(int argc, VALUE *argv, VALUE self)
{
VALUE cert, key, extra_chain_ary;
SSL_CTX *ctx;
X509 *x509;
STACK_OF(X509) *extra_chain = NULL;
EVP_PKEY *pkey, *pub_pkey;
GetSSLCTX(self, ctx);
rb_scan_args(argc, argv, "21", &cert, &key, &extra_chain_ary);
rb_check_frozen(self);
x509 = GetX509CertPtr(cert);
pkey = GetPrivPKeyPtr(key);
pub_pkey = X509_get_pubkey(x509);
EVP_PKEY_free(pub_pkey);
if (!pub_pkey)
rb_raise(rb_eArgError, "certificate does not contain public key");
if (EVP_PKEY_cmp(pub_pkey, pkey) != 1)
rb_raise(rb_eArgError, "public key mismatch");
if (argc >= 3)
extra_chain = ossl_x509_ary2sk(extra_chain_ary);
if (!SSL_CTX_use_certificate(ctx, x509)) {
sk_X509_pop_free(extra_chain, X509_free);
ossl_raise(eSSLError, "SSL_CTX_use_certificate");
}
if (!SSL_CTX_use_PrivateKey(ctx, pkey)) {
sk_X509_pop_free(extra_chain, X509_free);
ossl_raise(eSSLError, "SSL_CTX_use_PrivateKey");
}
if (extra_chain) {
#if OPENSSL_VERSION_NUMBER >= 0x10002000 && !defined(LIBRESSL_VERSION_NUMBER)
if (!SSL_CTX_set0_chain(ctx, extra_chain)) {
sk_X509_pop_free(extra_chain, X509_free);
ossl_raise(eSSLError, "SSL_CTX_set0_chain");
}
#else
STACK_OF(X509) *orig_extra_chain;
X509 *x509_tmp;
SSL_CTX_get_extra_chain_certs(ctx, &orig_extra_chain);
if (orig_extra_chain && sk_X509_num(orig_extra_chain)) {
rb_warning("SSL_CTX_set0_chain() is not available; " \
"clearing previously set certificate chain");
SSL_CTX_clear_extra_chain_certs(ctx);
}
while ((x509_tmp = sk_X509_shift(extra_chain))) {
if (!SSL_CTX_add_extra_chain_cert(ctx, x509_tmp)) {
X509_free(x509_tmp);
sk_X509_pop_free(extra_chain, X509_free);
ossl_raise(eSSLError, "SSL_CTX_add_extra_chain_cert");
}
}
sk_X509_free(extra_chain);
#endif
}
return self;
}
static VALUE
ossl_sslctx_add_certificate_chain_file(VALUE self, VALUE path)
{
SSL_CTX *ctx;
int ret;
GetSSLCTX(self, ctx);
StringValueCStr(path);
ret = SSL_CTX_use_certificate_chain_file(ctx, RSTRING_PTR(path));
if (ret != 1)
ossl_raise(eSSLError, "SSL_CTX_use_certificate_chain_file");
return Qtrue;
}
static VALUE
ossl_sslctx_session_add(VALUE self, VALUE arg)
{
SSL_CTX *ctx;
SSL_SESSION *sess;
GetSSLCTX(self, ctx);
GetSSLSession(arg, sess);
return SSL_CTX_add_session(ctx, sess) == 1 ? Qtrue : Qfalse;
}
static VALUE
ossl_sslctx_session_remove(VALUE self, VALUE arg)
{
SSL_CTX *ctx;
SSL_SESSION *sess;
GetSSLCTX(self, ctx);
GetSSLSession(arg, sess);
return SSL_CTX_remove_session(ctx, sess) == 1 ? Qtrue : Qfalse;
}
static VALUE
ossl_sslctx_get_session_cache_mode(VALUE self)
{
SSL_CTX *ctx;
GetSSLCTX(self, ctx);
return LONG2NUM(SSL_CTX_get_session_cache_mode(ctx));
}
static VALUE
ossl_sslctx_set_session_cache_mode(VALUE self, VALUE arg)
{
SSL_CTX *ctx;
GetSSLCTX(self, ctx);
SSL_CTX_set_session_cache_mode(ctx, NUM2LONG(arg));
return arg;
}
static VALUE
ossl_sslctx_get_session_cache_size(VALUE self)
{
SSL_CTX *ctx;
GetSSLCTX(self, ctx);
return LONG2NUM(SSL_CTX_sess_get_cache_size(ctx));
}
static VALUE
ossl_sslctx_set_session_cache_size(VALUE self, VALUE arg)
{
SSL_CTX *ctx;
GetSSLCTX(self, ctx);
SSL_CTX_sess_set_cache_size(ctx, NUM2LONG(arg));
return arg;
}
static VALUE
ossl_sslctx_get_session_cache_stats(VALUE self)
{
SSL_CTX *ctx;
VALUE hash;
GetSSLCTX(self, ctx);
hash = rb_hash_new();
rb_hash_aset(hash, ID2SYM(rb_intern("cache_num")), LONG2NUM(SSL_CTX_sess_number(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("connect")), LONG2NUM(SSL_CTX_sess_connect(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("connect_good")), LONG2NUM(SSL_CTX_sess_connect_good(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("connect_renegotiate")), LONG2NUM(SSL_CTX_sess_connect_renegotiate(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("accept")), LONG2NUM(SSL_CTX_sess_accept(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("accept_good")), LONG2NUM(SSL_CTX_sess_accept_good(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("accept_renegotiate")), LONG2NUM(SSL_CTX_sess_accept_renegotiate(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("cache_hits")), LONG2NUM(SSL_CTX_sess_hits(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("cb_hits")), LONG2NUM(SSL_CTX_sess_cb_hits(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("cache_misses")), LONG2NUM(SSL_CTX_sess_misses(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("cache_full")), LONG2NUM(SSL_CTX_sess_cache_full(ctx)));
rb_hash_aset(hash, ID2SYM(rb_intern("timeouts")), LONG2NUM(SSL_CTX_sess_timeouts(ctx)));
return hash;
}
static VALUE
ossl_sslctx_flush_sessions(int argc, VALUE *argv, VALUE self)
{
VALUE arg1;
SSL_CTX *ctx;
time_t tm = 0;
rb_scan_args(argc, argv, "01", &arg1);
GetSSLCTX(self, ctx);
if (NIL_P(arg1)) {
tm = time(0);
} else if (rb_obj_is_instance_of(arg1, rb_cTime)) {
tm = NUM2LONG(rb_funcall(arg1, rb_intern("to_i"), 0));
} else {
ossl_raise(rb_eArgError, "arg must be Time or nil");
}
SSL_CTX_flush_sessions(ctx, (long)tm);
return self;
}
#if !defined(OPENSSL_NO_SOCK)
static inline int
ssl_started(SSL *ssl)
{
return SSL_get_fd(ssl) >= 0;
}
static void
ossl_ssl_free(void *ssl)
{
SSL_free(ssl);
}
const rb_data_type_t ossl_ssl_type = {
"OpenSSL/SSL",
{
0, ossl_ssl_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};
static VALUE
ossl_ssl_s_alloc(VALUE klass)
{
return TypedData_Wrap_Struct(klass, &ossl_ssl_type, NULL);
}
static VALUE
ossl_ssl_initialize(int argc, VALUE *argv, VALUE self)
{
VALUE io, v_ctx, verify_cb;
SSL *ssl;
SSL_CTX *ctx;
TypedData_Get_Struct(self, SSL, &ossl_ssl_type, ssl);
if (ssl)
ossl_raise(eSSLError, "SSL already initialized");
if (rb_scan_args(argc, argv, "11", &io, &v_ctx) == 1)
v_ctx = rb_funcall(cSSLContext, rb_intern("new"), 0);
GetSSLCTX(v_ctx, ctx);
rb_ivar_set(self, id_i_context, v_ctx);
ossl_sslctx_setup(v_ctx);
if (rb_respond_to(io, rb_intern("nonblock=")))
rb_funcall(io, rb_intern("nonblock="), 1, Qtrue);
rb_ivar_set(self, id_i_io, io);
ssl = SSL_new(ctx);
if (!ssl)
ossl_raise(eSSLError, NULL);
RTYPEDDATA_DATA(self) = ssl;
SSL_set_ex_data(ssl, ossl_ssl_ex_ptr_idx, (void *)self);
SSL_set_info_callback(ssl, ssl_info_cb);
verify_cb = rb_attr_get(v_ctx, id_i_verify_callback);
SSL_set_ex_data(ssl, ossl_ssl_ex_vcb_idx, (void *)verify_cb);
rb_call_super(0, NULL);
return self;
}
static VALUE
ossl_ssl_setup(VALUE self)
{
VALUE io;
SSL *ssl;
rb_io_t *fptr;
GetSSL(self, ssl);
if (ssl_started(ssl))
return Qtrue;
io = rb_attr_get(self, id_i_io);
GetOpenFile(io, fptr);
rb_io_check_readable(fptr);
rb_io_check_writable(fptr);
if (!SSL_set_fd(ssl, TO_SOCKET(fptr->fd)))
ossl_raise(eSSLError, "SSL_set_fd");
return Qtrue;
}
#if defined(_WIN32)
#define ssl_get_error(ssl, ret) (errno = rb_w32_map_errno(WSAGetLastError()), SSL_get_error((ssl), (ret)))
#else
#define ssl_get_error(ssl, ret) SSL_get_error((ssl), (ret))
#endif
static void
write_would_block(int nonblock)
{
if (nonblock)
ossl_raise(eSSLErrorWaitWritable, "write would block");
}
static void
read_would_block(int nonblock)
{
if (nonblock)
ossl_raise(eSSLErrorWaitReadable, "read would block");
}
static int
no_exception_p(VALUE opts)
{
if (RB_TYPE_P(opts, T_HASH) &&
rb_hash_lookup2(opts, sym_exception, Qundef) == Qfalse)
return 1;
return 0;
}
static VALUE
ossl_start_ssl(VALUE self, int (*func)(), const char *funcname, VALUE opts)
{
SSL *ssl;
rb_io_t *fptr;
int ret, ret2;
VALUE cb_state;
int nonblock = opts != Qfalse;
#if defined(SSL_R_CERTIFICATE_VERIFY_FAILED)
unsigned long err;
#endif
rb_ivar_set(self, ID_callback_state, Qnil);
GetSSL(self, ssl);
GetOpenFile(rb_attr_get(self, id_i_io), fptr);
for(;;){
ret = func(ssl);
cb_state = rb_attr_get(self, ID_callback_state);
if (!NIL_P(cb_state)) {
ossl_clear_error();
rb_jump_tag(NUM2INT(cb_state));
}
if (ret > 0)
break;
switch((ret2 = ssl_get_error(ssl, ret))){
case SSL_ERROR_WANT_WRITE:
if (no_exception_p(opts)) { return sym_wait_writable; }
write_would_block(nonblock);
rb_io_wait_writable(fptr->fd);
continue;
case SSL_ERROR_WANT_READ:
if (no_exception_p(opts)) { return sym_wait_readable; }
read_would_block(nonblock);
rb_io_wait_readable(fptr->fd);
continue;
case SSL_ERROR_SYSCALL:
if (errno) rb_sys_fail(funcname);
ossl_raise(eSSLError, "%s SYSCALL returned=%d errno=%d state=%s", funcname, ret2, errno, SSL_state_string_long(ssl));
#if defined(SSL_R_CERTIFICATE_VERIFY_FAILED)
case SSL_ERROR_SSL:
err = ERR_peek_last_error();
if (ERR_GET_LIB(err) == ERR_LIB_SSL &&
ERR_GET_REASON(err) == SSL_R_CERTIFICATE_VERIFY_FAILED) {
const char *err_msg = ERR_reason_error_string(err),
*verify_msg = X509_verify_cert_error_string(SSL_get_verify_result(ssl));
if (!err_msg)
err_msg = "(null)";
if (!verify_msg)
verify_msg = "(null)";
ossl_clear_error(); 
ossl_raise(eSSLError, "%s returned=%d errno=%d state=%s: %s (%s)",
funcname, ret2, errno, SSL_state_string_long(ssl),
err_msg, verify_msg);
}
#endif
default:
ossl_raise(eSSLError, "%s returned=%d errno=%d state=%s", funcname, ret2, errno, SSL_state_string_long(ssl));
}
}
return self;
}
static VALUE
ossl_ssl_connect(VALUE self)
{
ossl_ssl_setup(self);
return ossl_start_ssl(self, SSL_connect, "SSL_connect", Qfalse);
}
static VALUE
ossl_ssl_connect_nonblock(int argc, VALUE *argv, VALUE self)
{
VALUE opts;
rb_scan_args(argc, argv, "0:", &opts);
ossl_ssl_setup(self);
return ossl_start_ssl(self, SSL_connect, "SSL_connect", opts);
}
static VALUE
ossl_ssl_accept(VALUE self)
{
ossl_ssl_setup(self);
return ossl_start_ssl(self, SSL_accept, "SSL_accept", Qfalse);
}
static VALUE
ossl_ssl_accept_nonblock(int argc, VALUE *argv, VALUE self)
{
VALUE opts;
rb_scan_args(argc, argv, "0:", &opts);
ossl_ssl_setup(self);
return ossl_start_ssl(self, SSL_accept, "SSL_accept", opts);
}
static VALUE
ossl_ssl_read_internal(int argc, VALUE *argv, VALUE self, int nonblock)
{
SSL *ssl;
int ilen, nread = 0;
VALUE len, str;
rb_io_t *fptr;
VALUE io, opts = Qnil;
if (nonblock) {
rb_scan_args(argc, argv, "11:", &len, &str, &opts);
} else {
rb_scan_args(argc, argv, "11", &len, &str);
}
ilen = NUM2INT(len);
if (NIL_P(str))
str = rb_str_new(0, ilen);
else {
StringValue(str);
if (RSTRING_LEN(str) >= ilen)
rb_str_modify(str);
else
rb_str_modify_expand(str, ilen - RSTRING_LEN(str));
}
rb_str_set_len(str, 0);
if (ilen == 0)
return str;
GetSSL(self, ssl);
io = rb_attr_get(self, id_i_io);
GetOpenFile(io, fptr);
if (ssl_started(ssl)) {
for (;;){
nread = SSL_read(ssl, RSTRING_PTR(str), ilen);
switch(ssl_get_error(ssl, nread)){
case SSL_ERROR_NONE:
goto end;
case SSL_ERROR_ZERO_RETURN:
if (no_exception_p(opts)) { return Qnil; }
rb_eof_error();
case SSL_ERROR_WANT_WRITE:
if (no_exception_p(opts)) { return sym_wait_writable; }
write_would_block(nonblock);
rb_io_wait_writable(fptr->fd);
continue;
case SSL_ERROR_WANT_READ:
if (no_exception_p(opts)) { return sym_wait_readable; }
read_would_block(nonblock);
rb_io_wait_readable(fptr->fd);
continue;
case SSL_ERROR_SYSCALL:
if (!ERR_peek_error()) {
if (errno)
rb_sys_fail(0);
else {
if (no_exception_p(opts)) { return Qnil; }
rb_eof_error();
}
}
default:
ossl_raise(eSSLError, "SSL_read");
}
}
}
else {
ID meth = nonblock ? rb_intern("read_nonblock") : rb_intern("sysread");
rb_warning("SSL session is not started yet.");
#if defined(RB_PASS_KEYWORDS)
if (nonblock) {
VALUE argv[3];
argv[0] = len;
argv[1] = str;
argv[2] = opts;
return rb_funcallv_kw(io, meth, 3, argv, RB_PASS_KEYWORDS);
}
#else
if (nonblock) {
return rb_funcall(io, meth, 3, len, str, opts);
}
#endif
else
return rb_funcall(io, meth, 2, len, str);
}
end:
rb_str_set_len(str, nread);
return str;
}
static VALUE
ossl_ssl_read(int argc, VALUE *argv, VALUE self)
{
return ossl_ssl_read_internal(argc, argv, self, 0);
}
static VALUE
ossl_ssl_read_nonblock(int argc, VALUE *argv, VALUE self)
{
return ossl_ssl_read_internal(argc, argv, self, 1);
}
static VALUE
ossl_ssl_write_internal(VALUE self, VALUE str, VALUE opts)
{
SSL *ssl;
int nwrite = 0;
rb_io_t *fptr;
int nonblock = opts != Qfalse;
VALUE io;
StringValue(str);
GetSSL(self, ssl);
io = rb_attr_get(self, id_i_io);
GetOpenFile(io, fptr);
if (ssl_started(ssl)) {
for (;;){
int num = RSTRING_LENINT(str);
if (num == 0)
goto end;
nwrite = SSL_write(ssl, RSTRING_PTR(str), num);
switch(ssl_get_error(ssl, nwrite)){
case SSL_ERROR_NONE:
goto end;
case SSL_ERROR_WANT_WRITE:
if (no_exception_p(opts)) { return sym_wait_writable; }
write_would_block(nonblock);
rb_io_wait_writable(fptr->fd);
continue;
case SSL_ERROR_WANT_READ:
if (no_exception_p(opts)) { return sym_wait_readable; }
read_would_block(nonblock);
rb_io_wait_readable(fptr->fd);
continue;
case SSL_ERROR_SYSCALL:
if (errno) rb_sys_fail(0);
default:
ossl_raise(eSSLError, "SSL_write");
}
}
}
else {
ID meth = nonblock ?
rb_intern("write_nonblock") : rb_intern("syswrite");
rb_warning("SSL session is not started yet.");
#if defined(RB_PASS_KEYWORDS)
if (nonblock) {
VALUE argv[2];
argv[0] = str;
argv[1] = opts;
return rb_funcallv_kw(io, meth, 2, argv, RB_PASS_KEYWORDS);
}
#else
if (nonblock) {
return rb_funcall(io, meth, 2, str, opts);
}
#endif
else
return rb_funcall(io, meth, 1, str);
}
end:
return INT2NUM(nwrite);
}
static VALUE
ossl_ssl_write(VALUE self, VALUE str)
{
return ossl_ssl_write_internal(self, str, Qfalse);
}
static VALUE
ossl_ssl_write_nonblock(int argc, VALUE *argv, VALUE self)
{
VALUE str, opts;
rb_scan_args(argc, argv, "1:", &str, &opts);
return ossl_ssl_write_internal(self, str, opts);
}
static VALUE
ossl_ssl_stop(VALUE self)
{
SSL *ssl;
int ret;
GetSSL(self, ssl);
if (!ssl_started(ssl))
return Qnil;
ret = SSL_shutdown(ssl);
if (ret == 1) 
return Qnil;
if (ret == 0) 
return Qnil;
ossl_clear_error();
return Qnil;
}
static VALUE
ossl_ssl_get_cert(VALUE self)
{
SSL *ssl;
X509 *cert = NULL;
GetSSL(self, ssl);
cert = SSL_get_certificate(ssl); 
if (!cert) {
return Qnil;
}
return ossl_x509_new(cert);
}
static VALUE
ossl_ssl_get_peer_cert(VALUE self)
{
SSL *ssl;
X509 *cert = NULL;
VALUE obj;
GetSSL(self, ssl);
cert = SSL_get_peer_certificate(ssl); 
if (!cert) {
return Qnil;
}
obj = ossl_x509_new(cert);
X509_free(cert);
return obj;
}
static VALUE
ossl_ssl_get_peer_cert_chain(VALUE self)
{
SSL *ssl;
STACK_OF(X509) *chain;
X509 *cert;
VALUE ary;
int i, num;
GetSSL(self, ssl);
chain = SSL_get_peer_cert_chain(ssl);
if(!chain) return Qnil;
num = sk_X509_num(chain);
ary = rb_ary_new2(num);
for (i = 0; i < num; i++){
cert = sk_X509_value(chain, i);
rb_ary_push(ary, ossl_x509_new(cert));
}
return ary;
}
static VALUE
ossl_ssl_get_version(VALUE self)
{
SSL *ssl;
GetSSL(self, ssl);
return rb_str_new2(SSL_get_version(ssl));
}
static VALUE
ossl_ssl_get_cipher(VALUE self)
{
SSL *ssl;
const SSL_CIPHER *cipher;
GetSSL(self, ssl);
cipher = SSL_get_current_cipher(ssl);
return cipher ? ossl_ssl_cipher_to_ary(cipher) : Qnil;
}
static VALUE
ossl_ssl_get_state(VALUE self)
{
SSL *ssl;
VALUE ret;
GetSSL(self, ssl);
ret = rb_str_new2(SSL_state_string(ssl));
if (ruby_verbose) {
rb_str_cat2(ret, ": ");
rb_str_cat2(ret, SSL_state_string_long(ssl));
}
return ret;
}
static VALUE
ossl_ssl_pending(VALUE self)
{
SSL *ssl;
GetSSL(self, ssl);
return INT2NUM(SSL_pending(ssl));
}
static VALUE
ossl_ssl_session_reused(VALUE self)
{
SSL *ssl;
GetSSL(self, ssl);
return SSL_session_reused(ssl) ? Qtrue : Qfalse;
}
static VALUE
ossl_ssl_set_session(VALUE self, VALUE arg1)
{
SSL *ssl;
SSL_SESSION *sess;
GetSSL(self, ssl);
GetSSLSession(arg1, sess);
if (SSL_set_session(ssl, sess) != 1)
ossl_raise(eSSLError, "SSL_set_session");
return arg1;
}
static VALUE
ossl_ssl_set_hostname(VALUE self, VALUE arg)
{
SSL *ssl;
char *hostname = NULL;
GetSSL(self, ssl);
if (!NIL_P(arg))
hostname = StringValueCStr(arg);
if (!SSL_set_tlsext_host_name(ssl, hostname))
ossl_raise(eSSLError, NULL);
rb_ivar_set(self, id_i_hostname, arg);
return arg;
}
static VALUE
ossl_ssl_get_verify_result(VALUE self)
{
SSL *ssl;
GetSSL(self, ssl);
return INT2NUM(SSL_get_verify_result(ssl));
}
static VALUE
ossl_ssl_get_finished(VALUE self)
{
SSL *ssl;
char sizer[1], *buf;
size_t len;
GetSSL(self, ssl);
len = SSL_get_finished(ssl, sizer, 0);
if (len == 0)
return Qnil;
buf = ALLOCA_N(char, len);
SSL_get_finished(ssl, buf, len);
return rb_str_new(buf, len);
}
static VALUE
ossl_ssl_get_peer_finished(VALUE self)
{
SSL *ssl;
char sizer[1], *buf;
size_t len;
GetSSL(self, ssl);
len = SSL_get_peer_finished(ssl, sizer, 0);
if (len == 0)
return Qnil;
buf = ALLOCA_N(char, len);
SSL_get_peer_finished(ssl, buf, len);
return rb_str_new(buf, len);
}
static VALUE
ossl_ssl_get_client_ca_list(VALUE self)
{
SSL *ssl;
STACK_OF(X509_NAME) *ca;
GetSSL(self, ssl);
ca = SSL_get_client_CA_list(ssl);
return ossl_x509name_sk2ary(ca);
}
#if !defined(OPENSSL_NO_NEXTPROTONEG)
static VALUE
ossl_ssl_npn_protocol(VALUE self)
{
SSL *ssl;
const unsigned char *out;
unsigned int outlen;
GetSSL(self, ssl);
SSL_get0_next_proto_negotiated(ssl, &out, &outlen);
if (!outlen)
return Qnil;
else
return rb_str_new((const char *) out, outlen);
}
#endif
#if defined(HAVE_SSL_CTX_SET_ALPN_SELECT_CB)
static VALUE
ossl_ssl_alpn_protocol(VALUE self)
{
SSL *ssl;
const unsigned char *out;
unsigned int outlen;
GetSSL(self, ssl);
SSL_get0_alpn_selected(ssl, &out, &outlen);
if (!outlen)
return Qnil;
else
return rb_str_new((const char *) out, outlen);
}
#endif
#if defined(HAVE_SSL_GET_SERVER_TMP_KEY)
static VALUE
ossl_ssl_tmp_key(VALUE self)
{
SSL *ssl;
EVP_PKEY *key;
GetSSL(self, ssl);
if (!SSL_get_server_tmp_key(ssl, &key))
return Qnil;
return ossl_pkey_new(key);
}
#endif 
#endif 
#undef rb_intern
#define rb_intern(s) rb_intern_const(s)
void
Init_ossl_ssl(void)
{
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
rb_mWaitReadable = rb_define_module_under(rb_cIO, "WaitReadable");
rb_mWaitWritable = rb_define_module_under(rb_cIO, "WaitWritable");
#endif
id_call = rb_intern("call");
ID_callback_state = rb_intern("callback_state");
ossl_ssl_ex_vcb_idx = SSL_get_ex_new_index(0, (void *)"ossl_ssl_ex_vcb_idx", 0, 0, 0);
if (ossl_ssl_ex_vcb_idx < 0)
ossl_raise(rb_eRuntimeError, "SSL_get_ex_new_index");
ossl_ssl_ex_ptr_idx = SSL_get_ex_new_index(0, (void *)"ossl_ssl_ex_ptr_idx", 0, 0, 0);
if (ossl_ssl_ex_ptr_idx < 0)
ossl_raise(rb_eRuntimeError, "SSL_get_ex_new_index");
ossl_sslctx_ex_ptr_idx = SSL_CTX_get_ex_new_index(0, (void *)"ossl_sslctx_ex_ptr_idx", 0, 0, 0);
if (ossl_sslctx_ex_ptr_idx < 0)
ossl_raise(rb_eRuntimeError, "SSL_CTX_get_ex_new_index");
#if !defined(HAVE_X509_STORE_UP_REF)
ossl_sslctx_ex_store_p = SSL_CTX_get_ex_new_index(0, (void *)"ossl_sslctx_ex_store_p", 0, 0, 0);
if (ossl_sslctx_ex_store_p < 0)
ossl_raise(rb_eRuntimeError, "SSL_CTX_get_ex_new_index");
#endif
mSSL = rb_define_module_under(mOSSL, "SSL");
mSSLExtConfig = rb_define_module_under(mOSSL, "ExtConfig");
eSSLError = rb_define_class_under(mSSL, "SSLError", eOSSLError);
eSSLErrorWaitReadable = rb_define_class_under(mSSL, "SSLErrorWaitReadable", eSSLError);
rb_include_module(eSSLErrorWaitReadable, rb_mWaitReadable);
eSSLErrorWaitWritable = rb_define_class_under(mSSL, "SSLErrorWaitWritable", eSSLError);
rb_include_module(eSSLErrorWaitWritable, rb_mWaitWritable);
Init_ossl_ssl_session();
cSSLContext = rb_define_class_under(mSSL, "SSLContext", rb_cObject);
rb_define_alloc_func(cSSLContext, ossl_sslctx_s_alloc);
rb_undef_method(cSSLContext, "initialize_copy");
rb_attr(cSSLContext, rb_intern("cert"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("key"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("client_ca"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("ca_file"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("ca_path"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("timeout"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("verify_mode"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("verify_depth"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("verify_callback"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("verify_hostname"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("cert_store"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("extra_chain_cert"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("client_cert_cb"), 1, 1, Qfalse);
#if !defined(OPENSSL_NO_EC) && defined(HAVE_SSL_CTX_SET_TMP_ECDH_CALLBACK)
rb_attr(cSSLContext, rb_intern("tmp_ecdh_callback"), 1, 1, Qfalse);
#endif
rb_attr(cSSLContext, rb_intern("session_id_context"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("session_get_cb"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("session_new_cb"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("session_remove_cb"), 1, 1, Qfalse);
rb_define_const(mSSLExtConfig, "HAVE_TLSEXT_HOST_NAME", Qtrue);
rb_attr(cSSLContext, rb_intern("renegotiation_cb"), 1, 1, Qfalse);
#if !defined(OPENSSL_NO_NEXTPROTONEG)
rb_attr(cSSLContext, rb_intern("npn_protocols"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("npn_select_cb"), 1, 1, Qfalse);
#endif
#if defined(HAVE_SSL_CTX_SET_ALPN_SELECT_CB)
rb_attr(cSSLContext, rb_intern("alpn_protocols"), 1, 1, Qfalse);
rb_attr(cSSLContext, rb_intern("alpn_select_cb"), 1, 1, Qfalse);
#endif
rb_define_alias(cSSLContext, "ssl_timeout", "timeout");
rb_define_alias(cSSLContext, "ssl_timeout=", "timeout=");
rb_define_private_method(cSSLContext, "set_minmax_proto_version",
ossl_sslctx_set_minmax_proto_version, 2);
rb_define_method(cSSLContext, "ciphers", ossl_sslctx_get_ciphers, 0);
rb_define_method(cSSLContext, "ciphers=", ossl_sslctx_set_ciphers, 1);
rb_define_method(cSSLContext, "ecdh_curves=", ossl_sslctx_set_ecdh_curves, 1);
rb_define_method(cSSLContext, "security_level", ossl_sslctx_get_security_level, 0);
rb_define_method(cSSLContext, "security_level=", ossl_sslctx_set_security_level, 1);
#if defined(SSL_MODE_SEND_FALLBACK_SCSV)
rb_define_method(cSSLContext, "enable_fallback_scsv", ossl_sslctx_enable_fallback_scsv, 0);
#endif
rb_define_method(cSSLContext, "add_certificate", ossl_sslctx_add_certificate, -1);
rb_define_method(cSSLContext, "add_certificate_chain_file", ossl_sslctx_add_certificate_chain_file, 1);
rb_define_method(cSSLContext, "setup", ossl_sslctx_setup, 0);
rb_define_alias(cSSLContext, "freeze", "setup");
rb_define_const(cSSLContext, "SESSION_CACHE_OFF", LONG2NUM(SSL_SESS_CACHE_OFF));
rb_define_const(cSSLContext, "SESSION_CACHE_CLIENT", LONG2NUM(SSL_SESS_CACHE_CLIENT)); 
rb_define_const(cSSLContext, "SESSION_CACHE_SERVER", LONG2NUM(SSL_SESS_CACHE_SERVER));
rb_define_const(cSSLContext, "SESSION_CACHE_BOTH", LONG2NUM(SSL_SESS_CACHE_BOTH)); 
rb_define_const(cSSLContext, "SESSION_CACHE_NO_AUTO_CLEAR", LONG2NUM(SSL_SESS_CACHE_NO_AUTO_CLEAR));
rb_define_const(cSSLContext, "SESSION_CACHE_NO_INTERNAL_LOOKUP", LONG2NUM(SSL_SESS_CACHE_NO_INTERNAL_LOOKUP));
rb_define_const(cSSLContext, "SESSION_CACHE_NO_INTERNAL_STORE", LONG2NUM(SSL_SESS_CACHE_NO_INTERNAL_STORE));
rb_define_const(cSSLContext, "SESSION_CACHE_NO_INTERNAL", LONG2NUM(SSL_SESS_CACHE_NO_INTERNAL));
rb_define_method(cSSLContext, "session_add", ossl_sslctx_session_add, 1);
rb_define_method(cSSLContext, "session_remove", ossl_sslctx_session_remove, 1);
rb_define_method(cSSLContext, "session_cache_mode", ossl_sslctx_get_session_cache_mode, 0);
rb_define_method(cSSLContext, "session_cache_mode=", ossl_sslctx_set_session_cache_mode, 1);
rb_define_method(cSSLContext, "session_cache_size", ossl_sslctx_get_session_cache_size, 0);
rb_define_method(cSSLContext, "session_cache_size=", ossl_sslctx_set_session_cache_size, 1);
rb_define_method(cSSLContext, "session_cache_stats", ossl_sslctx_get_session_cache_stats, 0);
rb_define_method(cSSLContext, "flush_sessions", ossl_sslctx_flush_sessions, -1);
rb_define_method(cSSLContext, "options", ossl_sslctx_get_options, 0);
rb_define_method(cSSLContext, "options=", ossl_sslctx_set_options, 1);
cSSLSocket = rb_define_class_under(mSSL, "SSLSocket", rb_cObject);
#if defined(OPENSSL_NO_SOCK)
rb_define_const(mSSLExtConfig, "OPENSSL_NO_SOCK", Qtrue);
rb_define_method(cSSLSocket, "initialize", rb_f_notimplement, -1);
#else
rb_define_const(mSSLExtConfig, "OPENSSL_NO_SOCK", Qfalse);
rb_define_alloc_func(cSSLSocket, ossl_ssl_s_alloc);
rb_define_method(cSSLSocket, "initialize", ossl_ssl_initialize, -1);
rb_undef_method(cSSLSocket, "initialize_copy");
rb_define_method(cSSLSocket, "connect", ossl_ssl_connect, 0);
rb_define_method(cSSLSocket, "connect_nonblock", ossl_ssl_connect_nonblock, -1);
rb_define_method(cSSLSocket, "accept", ossl_ssl_accept, 0);
rb_define_method(cSSLSocket, "accept_nonblock", ossl_ssl_accept_nonblock, -1);
rb_define_method(cSSLSocket, "sysread", ossl_ssl_read, -1);
rb_define_private_method(cSSLSocket, "sysread_nonblock", ossl_ssl_read_nonblock, -1);
rb_define_method(cSSLSocket, "syswrite", ossl_ssl_write, 1);
rb_define_private_method(cSSLSocket, "syswrite_nonblock", ossl_ssl_write_nonblock, -1);
rb_define_private_method(cSSLSocket, "stop", ossl_ssl_stop, 0);
rb_define_method(cSSLSocket, "cert", ossl_ssl_get_cert, 0);
rb_define_method(cSSLSocket, "peer_cert", ossl_ssl_get_peer_cert, 0);
rb_define_method(cSSLSocket, "peer_cert_chain", ossl_ssl_get_peer_cert_chain, 0);
rb_define_method(cSSLSocket, "ssl_version", ossl_ssl_get_version, 0);
rb_define_method(cSSLSocket, "cipher", ossl_ssl_get_cipher, 0);
rb_define_method(cSSLSocket, "state", ossl_ssl_get_state, 0);
rb_define_method(cSSLSocket, "pending", ossl_ssl_pending, 0);
rb_define_method(cSSLSocket, "session_reused?", ossl_ssl_session_reused, 0);
rb_define_method(cSSLSocket, "session=", ossl_ssl_set_session, 1);
rb_define_method(cSSLSocket, "verify_result", ossl_ssl_get_verify_result, 0);
rb_define_method(cSSLSocket, "client_ca", ossl_ssl_get_client_ca_list, 0);
rb_define_method(cSSLSocket, "hostname=", ossl_ssl_set_hostname, 1);
rb_define_method(cSSLSocket, "finished_message", ossl_ssl_get_finished, 0);
rb_define_method(cSSLSocket, "peer_finished_message", ossl_ssl_get_peer_finished, 0);
#if defined(HAVE_SSL_GET_SERVER_TMP_KEY)
rb_define_method(cSSLSocket, "tmp_key", ossl_ssl_tmp_key, 0);
#endif
#if defined(HAVE_SSL_CTX_SET_ALPN_SELECT_CB)
rb_define_method(cSSLSocket, "alpn_protocol", ossl_ssl_alpn_protocol, 0);
#endif
#if !defined(OPENSSL_NO_NEXTPROTONEG)
rb_define_method(cSSLSocket, "npn_protocol", ossl_ssl_npn_protocol, 0);
#endif
#endif
rb_define_const(mSSL, "VERIFY_NONE", INT2NUM(SSL_VERIFY_NONE));
rb_define_const(mSSL, "VERIFY_PEER", INT2NUM(SSL_VERIFY_PEER));
rb_define_const(mSSL, "VERIFY_FAIL_IF_NO_PEER_CERT", INT2NUM(SSL_VERIFY_FAIL_IF_NO_PEER_CERT));
rb_define_const(mSSL, "VERIFY_CLIENT_ONCE", INT2NUM(SSL_VERIFY_CLIENT_ONCE));
rb_define_const(mSSL, "OP_ALL", ULONG2NUM(SSL_OP_ALL));
rb_define_const(mSSL, "OP_LEGACY_SERVER_CONNECT", ULONG2NUM(SSL_OP_LEGACY_SERVER_CONNECT));
#if defined(SSL_OP_TLSEXT_PADDING)
rb_define_const(mSSL, "OP_TLSEXT_PADDING", ULONG2NUM(SSL_OP_TLSEXT_PADDING));
#endif
#if defined(SSL_OP_SAFARI_ECDHE_ECDSA_BUG)
rb_define_const(mSSL, "OP_SAFARI_ECDHE_ECDSA_BUG", ULONG2NUM(SSL_OP_SAFARI_ECDHE_ECDSA_BUG));
#endif
#if defined(SSL_OP_ALLOW_NO_DHE_KEX)
rb_define_const(mSSL, "OP_ALLOW_NO_DHE_KEX", ULONG2NUM(SSL_OP_ALLOW_NO_DHE_KEX));
#endif
rb_define_const(mSSL, "OP_DONT_INSERT_EMPTY_FRAGMENTS", ULONG2NUM(SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS));
rb_define_const(mSSL, "OP_NO_TICKET", ULONG2NUM(SSL_OP_NO_TICKET));
rb_define_const(mSSL, "OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION", ULONG2NUM(SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION));
rb_define_const(mSSL, "OP_NO_COMPRESSION", ULONG2NUM(SSL_OP_NO_COMPRESSION));
rb_define_const(mSSL, "OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION", ULONG2NUM(SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION));
#if defined(SSL_OP_NO_ENCRYPT_THEN_MAC)
rb_define_const(mSSL, "OP_NO_ENCRYPT_THEN_MAC", ULONG2NUM(SSL_OP_NO_ENCRYPT_THEN_MAC));
#endif
rb_define_const(mSSL, "OP_CIPHER_SERVER_PREFERENCE", ULONG2NUM(SSL_OP_CIPHER_SERVER_PREFERENCE));
rb_define_const(mSSL, "OP_TLS_ROLLBACK_BUG", ULONG2NUM(SSL_OP_TLS_ROLLBACK_BUG));
#if defined(SSL_OP_NO_RENEGOTIATION)
rb_define_const(mSSL, "OP_NO_RENEGOTIATION", ULONG2NUM(SSL_OP_NO_RENEGOTIATION));
#endif
rb_define_const(mSSL, "OP_CRYPTOPRO_TLSEXT_BUG", ULONG2NUM(SSL_OP_CRYPTOPRO_TLSEXT_BUG));
rb_define_const(mSSL, "OP_NO_SSLv3", ULONG2NUM(SSL_OP_NO_SSLv3));
rb_define_const(mSSL, "OP_NO_TLSv1", ULONG2NUM(SSL_OP_NO_TLSv1));
rb_define_const(mSSL, "OP_NO_TLSv1_1", ULONG2NUM(SSL_OP_NO_TLSv1_1));
rb_define_const(mSSL, "OP_NO_TLSv1_2", ULONG2NUM(SSL_OP_NO_TLSv1_2));
#if defined(SSL_OP_NO_TLSv1_3)
rb_define_const(mSSL, "OP_NO_TLSv1_3", ULONG2NUM(SSL_OP_NO_TLSv1_3));
#endif
#if 0
rb_define_const(mSSL, "OP_NO_QUERY_MTU", ULONG2NUM(SSL_OP_NO_QUERY_MTU));
rb_define_const(mSSL, "OP_COOKIE_EXCHANGE", ULONG2NUM(SSL_OP_COOKIE_EXCHANGE));
rb_define_const(mSSL, "OP_CISCO_ANYCONNECT", ULONG2NUM(SSL_OP_CISCO_ANYCONNECT));
#endif
rb_define_const(mSSL, "OP_MICROSOFT_SESS_ID_BUG", ULONG2NUM(SSL_OP_MICROSOFT_SESS_ID_BUG));
rb_define_const(mSSL, "OP_NETSCAPE_CHALLENGE_BUG", ULONG2NUM(SSL_OP_NETSCAPE_CHALLENGE_BUG));
rb_define_const(mSSL, "OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG", ULONG2NUM(SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG));
rb_define_const(mSSL, "OP_SSLREF2_REUSE_CERT_TYPE_BUG", ULONG2NUM(SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG));
rb_define_const(mSSL, "OP_MICROSOFT_BIG_SSLV3_BUFFER", ULONG2NUM(SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER));
rb_define_const(mSSL, "OP_MSIE_SSLV2_RSA_PADDING", ULONG2NUM(SSL_OP_MSIE_SSLV2_RSA_PADDING));
rb_define_const(mSSL, "OP_SSLEAY_080_CLIENT_DH_BUG", ULONG2NUM(SSL_OP_SSLEAY_080_CLIENT_DH_BUG));
rb_define_const(mSSL, "OP_TLS_D5_BUG", ULONG2NUM(SSL_OP_TLS_D5_BUG));
rb_define_const(mSSL, "OP_TLS_BLOCK_PADDING_BUG", ULONG2NUM(SSL_OP_TLS_BLOCK_PADDING_BUG));
rb_define_const(mSSL, "OP_SINGLE_ECDH_USE", ULONG2NUM(SSL_OP_SINGLE_ECDH_USE));
rb_define_const(mSSL, "OP_SINGLE_DH_USE", ULONG2NUM(SSL_OP_SINGLE_DH_USE));
rb_define_const(mSSL, "OP_EPHEMERAL_RSA", ULONG2NUM(SSL_OP_EPHEMERAL_RSA));
rb_define_const(mSSL, "OP_NO_SSLv2", ULONG2NUM(SSL_OP_NO_SSLv2));
rb_define_const(mSSL, "OP_PKCS1_CHECK_1", ULONG2NUM(SSL_OP_PKCS1_CHECK_1));
rb_define_const(mSSL, "OP_PKCS1_CHECK_2", ULONG2NUM(SSL_OP_PKCS1_CHECK_2));
rb_define_const(mSSL, "OP_NETSCAPE_CA_DN_BUG", ULONG2NUM(SSL_OP_NETSCAPE_CA_DN_BUG));
rb_define_const(mSSL, "OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG", ULONG2NUM(SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG));
rb_define_const(mSSL, "SSL2_VERSION", INT2NUM(SSL2_VERSION));
rb_define_const(mSSL, "SSL3_VERSION", INT2NUM(SSL3_VERSION));
rb_define_const(mSSL, "TLS1_VERSION", INT2NUM(TLS1_VERSION));
rb_define_const(mSSL, "TLS1_1_VERSION", INT2NUM(TLS1_1_VERSION));
rb_define_const(mSSL, "TLS1_2_VERSION", INT2NUM(TLS1_2_VERSION));
#if defined(TLS1_3_VERSION)
rb_define_const(mSSL, "TLS1_3_VERSION", INT2NUM(TLS1_3_VERSION));
#endif
sym_exception = ID2SYM(rb_intern("exception"));
sym_wait_readable = ID2SYM(rb_intern("wait_readable"));
sym_wait_writable = ID2SYM(rb_intern("wait_writable"));
id_tmp_dh_callback = rb_intern("tmp_dh_callback");
id_tmp_ecdh_callback = rb_intern("tmp_ecdh_callback");
id_npn_protocols_encoded = rb_intern("npn_protocols_encoded");
#define DefIVarID(name) do id_i_##name = rb_intern("@"#name); while (0)
DefIVarID(cert_store);
DefIVarID(ca_file);
DefIVarID(ca_path);
DefIVarID(verify_mode);
DefIVarID(verify_depth);
DefIVarID(verify_callback);
DefIVarID(client_ca);
DefIVarID(renegotiation_cb);
DefIVarID(cert);
DefIVarID(key);
DefIVarID(extra_chain_cert);
DefIVarID(client_cert_cb);
DefIVarID(tmp_ecdh_callback);
DefIVarID(timeout);
DefIVarID(session_id_context);
DefIVarID(session_get_cb);
DefIVarID(session_new_cb);
DefIVarID(session_remove_cb);
DefIVarID(npn_select_cb);
DefIVarID(npn_protocols);
DefIVarID(alpn_protocols);
DefIVarID(alpn_select_cb);
DefIVarID(servername_cb);
DefIVarID(verify_hostname);
DefIVarID(io);
DefIVarID(context);
DefIVarID(hostname);
}
