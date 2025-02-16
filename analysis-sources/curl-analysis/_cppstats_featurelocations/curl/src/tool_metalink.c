




















#include "tool_setup.h"

#if defined(USE_METALINK)

#include <sys/stat.h>
#include <stdlib.h>

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#undef HAVE_NSS_CONTEXT

#if defined(USE_OPENSSL)
#include <openssl/md5.h>
#include <openssl/sha.h>
#elif defined(USE_GNUTLS_NETTLE)
#include <nettle/md5.h>
#include <nettle/sha.h>
#define MD5_CTX struct md5_ctx
#define SHA_CTX struct sha1_ctx
#define SHA256_CTX struct sha256_ctx
#elif defined(USE_GNUTLS)
#include <gcrypt.h>
#define MD5_CTX gcry_md_hd_t
#define SHA_CTX gcry_md_hd_t
#define SHA256_CTX gcry_md_hd_t
#elif defined(USE_NSS)
#include <nss.h>
#include <pk11pub.h>
#define MD5_CTX void *
#define SHA_CTX void *
#define SHA256_CTX void *
#define HAVE_NSS_CONTEXT
static NSSInitContext *nss_context;
#elif (defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 1040)) || (defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && (__IPHONE_OS_VERSION_MAX_ALLOWED >= 20000))









#define COMMON_DIGEST_FOR_OPENSSL
#include <CommonCrypto/CommonDigest.h>
#elif defined(USE_WIN32_CRYPTO)


#include <wincrypt.h>

struct win32_crypto_hash {
HCRYPTPROV hCryptProv;
HCRYPTHASH hHash;
};

#if !defined(ALG_SID_SHA_256)
#define ALG_SID_SHA_256 12
#endif
#if !defined(CALG_SHA_256)
#define CALG_SHA_256 (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA_256)
#endif
#define MD5_CTX struct win32_crypto_hash
#define SHA_CTX struct win32_crypto_hash
#define SHA256_CTX struct win32_crypto_hash
#else
#error "Can't compile METALINK support without a crypto library."
#endif

#define ENABLE_CURLX_PRINTF

#include "curlx.h"

#include "tool_getparam.h"
#include "tool_paramhlp.h"
#include "tool_cfgable.h"
#include "tool_metalink.h"
#include "tool_operate.h"
#include "tool_msgs.h"

#include "memdebug.h" 


#define GetStr(str,val) do { if(*(str)) { free(*(str)); *(str) = NULL; } if((val)) *(str) = strdup((val)); if(!(val)) return PARAM_NO_MEM; } while(0)










#if defined(USE_OPENSSL)

#elif defined(USE_GNUTLS_NETTLE)

static int MD5_Init(MD5_CTX *ctx)
{
md5_init(ctx);
return 1;
}

static void MD5_Update(MD5_CTX *ctx,
const unsigned char *input,
unsigned int inputLen)
{
md5_update(ctx, inputLen, input);
}

static void MD5_Final(unsigned char digest[16], MD5_CTX *ctx)
{
md5_digest(ctx, 16, digest);
}

static int SHA1_Init(SHA_CTX *ctx)
{
sha1_init(ctx);
return 1;
}

static void SHA1_Update(SHA_CTX *ctx,
const unsigned char *input,
unsigned int inputLen)
{
sha1_update(ctx, inputLen, input);
}

static void SHA1_Final(unsigned char digest[20], SHA_CTX *ctx)
{
sha1_digest(ctx, 20, digest);
}

static int SHA256_Init(SHA256_CTX *ctx)
{
sha256_init(ctx);
return 1;
}

static void SHA256_Update(SHA256_CTX *ctx,
const unsigned char *input,
unsigned int inputLen)
{
sha256_update(ctx, inputLen, input);
}

static void SHA256_Final(unsigned char digest[32], SHA256_CTX *ctx)
{
sha256_digest(ctx, 32, digest);
}

#elif defined(USE_GNUTLS)

static int MD5_Init(MD5_CTX *ctx)
{
gcry_md_open(ctx, GCRY_MD_MD5, 0);
return 1;
}

static void MD5_Update(MD5_CTX *ctx,
const unsigned char *input,
unsigned int inputLen)
{
gcry_md_write(*ctx, input, inputLen);
}

static void MD5_Final(unsigned char digest[16], MD5_CTX *ctx)
{
memcpy(digest, gcry_md_read(*ctx, 0), 16);
gcry_md_close(*ctx);
}

static int SHA1_Init(SHA_CTX *ctx)
{
gcry_md_open(ctx, GCRY_MD_SHA1, 0);
return 1;
}

static void SHA1_Update(SHA_CTX *ctx,
const unsigned char *input,
unsigned int inputLen)
{
gcry_md_write(*ctx, input, inputLen);
}

static void SHA1_Final(unsigned char digest[20], SHA_CTX *ctx)
{
memcpy(digest, gcry_md_read(*ctx, 0), 20);
gcry_md_close(*ctx);
}

static int SHA256_Init(SHA256_CTX *ctx)
{
gcry_md_open(ctx, GCRY_MD_SHA256, 0);
return 1;
}

static void SHA256_Update(SHA256_CTX *ctx,
const unsigned char *input,
unsigned int inputLen)
{
gcry_md_write(*ctx, input, inputLen);
}

static void SHA256_Final(unsigned char digest[32], SHA256_CTX *ctx)
{
memcpy(digest, gcry_md_read(*ctx, 0), 32);
gcry_md_close(*ctx);
}

#elif defined(USE_NSS)

static int nss_hash_init(void **pctx, SECOidTag hash_alg)
{
PK11Context *ctx;


if(!NSS_IsInitialized() && !nss_context) {
static NSSInitParameters params;
params.length = sizeof(params);
nss_context = NSS_InitContext("", "", "", "", &params, NSS_INIT_READONLY
| NSS_INIT_NOCERTDB | NSS_INIT_NOMODDB | NSS_INIT_FORCEOPEN
| NSS_INIT_NOROOTINIT | NSS_INIT_OPTIMIZESPACE | NSS_INIT_PK11RELOAD);
}

ctx = PK11_CreateDigestContext(hash_alg);
if(!ctx)
return 0;

if(PK11_DigestBegin(ctx) != SECSuccess) {
PK11_DestroyContext(ctx, PR_TRUE);
return 0;
}

*pctx = ctx;
return 1;
}

static void nss_hash_final(void **pctx, unsigned char *out, unsigned int len)
{
PK11Context *ctx = *pctx;
unsigned int outlen;
PK11_DigestFinal(ctx, out, &outlen, len);
PK11_DestroyContext(ctx, PR_TRUE);
}

static int MD5_Init(MD5_CTX *pctx)
{
return nss_hash_init(pctx, SEC_OID_MD5);
}

static void MD5_Update(MD5_CTX *pctx,
const unsigned char *input,
unsigned int input_len)
{
PK11_DigestOp(*pctx, input, input_len);
}

static void MD5_Final(unsigned char digest[16], MD5_CTX *pctx)
{
nss_hash_final(pctx, digest, 16);
}

static int SHA1_Init(SHA_CTX *pctx)
{
return nss_hash_init(pctx, SEC_OID_SHA1);
}

static void SHA1_Update(SHA_CTX *pctx,
const unsigned char *input,
unsigned int input_len)
{
PK11_DigestOp(*pctx, input, input_len);
}

static void SHA1_Final(unsigned char digest[20], SHA_CTX *pctx)
{
nss_hash_final(pctx, digest, 20);
}

static int SHA256_Init(SHA256_CTX *pctx)
{
return nss_hash_init(pctx, SEC_OID_SHA256);
}

static void SHA256_Update(SHA256_CTX *pctx,
const unsigned char *input,
unsigned int input_len)
{
PK11_DigestOp(*pctx, input, input_len);
}

static void SHA256_Final(unsigned char digest[32], SHA256_CTX *pctx)
{
nss_hash_final(pctx, digest, 32);
}

#elif defined(USE_WIN32_CRYPTO)

static void win32_crypto_final(struct win32_crypto_hash *ctx,
unsigned char *digest,
unsigned int digestLen)
{
unsigned long length;
CryptGetHashParam(ctx->hHash, HP_HASHVAL, NULL, &length, 0);
if(length == digestLen)
CryptGetHashParam(ctx->hHash, HP_HASHVAL, digest, &length, 0);
if(ctx->hHash)
CryptDestroyHash(ctx->hHash);
if(ctx->hCryptProv)
CryptReleaseContext(ctx->hCryptProv, 0);
}

static int MD5_Init(MD5_CTX *ctx)
{
if(CryptAcquireContext(&ctx->hCryptProv, NULL, NULL, PROV_RSA_FULL,
CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
CryptCreateHash(ctx->hCryptProv, CALG_MD5, 0, 0, &ctx->hHash);
}
return 1;
}

static void MD5_Update(MD5_CTX *ctx,
const unsigned char *input,
unsigned int inputLen)
{
CryptHashData(ctx->hHash, (unsigned char *)input, inputLen, 0);
}

static void MD5_Final(unsigned char digest[16], MD5_CTX *ctx)
{
win32_crypto_final(ctx, digest, 16);
}

static int SHA1_Init(SHA_CTX *ctx)
{
if(CryptAcquireContext(&ctx->hCryptProv, NULL, NULL, PROV_RSA_FULL,
CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
CryptCreateHash(ctx->hCryptProv, CALG_SHA1, 0, 0, &ctx->hHash);
}
return 1;
}

static void SHA1_Update(SHA_CTX *ctx,
const unsigned char *input,
unsigned int inputLen)
{
CryptHashData(ctx->hHash, (unsigned char *)input, inputLen, 0);
}

static void SHA1_Final(unsigned char digest[20], SHA_CTX *ctx)
{
win32_crypto_final(ctx, digest, 20);
}

static int SHA256_Init(SHA256_CTX *ctx)
{
if(CryptAcquireContext(&ctx->hCryptProv, NULL, NULL, PROV_RSA_AES,
CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
CryptCreateHash(ctx->hCryptProv, CALG_SHA_256, 0, 0, &ctx->hHash);
}
return 1;
}

static void SHA256_Update(SHA256_CTX *ctx,
const unsigned char *input,
unsigned int inputLen)
{
CryptHashData(ctx->hHash, (unsigned char *)input, inputLen, 0);
}

static void SHA256_Final(unsigned char digest[32], SHA256_CTX *ctx)
{
win32_crypto_final(ctx, digest, 32);
}

#endif 

const digest_params MD5_DIGEST_PARAMS[] = {
{
CURLX_FUNCTION_CAST(Curl_digest_init_func, MD5_Init),
CURLX_FUNCTION_CAST(Curl_digest_update_func, MD5_Update),
CURLX_FUNCTION_CAST(Curl_digest_final_func, MD5_Final),
sizeof(MD5_CTX),
16
}
};

const digest_params SHA1_DIGEST_PARAMS[] = {
{
CURLX_FUNCTION_CAST(Curl_digest_init_func, SHA1_Init),
CURLX_FUNCTION_CAST(Curl_digest_update_func, SHA1_Update),
CURLX_FUNCTION_CAST(Curl_digest_final_func, SHA1_Final),
sizeof(SHA_CTX),
20
}
};

const digest_params SHA256_DIGEST_PARAMS[] = {
{
CURLX_FUNCTION_CAST(Curl_digest_init_func, SHA256_Init),
CURLX_FUNCTION_CAST(Curl_digest_update_func, SHA256_Update),
CURLX_FUNCTION_CAST(Curl_digest_final_func, SHA256_Final),
sizeof(SHA256_CTX),
32
}
};

static const metalink_digest_def SHA256_DIGEST_DEF[] = {
{"sha-256", SHA256_DIGEST_PARAMS}
};

static const metalink_digest_def SHA1_DIGEST_DEF[] = {
{"sha-1", SHA1_DIGEST_PARAMS}
};

static const metalink_digest_def MD5_DIGEST_DEF[] = {
{"md5", MD5_DIGEST_PARAMS}
};








static const metalink_digest_alias digest_aliases[] = {
{"sha-256", SHA256_DIGEST_DEF},
{"sha256", SHA256_DIGEST_DEF},
{"sha-1", SHA1_DIGEST_DEF},
{"sha1", SHA1_DIGEST_DEF},
{"md5", MD5_DIGEST_DEF},
{NULL, NULL}
};

digest_context *Curl_digest_init(const digest_params *dparams)
{
digest_context *ctxt;


ctxt = malloc(sizeof(*ctxt));

if(!ctxt)
return ctxt;

ctxt->digest_hashctx = malloc(dparams->digest_ctxtsize);

if(!ctxt->digest_hashctx) {
free(ctxt);
return NULL;
}

ctxt->digest_hash = dparams;

if(dparams->digest_init(ctxt->digest_hashctx) != 1) {
free(ctxt->digest_hashctx);
free(ctxt);
return NULL;
}

return ctxt;
}

int Curl_digest_update(digest_context *context,
const unsigned char *data,
unsigned int len)
{
(*context->digest_hash->digest_update)(context->digest_hashctx, data, len);

return 0;
}

int Curl_digest_final(digest_context *context, unsigned char *result)
{
if(result)
(*context->digest_hash->digest_final)(result, context->digest_hashctx);

free(context->digest_hashctx);
free(context);

return 0;
}

static unsigned char hex_to_uint(const char *s)
{
char buf[3];
unsigned long val;
buf[0] = s[0];
buf[1] = s[1];
buf[2] = 0;
val = strtoul(buf, NULL, 16);
return (unsigned char)(val&0xff);
}















static int check_hash(const char *filename,
const metalink_digest_def *digest_def,
const unsigned char *digest, FILE *error)
{
unsigned char *result;
digest_context *dctx;
int check_ok, flags, fd;

flags = O_RDONLY;
#if defined(O_BINARY)

flags |= O_BINARY;
#endif

fd = open(filename, flags);
if(fd == -1) {
fprintf(error, "Metalink: validating (%s) [%s] FAILED (%s)\n", filename,
digest_def->hash_name, strerror(errno));
return -1;
}

dctx = Curl_digest_init(digest_def->dparams);
if(!dctx) {
fprintf(error, "Metalink: validating (%s) [%s] FAILED (%s)\n", filename,
digest_def->hash_name, "failed to initialize hash algorithm");
close(fd);
return -2;
}

result = malloc(digest_def->dparams->digest_resultlen);
if(!result) {
close(fd);
Curl_digest_final(dctx, NULL);
return -1;
}
while(1) {
unsigned char buf[4096];
ssize_t len = read(fd, buf, sizeof(buf));
if(len == 0) {
break;
}
else if(len == -1) {
fprintf(error, "Metalink: validating (%s) [%s] FAILED (%s)\n", filename,
digest_def->hash_name, strerror(errno));
Curl_digest_final(dctx, result);
close(fd);
return -1;
}
Curl_digest_update(dctx, buf, (unsigned int)len);
}
Curl_digest_final(dctx, result);
check_ok = memcmp(result, digest,
digest_def->dparams->digest_resultlen) == 0;

if(check_ok)
fprintf(error, "Metalink: validating (%s) [%s] OK\n", filename,
digest_def->hash_name);
else
fprintf(error, "Metalink: validating (%s) [%s] FAILED (digest mismatch)\n",
filename, digest_def->hash_name);

free(result);
close(fd);
return check_ok;
}

int metalink_check_hash(struct GlobalConfig *config,
metalinkfile *mlfile,
const char *filename)
{
int rv;
fprintf(config->errors, "Metalink: validating (%s)...\n", filename);
if(mlfile->checksum == NULL) {
fprintf(config->errors,
"Metalink: validating (%s) FAILED (digest missing)\n", filename);
return -2;
}
rv = check_hash(filename, mlfile->checksum->digest_def,
mlfile->checksum->digest, config->errors);
return rv;
}

static metalink_checksum *
checksum_from_hex_digest(const metalink_digest_def *digest_def,
const char *hex_digest)
{
metalink_checksum *chksum;
unsigned char *digest;
size_t i;
size_t len = strlen(hex_digest);
digest = malloc(len/2);
if(!digest)
return 0;

for(i = 0; i < len; i += 2) {
digest[i/2] = hex_to_uint(hex_digest + i);
}
chksum = malloc(sizeof(metalink_checksum));
if(chksum) {
chksum->digest_def = digest_def;
chksum->digest = digest;
}
else
free(digest);
return chksum;
}

static metalink_resource *new_metalink_resource(const char *url)
{
metalink_resource *res;
res = malloc(sizeof(metalink_resource));
if(res) {
res->next = NULL;
res->url = strdup(url);
if(!res->url) {
free(res);
return NULL;
}
}
return res;
}




static int check_hex_digest(const char *hex_digest,
const metalink_digest_def *digest_def)
{
size_t i;
for(i = 0; hex_digest[i]; ++i) {
char c = hex_digest[i];
if(!(('0' <= c && c <= '9') || ('a' <= c && c <= 'z') ||
('A' <= c && c <= 'Z'))) {
return 0;
}
}
return digest_def->dparams->digest_resultlen * 2 == i;
}

static metalinkfile *new_metalinkfile(metalink_file_t *fileinfo)
{
metalinkfile *f;
f = (metalinkfile*)malloc(sizeof(metalinkfile));
if(!f)
return NULL;

f->next = NULL;
f->filename = strdup(fileinfo->name);
if(!f->filename) {
free(f);
return NULL;
}
f->checksum = NULL;
f->resource = NULL;
if(fileinfo->checksums) {
const metalink_digest_alias *digest_alias;
for(digest_alias = digest_aliases; digest_alias->alias_name;
++digest_alias) {
metalink_checksum_t **p;
for(p = fileinfo->checksums; *p; ++p) {
if(curl_strequal(digest_alias->alias_name, (*p)->type) &&
check_hex_digest((*p)->hash, digest_alias->digest_def)) {
f->checksum =
checksum_from_hex_digest(digest_alias->digest_def,
(*p)->hash);
break;
}
}
if(f->checksum) {
break;
}
}
}
if(fileinfo->resources) {
metalink_resource_t **p;
metalink_resource root, *tail;
root.next = NULL;
tail = &root;
for(p = fileinfo->resources; *p; ++p) {
metalink_resource *res;









if((*p)->type == NULL ||
curl_strequal((*p)->type, "http") ||
curl_strequal((*p)->type, "https") ||
curl_strequal((*p)->type, "ftp") ||
curl_strequal((*p)->type, "ftps")) {
res = new_metalink_resource((*p)->url);
if(res) {
tail->next = res;
tail = res;
}
else {
tail = root.next;


while(tail) {
res = tail->next;
free(tail->url);
free(tail);
tail = res;
}
free(f->filename);
free(f);
return NULL;
}
}
}
f->resource = root.next;
}
return f;
}

int parse_metalink(struct OperationConfig *config, struct OutStruct *outs,
const char *metalink_url)
{
metalink_error_t r;
metalink_t* metalink;
metalink_file_t **files;
bool warnings = FALSE;


r = metalink_parse_final(outs->metalink_parser, NULL, 0, &metalink);
outs->metalink_parser = NULL;
if(r != 0) {
return -1;
}
if(metalink->files == NULL) {
fprintf(config->global->errors, "Metalink: parsing (%s) WARNING "
"(missing or invalid file name)\n",
metalink_url);
metalink_delete(metalink);
return -1;
}
for(files = metalink->files; *files; ++files) {
struct getout *url;

if(!(*files)->resources) {
fprintf(config->global->errors, "Metalink: parsing (%s) WARNING "
"(missing or invalid resource)\n",
metalink_url);
continue;
}
if(config->url_get ||
((config->url_get = config->url_list) != NULL)) {


while(config->url_get && (config->url_get->flags & GETOUT_URL))
config->url_get = config->url_get->next;
}



if(config->url_get)

url = config->url_get;
else

url = new_getout(config);

if(url) {
metalinkfile *mlfile = new_metalinkfile(*files);
if(!mlfile)
break;

if(!mlfile->checksum) {
warnings = TRUE;
fprintf(config->global->errors,
"Metalink: parsing (%s) WARNING (digest missing)\n",
metalink_url);
}

GetStr(&url->url, mlfile->filename);


url->flags |= GETOUT_URL | GETOUT_METALINK;

if(config->metalinkfile_list) {
config->metalinkfile_last->next = mlfile;
config->metalinkfile_last = mlfile;
}
else {
config->metalinkfile_list = config->metalinkfile_last = mlfile;
}
}
}
metalink_delete(metalink);
return (warnings) ? -2 : 0;
}

size_t metalink_write_cb(void *buffer, size_t sz, size_t nmemb,
void *userdata)
{
struct per_transfer *per = userdata;
struct OutStruct *outs = &per->outs;
struct OperationConfig *config = per->config;
int rv;







const size_t failure = (sz && nmemb) ? 0 : 1;

if(!config)
return failure;

rv = metalink_parse_update(outs->metalink_parser, buffer, sz * nmemb);
if(rv == 0)
return sz * nmemb;
else {
fprintf(config->global->errors, "Metalink: parsing FAILED\n");
return failure;
}
}




static int check_content_type(const char *content_type, const char *media_type)
{
const char *ptr = content_type;
size_t media_type_len = strlen(media_type);
for(; *ptr && (*ptr == ' ' || *ptr == '\t'); ++ptr);
if(!*ptr) {
return 0;
}
return curl_strnequal(ptr, media_type, media_type_len) &&
(*(ptr + media_type_len) == '\0' || *(ptr + media_type_len) == ' ' ||
*(ptr + media_type_len) == '\t' || *(ptr + media_type_len) == ';');
}

int check_metalink_content_type(const char *content_type)
{
return check_content_type(content_type, "application/metalink+xml");
}

int count_next_metalink_resource(metalinkfile *mlfile)
{
int count = 0;
metalink_resource *res;
for(res = mlfile->resource; res; res = res->next, ++count);
return count;
}

static void delete_metalink_checksum(metalink_checksum *chksum)
{
if(chksum == NULL) {
return;
}
Curl_safefree(chksum->digest);
Curl_safefree(chksum);
}

static void delete_metalink_resource(metalink_resource *res)
{
if(res == NULL) {
return;
}
Curl_safefree(res->url);
Curl_safefree(res);
}

void delete_metalinkfile(metalinkfile *mlfile)
{
metalink_resource *res;
if(mlfile == NULL) {
return;
}
Curl_safefree(mlfile->filename);
delete_metalink_checksum(mlfile->checksum);
for(res = mlfile->resource; res;) {
metalink_resource *next;
next = res->next;
delete_metalink_resource(res);
res = next;
}
Curl_safefree(mlfile);
}

void clean_metalink(struct OperationConfig *config)
{
if(config) {
while(config->metalinkfile_list) {
metalinkfile *mlfile = config->metalinkfile_list;
config->metalinkfile_list = config->metalinkfile_list->next;
delete_metalinkfile(mlfile);
}
config->metalinkfile_last = 0;
}
}

void metalink_cleanup(void)
{
#if defined(HAVE_NSS_CONTEXT)
if(nss_context) {
NSS_ShutdownContext(nss_context);
nss_context = NULL;
}
#endif
}

#endif 
