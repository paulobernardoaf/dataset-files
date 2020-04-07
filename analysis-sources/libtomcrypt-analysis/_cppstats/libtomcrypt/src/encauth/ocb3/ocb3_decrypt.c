#include "tomcrypt_private.h"
#if defined(LTC_OCB3_MODE)
int ocb3_decrypt(ocb3_state *ocb, const unsigned char *ct, unsigned long ctlen, unsigned char *pt)
{
unsigned char tmp[MAXBLOCKSIZE];
int err, i, full_blocks;
unsigned char *pt_b, *ct_b;
LTC_ARGCHK(ocb != NULL);
if (ctlen == 0) return CRYPT_OK; 
LTC_ARGCHK(ct != NULL);
LTC_ARGCHK(pt != NULL);
if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
return err;
}
if (ocb->block_len != cipher_descriptor[ocb->cipher].block_length) {
return CRYPT_INVALID_ARG;
}
if (ctlen % ocb->block_len) { 
return CRYPT_INVALID_ARG;
}
full_blocks = ctlen/ocb->block_len;
for(i=0; i<full_blocks; i++) {
pt_b = (unsigned char *)pt+i*ocb->block_len;
ct_b = (unsigned char *)ct+i*ocb->block_len;
ocb3_int_xor_blocks(ocb->Offset_current, ocb->Offset_current, ocb->L_[ocb3_int_ntz(ocb->block_index)], ocb->block_len);
ocb3_int_xor_blocks(tmp, ct_b, ocb->Offset_current, ocb->block_len);
if ((err = cipher_descriptor[ocb->cipher].ecb_decrypt(tmp, tmp, &ocb->key)) != CRYPT_OK) {
goto LBL_ERR;
}
ocb3_int_xor_blocks(pt_b, tmp, ocb->Offset_current, ocb->block_len);
ocb3_int_xor_blocks(ocb->checksum, ocb->checksum, pt_b, ocb->block_len);
ocb->block_index++;
}
err = CRYPT_OK;
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(tmp, sizeof(tmp));
#endif
return err;
}
#endif
