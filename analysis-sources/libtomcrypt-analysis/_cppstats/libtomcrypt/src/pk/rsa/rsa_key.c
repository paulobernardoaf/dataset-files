#include "tomcrypt_private.h"
#if defined(LTC_MRSA)
#include <stdarg.h>
static void _mpi_shrink_multi(void **a, ...)
{
void **cur;
unsigned n;
int err;
va_list args;
void *tmp[10] = { 0 };
void **arg[10] = { 0 };
n = 0;
err = CRYPT_ERROR;
cur = a;
va_start(args, a);
while (cur != NULL) {
if (n >= sizeof(tmp)/sizeof(tmp[0])) {
goto out;
}
if (*cur != NULL) {
arg[n] = cur;
if ((err = mp_init_copy(&tmp[n], *arg[n])) != CRYPT_OK) {
goto out;
}
n++;
}
cur = va_arg(args, void**);
}
va_end(args);
while (n != 0 && arg[--n] != NULL) {
mp_clear(*arg[n]);
*arg[n] = tmp[n];
}
out:
va_end(args);
if ((err != CRYPT_OK) ||
(n >= sizeof(tmp)/sizeof(tmp[0]))) {
for (n = 0; n < sizeof(tmp)/sizeof(tmp[0]); ++n) {
if (tmp[n] != NULL) {
mp_clear(tmp[n]);
}
}
}
}
void rsa_shrink_key(rsa_key *key)
{
LTC_ARGCHKVD(key != NULL);
_mpi_shrink_multi(&key->e, &key->d, &key->N, &key->dQ, &key->dP, &key->qP, &key->p, &key->q, NULL);
}
int rsa_init(rsa_key *key)
{
LTC_ARGCHK(key != NULL);
return mp_init_multi(&key->e, &key->d, &key->N, &key->dQ, &key->dP, &key->qP, &key->p, &key->q, NULL);
}
void rsa_free(rsa_key *key)
{
LTC_ARGCHKVD(key != NULL);
mp_cleanup_multi(&key->q, &key->p, &key->qP, &key->dP, &key->dQ, &key->N, &key->d, &key->e, NULL);
}
#endif
