







#include "tomcrypt_private.h"














int ecc_ssh_ecdsa_encode_name(char *buffer, unsigned long *buflen, const ecc_key *key)
{
char oidstr[64];
unsigned long oidlen = sizeof(oidstr);
int err, size = 0;

LTC_ARGCHK(buffer != NULL);
LTC_ARGCHK(buflen != NULL);
LTC_ARGCHK(key != NULL);


if ((err = ecc_get_oid_str(oidstr, &oidlen, key)) != CRYPT_OK) goto error;


if (XSTRCMP("1.2.840.10045.3.1.7", oidstr) == 0) {

size = snprintf(buffer, *buflen, "ecdsa-sha2-nistp256");
}
else if (XSTRCMP("1.3.132.0.34", oidstr) == 0) {

size = snprintf(buffer, *buflen, "ecdsa-sha2-nistp384");
}
else if (XSTRCMP("1.3.132.0.35", oidstr) == 0) {

size = snprintf(buffer, *buflen, "ecdsa-sha2-nistp521");
} else {

size = snprintf(buffer, *buflen, "ecdsa-sha2-%s", oidstr);
}



if (size < 0) {
err = CRYPT_ERROR;
} else if ((unsigned)size >= *buflen) {
err = CRYPT_BUFFER_OVERFLOW;
} else {
err = CRYPT_OK;
}
*buflen = size + 1; 

error:
return err;
}





