












#include "tomcrypt_private.h"

#if defined(LTC_OCB3_MODE)

static void _ocb3_int_calc_offset_zero(ocb3_state *ocb, const unsigned char *nonce, unsigned long noncelen, unsigned long taglen)
{
int x, y, bottom;
int idx, shift;
unsigned char iNonce[MAXBLOCKSIZE];
unsigned char iKtop[MAXBLOCKSIZE];
unsigned char iStretch[MAXBLOCKSIZE+8];


zeromem(iNonce, sizeof(iNonce));
for (x = ocb->block_len-1, y=0; y<(int)noncelen; x--, y++) {
iNonce[x] = nonce[noncelen-y-1];
}
iNonce[x] = 0x01;
iNonce[0] |= ((taglen*8) % 128) << 1;


bottom = iNonce[ocb->block_len-1] & 0x3F;


iNonce[ocb->block_len-1] = iNonce[ocb->block_len-1] & 0xC0;
if ((cipher_descriptor[ocb->cipher].ecb_encrypt(iNonce, iKtop, &ocb->key)) != CRYPT_OK) {
zeromem(ocb->Offset_current, ocb->block_len);
return;
}


for (x = 0; x < ocb->block_len; x++) {
iStretch[x] = iKtop[x];
}
for (y = 0; y < 8; y++) {
iStretch[x+y] = iKtop[y] ^ iKtop[y+1];
}


idx = bottom / 8;
shift = (bottom % 8);
for (x = 0; x < ocb->block_len; x++) {
ocb->Offset_current[x] = iStretch[idx+x] << shift;
if (shift > 0) {
ocb->Offset_current[x] |= iStretch[idx+x+1] >> (8-shift);
}
}
}

static const struct {
int len;
unsigned char poly_mul[MAXBLOCKSIZE];
} polys[] = {
{
8,
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B }
}, {
16,
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87 }
}
};












int ocb3_init(ocb3_state *ocb, int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
unsigned long taglen)
{
int poly, x, y, m, err;
unsigned char *previous, *current;

LTC_ARGCHK(ocb != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(nonce != NULL);


if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
return err;
}
ocb->cipher = cipher;



if (noncelen > (120/8)) {
return CRYPT_INVALID_ARG;
}


if (cipher_descriptor[cipher].block_length != 16) {
return CRYPT_INVALID_ARG;
}


if (taglen > 16) {
return CRYPT_INVALID_ARG;
}
ocb->tag_len = taglen;


ocb->block_len = cipher_descriptor[cipher].block_length;
x = (int)(sizeof(polys)/sizeof(polys[0]));
for (poly = 0; poly < x; poly++) {
if (polys[poly].len == ocb->block_len) {
break;
}
}
if (poly == x) {
return CRYPT_INVALID_ARG; 
}
if (polys[poly].len != ocb->block_len) {
return CRYPT_INVALID_ARG;
}


if ((err = cipher_descriptor[cipher].setup(key, keylen, 0, &ocb->key)) != CRYPT_OK) {
return err;
}


zeromem(ocb->L_star, ocb->block_len);
if ((err = cipher_descriptor[cipher].ecb_encrypt(ocb->L_star, ocb->L_star, &ocb->key)) != CRYPT_OK) {
return err;
}


for (x = -1; x < 32; x++) {
if (x == -1) { 
current = ocb->L_dollar;
previous = ocb->L_star;
}
else if (x == 0) { 
current = ocb->L_[0];
previous = ocb->L_dollar;
}
else { 
current = ocb->L_[x];
previous = ocb->L_[x-1];
}
m = previous[0] >> 7;
for (y = 0; y < ocb->block_len-1; y++) {
current[y] = ((previous[y] << 1) | (previous[y+1] >> 7)) & 255;
}
current[ocb->block_len-1] = (previous[ocb->block_len-1] << 1) & 255;
if (m == 1) {

ocb3_int_xor_blocks(current, current, polys[poly].poly_mul, ocb->block_len);
}
}


_ocb3_int_calc_offset_zero(ocb, nonce, noncelen, taglen);


zeromem(ocb->checksum, ocb->block_len);


ocb->block_index = 1;


ocb->ablock_index = 1;
ocb->adata_buffer_bytes = 0;
zeromem(ocb->aOffset_current, ocb->block_len);
zeromem(ocb->aSum_current, ocb->block_len);

return CRYPT_OK;
}

#endif




