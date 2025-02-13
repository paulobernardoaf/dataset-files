




















#if !defined(AVFORMAT_RTMPDH_H)
#define AVFORMAT_RTMPDH_H

#include <stdint.h>

#include "config.h"

#if CONFIG_GMP
#include <gmp.h>

typedef mpz_ptr FFBigNum;
#elif CONFIG_GCRYPT
#include <gcrypt.h>

typedef gcry_mpi_t FFBigNum;

#elif CONFIG_OPENSSL
#include <openssl/bn.h>
#include <openssl/dh.h>

typedef BIGNUM *FFBigNum;
#elif CONFIG_MBEDTLS
#include <mbedtls/bignum.h>

typedef mbedtls_mpi *FFBigNum;

#endif

typedef struct FF_DH {
FFBigNum p;
FFBigNum g;
FFBigNum pub_key;
FFBigNum priv_key;
long length;
} FF_DH;








FF_DH *ff_dh_init(int key_len);






void ff_dh_free(FF_DH *dh);







int ff_dh_generate_public_key(FF_DH *dh);









int ff_dh_write_public_key(FF_DH *dh, uint8_t *pub_key, int pub_key_len);












int ff_dh_compute_shared_secret_key(FF_DH *dh, const uint8_t *pub_key,
int pub_key_len, uint8_t *secret_key,
int secret_key_len);

#endif 
