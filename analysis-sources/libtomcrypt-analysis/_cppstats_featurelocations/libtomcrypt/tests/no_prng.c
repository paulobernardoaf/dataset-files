








#include "tomcrypt.h"
#include "tomcrypt_test.h"






#if defined(LTC_PKCS_1)

typedef struct
{
struct ltc_prng_descriptor desc;
char name[64];
unsigned char entropy[1024];
unsigned long len;
unsigned long offset;
} no_prng_desc_t;






static int no_prng_start(prng_state *prng)
{
no_prng_desc_t *no_prng = (no_prng_desc_t*) prng;
LTC_ARGCHK(no_prng != NULL);
LTC_ARGCHK(no_prng->name == (char*)no_prng + offsetof(no_prng_desc_t, name));
no_prng->len = 0;
no_prng->offset = 0;

return CRYPT_OK;
}








static int no_prng_add_entropy(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
no_prng_desc_t *no_prng = (no_prng_desc_t*) prng;
LTC_ARGCHK(no_prng != NULL);
LTC_ARGCHK(no_prng->name == (char*)no_prng + offsetof(no_prng_desc_t, name));
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(inlen <= sizeof(no_prng->entropy));

no_prng->len = MIN(inlen, sizeof(no_prng->entropy));
memcpy(no_prng->entropy, in, no_prng->len);
no_prng->offset = 0;

return CRYPT_OK;

}






static int no_prng_ready(prng_state *prng)
{
LTC_ARGCHK(prng != NULL);

return CRYPT_OK;
}








static unsigned long no_prng_read(unsigned char *out, unsigned long outlen, prng_state *prng)
{
no_prng_desc_t *no_prng = (no_prng_desc_t*) prng;
LTC_ARGCHK(no_prng != NULL);
LTC_ARGCHK(no_prng->name == (char*)no_prng + offsetof(no_prng_desc_t, name));
LTC_ARGCHK(out != NULL);

outlen = MIN(outlen, no_prng->len - no_prng->offset);
memcpy(out, &no_prng->entropy[no_prng->offset], outlen);
no_prng->offset += outlen;

return outlen;
}






static int no_prng_done(prng_state *prng)
{
LTC_UNUSED_PARAM(prng);
return CRYPT_OK;
}








static int no_prng_export(unsigned char *out, unsigned long *outlen, prng_state *prng)
{
LTC_UNUSED_PARAM(out);
LTC_UNUSED_PARAM(outlen);
LTC_UNUSED_PARAM(prng);
return CRYPT_OK;
}








static int no_prng_import(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
LTC_UNUSED_PARAM(in);
LTC_UNUSED_PARAM(inlen);
LTC_UNUSED_PARAM(prng);
return CRYPT_OK;
}





static int no_prng_test(void)
{
return CRYPT_OK;
}

static const struct ltc_prng_descriptor no_prng_desc =
{
NULL, 0,
&no_prng_start,
&no_prng_add_entropy,
&no_prng_ready,
&no_prng_read,
&no_prng_done,
&no_prng_export,
&no_prng_import,
&no_prng_test
};

struct ltc_prng_descriptor* no_prng_desc_get(void)
{
int ret;
no_prng_desc_t* no_prng = XMALLOC(sizeof(*no_prng));
if (no_prng == NULL) return NULL;
XMEMCPY(&no_prng->desc, &no_prng_desc, sizeof(no_prng_desc));
ret = snprintf(no_prng->name, sizeof(no_prng->name), "no_prng@%p", no_prng);
if((ret >= (int)sizeof(no_prng->name)) || (ret == -1)) {
XFREE(no_prng);
return NULL;
}
no_prng->desc.name = no_prng->name;
return &no_prng->desc;
}

void no_prng_desc_free(struct ltc_prng_descriptor* prng)
{
no_prng_desc_t *no_prng = (no_prng_desc_t*) prng;
LTC_ARGCHKVD(no_prng != NULL);
LTC_ARGCHKVD(no_prng->name == (char*)no_prng + offsetof(no_prng_desc_t, name));
XFREE(no_prng);
}

#endif





