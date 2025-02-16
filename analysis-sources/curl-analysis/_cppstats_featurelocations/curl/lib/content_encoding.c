





















#include "curl_setup.h"

#include "urldata.h"
#include <curl/curl.h>
#include <stddef.h>

#if defined(HAVE_ZLIB_H)
#include <zlib.h>
#if defined(__SYMBIAN32__)

#undef WIN32
#endif
#endif

#if defined(HAVE_BROTLI)
#include <brotli/decode.h>
#endif

#include "sendf.h"
#include "http.h"
#include "content_encoding.h"
#include "strdup.h"
#include "strcase.h"
#include "curl_memory.h"
#include "memdebug.h"

#define CONTENT_ENCODING_DEFAULT "identity"

#if !defined(CURL_DISABLE_HTTP)

#define DSIZ CURL_MAX_WRITE_SIZE 


#if defined(HAVE_LIBZ)



#define OLD_ZLIB_SUPPORT 1

#define GZIP_MAGIC_0 0x1f
#define GZIP_MAGIC_1 0x8b


#define ASCII_FLAG 0x01 
#define HEAD_CRC 0x02 
#define EXTRA_FIELD 0x04 
#define ORIG_NAME 0x08 
#define COMMENT 0x10 
#define RESERVED 0xE0 

typedef enum {
ZLIB_UNINIT, 
ZLIB_INIT, 
ZLIB_INFLATING, 
ZLIB_EXTERNAL_TRAILER, 
ZLIB_GZIP_HEADER, 
ZLIB_GZIP_INFLATING, 
ZLIB_INIT_GZIP 
} zlibInitState;


typedef struct {
zlibInitState zlib_init; 
uInt trailerlen; 
z_stream z; 
} zlib_params;


static voidpf
zalloc_cb(voidpf opaque, unsigned int items, unsigned int size)
{
(void) opaque;

return (voidpf) calloc(items, size);
}

static void
zfree_cb(voidpf opaque, voidpf ptr)
{
(void) opaque;
free(ptr);
}

static CURLcode
process_zlib_error(struct connectdata *conn, z_stream *z)
{
struct Curl_easy *data = conn->data;
if(z->msg)
failf(data, "Error while processing content unencoding: %s",
z->msg);
else
failf(data, "Error while processing content unencoding: "
"Unknown failure within decompression software.");

return CURLE_BAD_CONTENT_ENCODING;
}

static CURLcode
exit_zlib(struct connectdata *conn,
z_stream *z, zlibInitState *zlib_init, CURLcode result)
{
if(*zlib_init == ZLIB_GZIP_HEADER)
Curl_safefree(z->next_in);

if(*zlib_init != ZLIB_UNINIT) {
if(inflateEnd(z) != Z_OK && result == CURLE_OK)
result = process_zlib_error(conn, z);
*zlib_init = ZLIB_UNINIT;
}

return result;
}

static CURLcode process_trailer(struct connectdata *conn, zlib_params *zp)
{
z_stream *z = &zp->z;
CURLcode result = CURLE_OK;
uInt len = z->avail_in < zp->trailerlen? z->avail_in: zp->trailerlen;




zp->trailerlen -= len;
z->avail_in -= len;
z->next_in += len;
if(z->avail_in)
result = CURLE_WRITE_ERROR;
if(result || !zp->trailerlen)
result = exit_zlib(conn, z, &zp->zlib_init, result);
else {

zp->zlib_init = ZLIB_EXTERNAL_TRAILER;
}
return result;
}

static CURLcode inflate_stream(struct connectdata *conn,
contenc_writer *writer, zlibInitState started)
{
zlib_params *zp = (zlib_params *) &writer->params;
z_stream *z = &zp->z; 
uInt nread = z->avail_in;
Bytef *orig_in = z->next_in;
bool done = FALSE;
CURLcode result = CURLE_OK; 
char *decomp; 


if(zp->zlib_init != ZLIB_INIT &&
zp->zlib_init != ZLIB_INFLATING &&
zp->zlib_init != ZLIB_INIT_GZIP &&
zp->zlib_init != ZLIB_GZIP_INFLATING)
return exit_zlib(conn, z, &zp->zlib_init, CURLE_WRITE_ERROR);



decomp = malloc(DSIZ);
if(decomp == NULL)
return exit_zlib(conn, z, &zp->zlib_init, CURLE_OUT_OF_MEMORY);



while(!done) {
int status; 
done = TRUE;


z->next_out = (Bytef *) decomp;
z->avail_out = DSIZ;

#if defined(Z_BLOCK)

status = inflate(z, Z_BLOCK);
#else

status = inflate(z, Z_SYNC_FLUSH);
#endif


if(z->avail_out != DSIZ) {
if(status == Z_OK || status == Z_STREAM_END) {
zp->zlib_init = started; 
result = Curl_unencode_write(conn, writer->downstream, decomp,
DSIZ - z->avail_out);
if(result) {
exit_zlib(conn, z, &zp->zlib_init, result);
break;
}
}
}


switch(status) {
case Z_OK:

done = FALSE;
break;
case Z_BUF_ERROR:

break;
case Z_STREAM_END:
result = process_trailer(conn, zp);
break;
case Z_DATA_ERROR:


if(zp->zlib_init == ZLIB_INIT) {

(void) inflateEnd(z); 
if(inflateInit2(z, -MAX_WBITS) == Z_OK) {
z->next_in = orig_in;
z->avail_in = nread;
zp->zlib_init = ZLIB_INFLATING;
zp->trailerlen = 4; 
done = FALSE;
break;
}
zp->zlib_init = ZLIB_UNINIT; 
}

default:
result = exit_zlib(conn, z, &zp->zlib_init, process_zlib_error(conn, z));
break;
}
}
free(decomp);




if(nread && zp->zlib_init == ZLIB_INIT)
zp->zlib_init = started; 

return result;
}



static CURLcode deflate_init_writer(struct connectdata *conn,
contenc_writer *writer)
{
zlib_params *zp = (zlib_params *) &writer->params;
z_stream *z = &zp->z; 

if(!writer->downstream)
return CURLE_WRITE_ERROR;


z->zalloc = (alloc_func) zalloc_cb;
z->zfree = (free_func) zfree_cb;

if(inflateInit(z) != Z_OK)
return process_zlib_error(conn, z);
zp->zlib_init = ZLIB_INIT;
return CURLE_OK;
}

static CURLcode deflate_unencode_write(struct connectdata *conn,
contenc_writer *writer,
const char *buf, size_t nbytes)
{
zlib_params *zp = (zlib_params *) &writer->params;
z_stream *z = &zp->z; 


z->next_in = (Bytef *) buf;
z->avail_in = (uInt) nbytes;

if(zp->zlib_init == ZLIB_EXTERNAL_TRAILER)
return process_trailer(conn, zp);


return inflate_stream(conn, writer, ZLIB_INFLATING);
}

static void deflate_close_writer(struct connectdata *conn,
contenc_writer *writer)
{
zlib_params *zp = (zlib_params *) &writer->params;
z_stream *z = &zp->z; 

exit_zlib(conn, z, &zp->zlib_init, CURLE_OK);
}

static const content_encoding deflate_encoding = {
"deflate",
NULL,
deflate_init_writer,
deflate_unencode_write,
deflate_close_writer,
sizeof(zlib_params)
};



static CURLcode gzip_init_writer(struct connectdata *conn,
contenc_writer *writer)
{
zlib_params *zp = (zlib_params *) &writer->params;
z_stream *z = &zp->z; 

if(!writer->downstream)
return CURLE_WRITE_ERROR;


z->zalloc = (alloc_func) zalloc_cb;
z->zfree = (free_func) zfree_cb;

if(strcmp(zlibVersion(), "1.2.0.4") >= 0) {

if(inflateInit2(z, MAX_WBITS + 32) != Z_OK) {
return process_zlib_error(conn, z);
}
zp->zlib_init = ZLIB_INIT_GZIP; 
}
else {

if(inflateInit2(z, -MAX_WBITS) != Z_OK) {
return process_zlib_error(conn, z);
}
zp->trailerlen = 8; 
zp->zlib_init = ZLIB_INIT; 
}

return CURLE_OK;
}

#if defined(OLD_ZLIB_SUPPORT)

static enum {
GZIP_OK,
GZIP_BAD,
GZIP_UNDERFLOW
} check_gzip_header(unsigned char const *data, ssize_t len, ssize_t *headerlen)
{
int method, flags;
const ssize_t totallen = len;


if(len < 10)
return GZIP_UNDERFLOW;

if((data[0] != GZIP_MAGIC_0) || (data[1] != GZIP_MAGIC_1))
return GZIP_BAD;

method = data[2];
flags = data[3];

if(method != Z_DEFLATED || (flags & RESERVED) != 0) {

return GZIP_BAD;
}


len -= 10;
data += 10;

if(flags & EXTRA_FIELD) {
ssize_t extra_len;

if(len < 2)
return GZIP_UNDERFLOW;

extra_len = (data[1] << 8) | data[0];

if(len < (extra_len + 2))
return GZIP_UNDERFLOW;

len -= (extra_len + 2);
data += (extra_len + 2);
}

if(flags & ORIG_NAME) {

while(len && *data) {
--len;
++data;
}
if(!len || *data)
return GZIP_UNDERFLOW;


--len;
++data;
}

if(flags & COMMENT) {

while(len && *data) {
--len;
++data;
}
if(!len || *data)
return GZIP_UNDERFLOW;


--len;
}

if(flags & HEAD_CRC) {
if(len < 2)
return GZIP_UNDERFLOW;

len -= 2;
}

*headerlen = totallen - len;
return GZIP_OK;
}
#endif

static CURLcode gzip_unencode_write(struct connectdata *conn,
contenc_writer *writer,
const char *buf, size_t nbytes)
{
zlib_params *zp = (zlib_params *) &writer->params;
z_stream *z = &zp->z; 

if(zp->zlib_init == ZLIB_INIT_GZIP) {

z->next_in = (Bytef *) buf;
z->avail_in = (uInt) nbytes;

return inflate_stream(conn, writer, ZLIB_INIT_GZIP);
}

#if !defined(OLD_ZLIB_SUPPORT)


return exit_zlib(conn, z, &zp->zlib_init, CURLE_WRITE_ERROR);

#else










switch(zp->zlib_init) {

case ZLIB_INIT:
{

ssize_t hlen;

switch(check_gzip_header((unsigned char *) buf, nbytes, &hlen)) {
case GZIP_OK:
z->next_in = (Bytef *) buf + hlen;
z->avail_in = (uInt) (nbytes - hlen);
zp->zlib_init = ZLIB_GZIP_INFLATING; 
break;

case GZIP_UNDERFLOW:







z->avail_in = (uInt) nbytes;
z->next_in = malloc(z->avail_in);
if(z->next_in == NULL) {
return exit_zlib(conn, z, &zp->zlib_init, CURLE_OUT_OF_MEMORY);
}
memcpy(z->next_in, buf, z->avail_in);
zp->zlib_init = ZLIB_GZIP_HEADER; 

return CURLE_OK;

case GZIP_BAD:
default:
return exit_zlib(conn, z, &zp->zlib_init, process_zlib_error(conn, z));
}

}
break;

case ZLIB_GZIP_HEADER:
{

ssize_t hlen;
z->avail_in += (uInt) nbytes;
z->next_in = Curl_saferealloc(z->next_in, z->avail_in);
if(z->next_in == NULL) {
return exit_zlib(conn, z, &zp->zlib_init, CURLE_OUT_OF_MEMORY);
}

memcpy(z->next_in + z->avail_in - nbytes, buf, nbytes);

switch(check_gzip_header(z->next_in, z->avail_in, &hlen)) {
case GZIP_OK:

free(z->next_in);

z->next_in = (Bytef *) buf + hlen + nbytes - z->avail_in;
z->avail_in = (uInt) (z->avail_in - hlen);
zp->zlib_init = ZLIB_GZIP_INFLATING; 
break;

case GZIP_UNDERFLOW:

return CURLE_OK;

case GZIP_BAD:
default:
return exit_zlib(conn, z, &zp->zlib_init, process_zlib_error(conn, z));
}

}
break;

case ZLIB_EXTERNAL_TRAILER:
z->next_in = (Bytef *) buf;
z->avail_in = (uInt) nbytes;
return process_trailer(conn, zp);

case ZLIB_GZIP_INFLATING:
default:

z->next_in = (Bytef *) buf;
z->avail_in = (uInt) nbytes;
break;
}

if(z->avail_in == 0) {

return CURLE_OK;
}


return inflate_stream(conn, writer, ZLIB_GZIP_INFLATING);
#endif
}

static void gzip_close_writer(struct connectdata *conn,
contenc_writer *writer)
{
zlib_params *zp = (zlib_params *) &writer->params;
z_stream *z = &zp->z; 

exit_zlib(conn, z, &zp->zlib_init, CURLE_OK);
}

static const content_encoding gzip_encoding = {
"gzip",
"x-gzip",
gzip_init_writer,
gzip_unencode_write,
gzip_close_writer,
sizeof(zlib_params)
};

#endif 


#if defined(HAVE_BROTLI)


typedef struct {
BrotliDecoderState *br; 
} brotli_params;


static CURLcode brotli_map_error(BrotliDecoderErrorCode be)
{
switch(be) {
case BROTLI_DECODER_ERROR_FORMAT_EXUBERANT_NIBBLE:
case BROTLI_DECODER_ERROR_FORMAT_EXUBERANT_META_NIBBLE:
case BROTLI_DECODER_ERROR_FORMAT_SIMPLE_HUFFMAN_ALPHABET:
case BROTLI_DECODER_ERROR_FORMAT_SIMPLE_HUFFMAN_SAME:
case BROTLI_DECODER_ERROR_FORMAT_CL_SPACE:
case BROTLI_DECODER_ERROR_FORMAT_HUFFMAN_SPACE:
case BROTLI_DECODER_ERROR_FORMAT_CONTEXT_MAP_REPEAT:
case BROTLI_DECODER_ERROR_FORMAT_BLOCK_LENGTH_1:
case BROTLI_DECODER_ERROR_FORMAT_BLOCK_LENGTH_2:
case BROTLI_DECODER_ERROR_FORMAT_TRANSFORM:
case BROTLI_DECODER_ERROR_FORMAT_DICTIONARY:
case BROTLI_DECODER_ERROR_FORMAT_WINDOW_BITS:
case BROTLI_DECODER_ERROR_FORMAT_PADDING_1:
case BROTLI_DECODER_ERROR_FORMAT_PADDING_2:
#if defined(BROTLI_DECODER_ERROR_COMPOUND_DICTIONARY)
case BROTLI_DECODER_ERROR_COMPOUND_DICTIONARY:
#endif
#if defined(BROTLI_DECODER_ERROR_DICTIONARY_NOT_SET)
case BROTLI_DECODER_ERROR_DICTIONARY_NOT_SET:
#endif
case BROTLI_DECODER_ERROR_INVALID_ARGUMENTS:
return CURLE_BAD_CONTENT_ENCODING;
case BROTLI_DECODER_ERROR_ALLOC_CONTEXT_MODES:
case BROTLI_DECODER_ERROR_ALLOC_TREE_GROUPS:
case BROTLI_DECODER_ERROR_ALLOC_CONTEXT_MAP:
case BROTLI_DECODER_ERROR_ALLOC_RING_BUFFER_1:
case BROTLI_DECODER_ERROR_ALLOC_RING_BUFFER_2:
case BROTLI_DECODER_ERROR_ALLOC_BLOCK_TYPE_TREES:
return CURLE_OUT_OF_MEMORY;
default:
break;
}
return CURLE_WRITE_ERROR;
}

static CURLcode brotli_init_writer(struct connectdata *conn,
contenc_writer *writer)
{
brotli_params *bp = (brotli_params *) &writer->params;

(void) conn;

if(!writer->downstream)
return CURLE_WRITE_ERROR;

bp->br = BrotliDecoderCreateInstance(NULL, NULL, NULL);
return bp->br? CURLE_OK: CURLE_OUT_OF_MEMORY;
}

static CURLcode brotli_unencode_write(struct connectdata *conn,
contenc_writer *writer,
const char *buf, size_t nbytes)
{
brotli_params *bp = (brotli_params *) &writer->params;
const uint8_t *src = (const uint8_t *) buf;
char *decomp;
uint8_t *dst;
size_t dstleft;
CURLcode result = CURLE_OK;
BrotliDecoderResult r = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;

if(!bp->br)
return CURLE_WRITE_ERROR; 

decomp = malloc(DSIZ);
if(!decomp)
return CURLE_OUT_OF_MEMORY;

while((nbytes || r == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) &&
result == CURLE_OK) {
dst = (uint8_t *) decomp;
dstleft = DSIZ;
r = BrotliDecoderDecompressStream(bp->br,
&nbytes, &src, &dstleft, &dst, NULL);
result = Curl_unencode_write(conn, writer->downstream,
decomp, DSIZ - dstleft);
if(result)
break;
switch(r) {
case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT:
case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT:
break;
case BROTLI_DECODER_RESULT_SUCCESS:
BrotliDecoderDestroyInstance(bp->br);
bp->br = NULL;
if(nbytes)
result = CURLE_WRITE_ERROR;
break;
default:
result = brotli_map_error(BrotliDecoderGetErrorCode(bp->br));
break;
}
}
free(decomp);
return result;
}

static void brotli_close_writer(struct connectdata *conn,
contenc_writer *writer)
{
brotli_params *bp = (brotli_params *) &writer->params;

(void) conn;

if(bp->br) {
BrotliDecoderDestroyInstance(bp->br);
bp->br = NULL;
}
}

static const content_encoding brotli_encoding = {
"br",
NULL,
brotli_init_writer,
brotli_unencode_write,
brotli_close_writer,
sizeof(brotli_params)
};
#endif



static CURLcode identity_init_writer(struct connectdata *conn,
contenc_writer *writer)
{
(void) conn;
return writer->downstream? CURLE_OK: CURLE_WRITE_ERROR;
}

static CURLcode identity_unencode_write(struct connectdata *conn,
contenc_writer *writer,
const char *buf, size_t nbytes)
{
return Curl_unencode_write(conn, writer->downstream, buf, nbytes);
}

static void identity_close_writer(struct connectdata *conn,
contenc_writer *writer)
{
(void) conn;
(void) writer;
}

static const content_encoding identity_encoding = {
"identity",
"none",
identity_init_writer,
identity_unencode_write,
identity_close_writer,
0
};



static const content_encoding * const encodings[] = {
&identity_encoding,
#if defined(HAVE_LIBZ)
&deflate_encoding,
&gzip_encoding,
#endif
#if defined(HAVE_BROTLI)
&brotli_encoding,
#endif
NULL
};



char *Curl_all_content_encodings(void)
{
size_t len = 0;
const content_encoding * const *cep;
const content_encoding *ce;
char *ace;

for(cep = encodings; *cep; cep++) {
ce = *cep;
if(!strcasecompare(ce->name, CONTENT_ENCODING_DEFAULT))
len += strlen(ce->name) + 2;
}

if(!len)
return strdup(CONTENT_ENCODING_DEFAULT);

ace = malloc(len);
if(ace) {
char *p = ace;
for(cep = encodings; *cep; cep++) {
ce = *cep;
if(!strcasecompare(ce->name, CONTENT_ENCODING_DEFAULT)) {
strcpy(p, ce->name);
p += strlen(p);
*p++ = ',';
*p++ = ' ';
}
}
p[-2] = '\0';
}

return ace;
}



static CURLcode client_init_writer(struct connectdata *conn,
contenc_writer *writer)
{
(void) conn;
return writer->downstream? CURLE_WRITE_ERROR: CURLE_OK;
}

static CURLcode client_unencode_write(struct connectdata *conn,
contenc_writer *writer,
const char *buf, size_t nbytes)
{
struct Curl_easy *data = conn->data;
struct SingleRequest *k = &data->req;

(void) writer;

if(!nbytes || k->ignorebody)
return CURLE_OK;

return Curl_client_write(conn, CLIENTWRITE_BODY, (char *) buf, nbytes);
}

static void client_close_writer(struct connectdata *conn,
contenc_writer *writer)
{
(void) conn;
(void) writer;
}

static const content_encoding client_encoding = {
NULL,
NULL,
client_init_writer,
client_unencode_write,
client_close_writer,
0
};



static CURLcode error_init_writer(struct connectdata *conn,
contenc_writer *writer)
{
(void) conn;
return writer->downstream? CURLE_OK: CURLE_WRITE_ERROR;
}

static CURLcode error_unencode_write(struct connectdata *conn,
contenc_writer *writer,
const char *buf, size_t nbytes)
{
char *all = Curl_all_content_encodings();

(void) writer;
(void) buf;
(void) nbytes;

if(!all)
return CURLE_OUT_OF_MEMORY;
failf(conn->data, "Unrecognized content encoding type. "
"libcurl understands %s content encodings.", all);
free(all);
return CURLE_BAD_CONTENT_ENCODING;
}

static void error_close_writer(struct connectdata *conn,
contenc_writer *writer)
{
(void) conn;
(void) writer;
}

static const content_encoding error_encoding = {
NULL,
NULL,
error_init_writer,
error_unencode_write,
error_close_writer,
0
};


static contenc_writer *new_unencoding_writer(struct connectdata *conn,
const content_encoding *handler,
contenc_writer *downstream)
{
size_t sz = offsetof(contenc_writer, params) + handler->paramsize;
contenc_writer *writer = (contenc_writer *) calloc(1, sz);

if(writer) {
writer->handler = handler;
writer->downstream = downstream;
if(handler->init_writer(conn, writer)) {
free(writer);
writer = NULL;
}
}

return writer;
}


CURLcode Curl_unencode_write(struct connectdata *conn, contenc_writer *writer,
const char *buf, size_t nbytes)
{
if(!nbytes)
return CURLE_OK;
return writer->handler->unencode_write(conn, writer, buf, nbytes);
}


void Curl_unencode_cleanup(struct connectdata *conn)
{
struct Curl_easy *data = conn->data;
struct SingleRequest *k = &data->req;
contenc_writer *writer = k->writer_stack;

while(writer) {
k->writer_stack = writer->downstream;
writer->handler->close_writer(conn, writer);
free(writer);
writer = k->writer_stack;
}
}


static const content_encoding *find_encoding(const char *name, size_t len)
{
const content_encoding * const *cep;

for(cep = encodings; *cep; cep++) {
const content_encoding *ce = *cep;
if((strncasecompare(name, ce->name, len) && !ce->name[len]) ||
(ce->alias && strncasecompare(name, ce->alias, len) && !ce->alias[len]))
return ce;
}
return NULL;
}



CURLcode Curl_build_unencoding_stack(struct connectdata *conn,
const char *enclist, int maybechunked)
{
struct Curl_easy *data = conn->data;
struct SingleRequest *k = &data->req;

do {
const char *name;
size_t namelen;


while(ISSPACE(*enclist) || *enclist == ',')
enclist++;

name = enclist;

for(namelen = 0; *enclist && *enclist != ','; enclist++)
if(!ISSPACE(*enclist))
namelen = enclist - name + 1;


if(maybechunked && namelen == 7 && strncasecompare(name, "chunked", 7)) {
k->chunk = TRUE; 
Curl_httpchunk_init(conn); 
}
else if(namelen) {
const content_encoding *encoding = find_encoding(name, namelen);
contenc_writer *writer;

if(!k->writer_stack) {
k->writer_stack = new_unencoding_writer(conn, &client_encoding, NULL);

if(!k->writer_stack)
return CURLE_OUT_OF_MEMORY;
}

if(!encoding)
encoding = &error_encoding; 


writer = new_unencoding_writer(conn, encoding, k->writer_stack);
if(!writer)
return CURLE_OUT_OF_MEMORY;
k->writer_stack = writer;
}
} while(*enclist);

return CURLE_OK;
}

#else

CURLcode Curl_build_unencoding_stack(struct connectdata *conn,
const char *enclist, int maybechunked)
{
(void) conn;
(void) enclist;
(void) maybechunked;
return CURLE_NOT_BUILT_IN;
}

CURLcode Curl_unencode_write(struct connectdata *conn, contenc_writer *writer,
const char *buf, size_t nbytes)
{
(void) conn;
(void) writer;
(void) buf;
(void) nbytes;
return CURLE_NOT_BUILT_IN;
}

void Curl_unencode_cleanup(struct connectdata *conn)
{
(void) conn;
}

char *Curl_all_content_encodings(void)
{
return strdup(CONTENT_ENCODING_DEFAULT); 
}

#endif 
