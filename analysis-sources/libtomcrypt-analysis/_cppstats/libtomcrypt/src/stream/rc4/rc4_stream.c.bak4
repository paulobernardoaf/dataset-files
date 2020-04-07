








#include "tomcrypt_private.h"

#if defined(LTC_RC4_STREAM)








int rc4_stream_setup(rc4_state *st, const unsigned char *key, unsigned long keylen)
{
unsigned char tmp, *s;
int x, y;
unsigned long j;

LTC_ARGCHK(st != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(keylen >= 5); 

s = st->buf;
for (x = 0; x < 256; x++) {
s[x] = x;
}

for (j = x = y = 0; x < 256; x++) {
y = (y + s[x] + key[j++]) & 255;
if (j == keylen) {
j = 0;
}
tmp = s[x]; s[x] = s[y]; s[y] = tmp;
}
st->x = 0;
st->y = 0;

return CRYPT_OK;
}









int rc4_stream_crypt(rc4_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out)
{
unsigned char x, y, *s, tmp;

LTC_ARGCHK(st != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);

x = st->x;
y = st->y;
s = st->buf;
while (inlen--) {
x = (x + 1) & 255;
y = (y + s[x]) & 255;
tmp = s[x]; s[x] = s[y]; s[y] = tmp;
tmp = (s[x] + s[y]) & 255;
*out++ = *in++ ^ s[tmp];
}
st->x = x;
st->y = y;
return CRYPT_OK;
}








int rc4_stream_keystream(rc4_state *st, unsigned char *out, unsigned long outlen)
{
if (outlen == 0) return CRYPT_OK; 
LTC_ARGCHK(out != NULL);
XMEMSET(out, 0, outlen);
return rc4_stream_crypt(st, out, outlen, out);
}






int rc4_stream_done(rc4_state *st)
{
LTC_ARGCHK(st != NULL);
zeromem(st, sizeof(rc4_state));
return CRYPT_OK;
}

#endif




