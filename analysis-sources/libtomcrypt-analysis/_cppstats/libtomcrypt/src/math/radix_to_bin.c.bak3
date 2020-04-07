







#include "tomcrypt_private.h"
























int radix_to_bin(const void *in, int radix, void *out, unsigned long *len)
{
unsigned long l;
void* mpi;
int err;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(len != NULL);

if ((err = mp_init(&mpi)) != CRYPT_OK) return err;
if ((err = mp_read_radix(mpi, in, radix)) != CRYPT_OK) goto LBL_ERR;

if ((l = mp_unsigned_bin_size(mpi)) > *len) {
*len = l;
err = CRYPT_BUFFER_OVERFLOW;
goto LBL_ERR;
}
*len = l;

if ((err = mp_to_unsigned_bin(mpi, out)) != CRYPT_OK) goto LBL_ERR;

LBL_ERR:
mp_clear(mpi);
return err;
}




