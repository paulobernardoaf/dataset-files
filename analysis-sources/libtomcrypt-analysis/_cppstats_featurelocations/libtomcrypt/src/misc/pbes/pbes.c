







#include "tomcrypt_private.h"

#if defined(LTC_PBES)









int pbes_decrypt(const pbes_arg *arg, unsigned char *dec_data, unsigned long *dec_size)
{
int err, hid, cid;
unsigned char k[32], *iv;
unsigned long klen, keylen, dlen;
long diff;
symmetric_CBC cbc;

LTC_ARGCHK(arg != NULL);
LTC_ARGCHK(arg->type.kdf != NULL);
LTC_ARGCHK(dec_data != NULL);
LTC_ARGCHK(dec_size != NULL);

hid = find_hash(arg->type.h);
if (hid == -1) return CRYPT_INVALID_HASH;
cid = find_cipher(arg->type.c);
if (cid == -1) return CRYPT_INVALID_CIPHER;

klen = arg->type.keylen;


if (arg->key_bits != 0) {

if ((arg->key_bits % 8) != 0) return CRYPT_INVALID_KEYSIZE;

klen = arg->key_bits / 8;
}
keylen = klen;

if (arg->iv != NULL) {
iv = arg->iv->data;
} else {
iv = k + klen;
klen += arg->type.blocklen;
}

if (klen > sizeof(k)) return CRYPT_INVALID_ARG;

if ((err = arg->type.kdf(arg->pwd, arg->pwdlen, arg->salt->data, arg->salt->size, arg->iterations, hid, k, &klen)) != CRYPT_OK) goto LBL_ERROR;
if ((err = cbc_start(cid, iv, k, keylen, 0, &cbc)) != CRYPT_OK) goto LBL_ERROR;
if ((err = cbc_decrypt(arg->enc_data->data, dec_data, arg->enc_data->size, &cbc)) != CRYPT_OK) goto LBL_ERROR;
if ((err = cbc_done(&cbc)) != CRYPT_OK) goto LBL_ERROR;
dlen = arg->enc_data->size;
if ((err = padding_depad(dec_data, &dlen, LTC_PAD_PKCS7)) != CRYPT_OK) goto LBL_ERROR;
diff = (long)arg->enc_data->size - (long)dlen;
if ((diff <= 0) || (diff > cipher_descriptor[cid].block_length)) {
err = CRYPT_PK_INVALID_PADDING;
goto LBL_ERROR;
}
*dec_size = dlen;
return CRYPT_OK;

LBL_ERROR:
zeromem(k, sizeof(k));
zeromem(dec_data, *dec_size);
return err;
}

#endif




