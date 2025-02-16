#include "tomcrypt_private.h"
#if defined(LTC_OCB3_MODE)
int ocb3_decrypt_last(ocb3_state *ocb, const unsigned char *ct, unsigned long ctlen, unsigned char *pt)
{
unsigned char iOffset_star[MAXBLOCKSIZE];
unsigned char iPad[MAXBLOCKSIZE];
int err, x, full_blocks, full_blocks_len, last_block_len;
LTC_ARGCHK(ocb != NULL);
if (ct == NULL) LTC_ARGCHK(ctlen == 0);
if (ctlen != 0) {
LTC_ARGCHK(ct != NULL);
LTC_ARGCHK(pt != NULL);
}
if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
goto LBL_ERR;
}
full_blocks = ctlen/ocb->block_len;
full_blocks_len = full_blocks * ocb->block_len;
last_block_len = ctlen - full_blocks_len;
if (full_blocks>0) {
if ((err = ocb3_decrypt(ocb, ct, full_blocks_len, pt)) != CRYPT_OK) {
goto LBL_ERR;
}
}
if (last_block_len>0) {
ocb3_int_xor_blocks(iOffset_star, ocb->Offset_current, ocb->L_star, ocb->block_len);
if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(iOffset_star, iPad, &ocb->key)) != CRYPT_OK) {
goto LBL_ERR;
}
ocb3_int_xor_blocks(pt+full_blocks_len, (unsigned char *)ct+full_blocks_len, iPad, last_block_len);
ocb3_int_xor_blocks(ocb->checksum, ocb->checksum, pt+full_blocks_len, last_block_len);
for(x=last_block_len; x<ocb->block_len; x++) {
if (x == last_block_len) {
ocb->checksum[x] ^= 0x80;
} else {
ocb->checksum[x] ^= 0x00;
}
}
for(x=0; x<ocb->block_len; x++) {
ocb->tag_part[x] = (ocb->checksum[x] ^ iOffset_star[x]) ^ ocb->L_dollar[x];
}
if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(ocb->tag_part, ocb->tag_part, &ocb->key)) != CRYPT_OK) {
goto LBL_ERR;
}
}
else {
for(x=0; x<ocb->block_len; x++) {
ocb->tag_part[x] = (ocb->checksum[x] ^ ocb->Offset_current[x]) ^ ocb->L_dollar[x];
}
if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(ocb->tag_part, ocb->tag_part, &ocb->key)) != CRYPT_OK) {
goto LBL_ERR;
}
}
err = CRYPT_OK;
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(iOffset_star, MAXBLOCKSIZE);
zeromem(iPad, MAXBLOCKSIZE);
#endif
return err;
}
#endif
