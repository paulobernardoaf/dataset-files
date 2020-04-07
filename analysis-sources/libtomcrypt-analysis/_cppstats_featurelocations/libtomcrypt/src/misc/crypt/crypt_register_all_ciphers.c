








#include "tomcrypt_private.h"







#define REGISTER_CIPHER(h) do {LTC_ARGCHK(register_cipher(h) != -1); } while(0)



int register_all_ciphers(void)
{
#if defined(LTC_RIJNDAEL)
#if defined(ENCRYPT_ONLY)



REGISTER_CIPHER(&aes_enc_desc);
#else



REGISTER_CIPHER(&aes_desc);
#endif
#endif
#if defined(LTC_BLOWFISH)
REGISTER_CIPHER(&blowfish_desc);
#endif
#if defined(LTC_XTEA)
REGISTER_CIPHER(&xtea_desc);
#endif
#if defined(LTC_RC5)
REGISTER_CIPHER(&rc5_desc);
#endif
#if defined(LTC_RC6)
REGISTER_CIPHER(&rc6_desc);
#endif
#if defined(LTC_SAFERP)
REGISTER_CIPHER(&saferp_desc);
#endif
#if defined(LTC_TWOFISH)
REGISTER_CIPHER(&twofish_desc);
#endif
#if defined(LTC_SAFER)
REGISTER_CIPHER(&safer_k64_desc);
REGISTER_CIPHER(&safer_sk64_desc);
REGISTER_CIPHER(&safer_k128_desc);
REGISTER_CIPHER(&safer_sk128_desc);
#endif
#if defined(LTC_RC2)
REGISTER_CIPHER(&rc2_desc);
#endif
#if defined(LTC_DES)
REGISTER_CIPHER(&des_desc);
REGISTER_CIPHER(&des3_desc);
#endif
#if defined(LTC_CAST5)
REGISTER_CIPHER(&cast5_desc);
#endif
#if defined(LTC_NOEKEON)
REGISTER_CIPHER(&noekeon_desc);
#endif
#if defined(LTC_SKIPJACK)
REGISTER_CIPHER(&skipjack_desc);
#endif
#if defined(LTC_ANUBIS)
REGISTER_CIPHER(&anubis_desc);
#endif
#if defined(LTC_KHAZAD)
REGISTER_CIPHER(&khazad_desc);
#endif
#if defined(LTC_KSEED)
REGISTER_CIPHER(&kseed_desc);
#endif
#if defined(LTC_KASUMI)
REGISTER_CIPHER(&kasumi_desc);
#endif
#if defined(LTC_MULTI2)
REGISTER_CIPHER(&multi2_desc);
#endif
#if defined(LTC_CAMELLIA)
REGISTER_CIPHER(&camellia_desc);
#endif
#if defined(LTC_IDEA)
REGISTER_CIPHER(&idea_desc);
#endif
#if defined(LTC_SERPENT)
REGISTER_CIPHER(&serpent_desc);
#endif
#if defined(LTC_TEA)
REGISTER_CIPHER(&tea_desc);
#endif
return CRYPT_OK;
}




