












#include "tomcrypt_private.h"

#if defined(LTC_OCB3_MODE)







static int _ocb3_int_aad_add_block(ocb3_state *ocb, const unsigned char *aad_block)
{
unsigned char tmp[MAXBLOCKSIZE];
int err;


ocb3_int_xor_blocks(ocb->aOffset_current, ocb->aOffset_current, ocb->L_[ocb3_int_ntz(ocb->ablock_index)], ocb->block_len);


ocb3_int_xor_blocks(tmp, aad_block, ocb->aOffset_current, ocb->block_len);
if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(tmp, tmp, &ocb->key)) != CRYPT_OK) {
return err;
}
ocb3_int_xor_blocks(ocb->aSum_current, ocb->aSum_current, tmp, ocb->block_len);

ocb->ablock_index++;

return CRYPT_OK;
}








int ocb3_add_aad(ocb3_state *ocb, const unsigned char *aad, unsigned long aadlen)
{
int err, x, full_blocks, full_blocks_len, last_block_len;
unsigned char *data;
unsigned long datalen, l;

LTC_ARGCHK(ocb != NULL);
if (aadlen == 0) return CRYPT_OK;
LTC_ARGCHK(aad != NULL);

if (ocb->adata_buffer_bytes > 0) {
l = ocb->block_len - ocb->adata_buffer_bytes;
if (l > aadlen) l = aadlen;
XMEMCPY(ocb->adata_buffer+ocb->adata_buffer_bytes, aad, l);
ocb->adata_buffer_bytes += l;

if (ocb->adata_buffer_bytes == ocb->block_len) {
if ((err = _ocb3_int_aad_add_block(ocb, ocb->adata_buffer)) != CRYPT_OK) {
return err;
}
ocb->adata_buffer_bytes = 0;
}

data = (unsigned char *)aad + l;
datalen = aadlen - l;
}
else {
data = (unsigned char *)aad;
datalen = aadlen;
}

if (datalen == 0) return CRYPT_OK;

full_blocks = datalen/ocb->block_len;
full_blocks_len = full_blocks * ocb->block_len;
last_block_len = datalen - full_blocks_len;

for (x=0; x<full_blocks; x++) {
if ((err = _ocb3_int_aad_add_block(ocb, data+x*ocb->block_len)) != CRYPT_OK) {
return err;
}
}

if (last_block_len>0) {
XMEMCPY(ocb->adata_buffer, data+full_blocks_len, last_block_len);
ocb->adata_buffer_bytes = last_block_len;
}

return CRYPT_OK;
}

#endif




