








#include "tomcrypt.h"






#if defined(LTC_GCM_MODE)
#if !defined(LTC_NO_FILE)

#if defined(_MSC_VER)
#define ftruncate _chsize
#else
#include <unistd.h>
#endif




































int gcm_filehandle( int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *IV, unsigned long IVlen,
const unsigned char *adata, unsigned long adatalen,
FILE *in,
FILE *out,
unsigned long taglen,
int direction,
int *res)
{
void *orig;
gcm_state *gcm;
int err;
unsigned char *buf, tag[16];
size_t x, tot_data;
unsigned long tag_len;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(res != NULL);

*res = 0;

if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
return err;
}

#if !defined(LTC_GCM_TABLES_SSE2)
orig = gcm = XMALLOC(sizeof(*gcm));
#else
orig = gcm = XMALLOC(sizeof(*gcm) + 16);
#endif
if (gcm == NULL) {
return CRYPT_MEM;
}

if ((buf = XMALLOC(LTC_FILE_READ_BUFSIZE)) == NULL) {
XFREE(gcm);
return CRYPT_MEM;
}





#if defined(LTC_GCM_TABLES_SSE2)
if ((unsigned long)gcm & 15) {
gcm = (gcm_state *)((unsigned long)gcm + (16 - ((unsigned long)gcm & 15)));
}
#endif

if ((err = gcm_init(gcm, cipher, key, keylen)) != CRYPT_OK) {
goto LBL_ERR;
}
if ((err = gcm_add_iv(gcm, IV, IVlen)) != CRYPT_OK) {
goto LBL_ERR;
}
if ((err = gcm_add_aad(gcm, adata, adatalen)) != CRYPT_OK) {
goto LBL_ERR;
}

fseek(in, 0, SEEK_END);
tot_data = ftell(in);
if (direction == GCM_DECRYPT) {
tot_data -= taglen;
}
rewind(in);
do {
x = MIN(tot_data, LTC_FILE_READ_BUFSIZE);
x = fread(buf, 1, x, in);
tot_data -= x;
if ((err = gcm_process(gcm, buf, (unsigned long)x, buf, direction)) != CRYPT_OK) {
goto LBL_CLEANBUF;
}
if(fwrite(buf, 1, x, out) != x) {
err = CRYPT_ERROR;
goto LBL_CLEANBUF;
}
} while (x == LTC_FILE_READ_BUFSIZE);

tag_len = taglen;
if ((err = gcm_done(gcm, tag, &tag_len)) != CRYPT_OK) {
goto LBL_CLEANBUF;
}
if (tag_len != taglen) {
err = CRYPT_ERROR;
goto LBL_CLEANBUF;
}

if (direction == GCM_DECRYPT) {
x = fread(buf, 1, taglen, in);
if (x != taglen) {
err = CRYPT_ERROR;
goto LBL_CLEANBUF;
}

if (XMEM_NEQ(buf, tag, taglen) == 0) {
*res = 1;
}
} else {
if(fwrite(tag, 1, taglen, out) != taglen) {
err = CRYPT_ERROR;
goto LBL_CLEANBUF;
}
*res = 1;
}

LBL_CLEANBUF:
zeromem(buf, LTC_FILE_READ_BUFSIZE);
zeromem(tag, sizeof(tag));
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
#if !defined(LTC_GCM_TABLES_SSE2)
zeromem(orig, sizeof(*gcm));
#else
zeromem(orig, sizeof(*gcm) + 16);
#endif
#endif
if(*res == 0) {
x = ftell(out);
rewind(out);
while((size_t)ftell(out) < x) {
fwrite(buf, 1, LTC_FILE_READ_BUFSIZE, out);
}
if(ftruncate(fileno(out), 0)) {

}
}
fflush(out);

XFREE(buf);
XFREE(orig);

return err;
}
#endif
#endif





