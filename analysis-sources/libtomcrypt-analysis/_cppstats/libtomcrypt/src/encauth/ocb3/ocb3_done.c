#include "tomcrypt_private.h"
#if defined(LTC_OCB3_MODE)
int ocb3_done(ocb3_state *ocb, unsigned char *tag, unsigned long *taglen)
{
unsigned char tmp[MAXBLOCKSIZE];
int err, x;
LTC_ARGCHK(ocb != NULL);
LTC_ARGCHK(tag != NULL);
LTC_ARGCHK(taglen != NULL);
if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
goto LBL_ERR;
}
if ((int)*taglen < ocb->tag_len) {
*taglen = (unsigned long)ocb->tag_len;
return CRYPT_BUFFER_OVERFLOW;
}
if (ocb->adata_buffer_bytes>0) {
ocb3_int_xor_blocks(ocb->aOffset_current, ocb->aOffset_current, ocb->L_star, ocb->block_len);
ocb3_int_xor_blocks(tmp, ocb->adata_buffer, ocb->aOffset_current, ocb->adata_buffer_bytes);
for(x=ocb->adata_buffer_bytes; x<ocb->block_len; x++) {
if (x == ocb->adata_buffer_bytes) {
tmp[x] = 0x80 ^ ocb->aOffset_current[x];
}
else {
tmp[x] = 0x00 ^ ocb->aOffset_current[x];
}
}
if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(tmp, tmp, &ocb->key)) != CRYPT_OK) {
goto LBL_ERR;
}
ocb3_int_xor_blocks(ocb->aSum_current, ocb->aSum_current, tmp, ocb->block_len);
}
ocb3_int_xor_blocks(tmp, ocb->tag_part, ocb->aSum_current, ocb->block_len);
for(x = 0; x < ocb->tag_len; x++) tag[x] = tmp[x];
*taglen = (unsigned long)ocb->tag_len;
err = CRYPT_OK;
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(tmp, MAXBLOCKSIZE);
zeromem(ocb, sizeof(*ocb));
#endif
return err;
}
#endif
