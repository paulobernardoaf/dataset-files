#include <tomcrypt.h>
int main(void)
{
register_cipher(&rijndael_enc_desc);
register_prng(&yarrow_desc);
register_hash(&sha256_desc);
return 0;
}
