







#include "tomcrypt_private.h"
#include <stdarg.h>






#if defined(LTC_SSH)








int ssh_decode_sequence_multi(const unsigned char *in, unsigned long *inlen, ...)
{
int err;
va_list args;
ssh_data_type type;
void *vdata;
unsigned char *cdata;
char *sdata;
ulong32 *u32data;
ulong64 *u64data;
unsigned long *bufsize;
ulong32 size;
unsigned long remaining;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(inlen != NULL);

remaining = *inlen;

va_start(args, inlen);
while ((type = (ssh_data_type)va_arg(args, int)) != LTC_SSHDATA_EOL) {

if (type == LTC_SSHDATA_STRING ||
type == LTC_SSHDATA_NAMELIST ||
type == LTC_SSHDATA_MPINT)
{

if (remaining < 4) {
err = CRYPT_BUFFER_OVERFLOW;
goto error;
}
}


size = 0xFFFFFFFFU;
switch (type) {
case LTC_SSHDATA_BYTE:
case LTC_SSHDATA_BOOLEAN:
size = 1;
break;
case LTC_SSHDATA_UINT32:
size = 4;
break;
case LTC_SSHDATA_UINT64:
size = 8;
break;
case LTC_SSHDATA_STRING:
case LTC_SSHDATA_NAMELIST:
case LTC_SSHDATA_MPINT:
LOAD32H(size, in);
in += 4;
remaining -= 4;
break;

case LTC_SSHDATA_EOL:

err = CRYPT_INVALID_ARG;
goto error;
}


if (remaining < size) {
err = CRYPT_BUFFER_OVERFLOW;
goto error;
} else {
remaining -= size;
}

vdata = va_arg(args, void*);
if (vdata == NULL) {
err = CRYPT_INVALID_ARG;
goto error;
}


switch (type) {
case LTC_SSHDATA_BYTE:
cdata = vdata;
*cdata = *in++;
break;
case LTC_SSHDATA_BOOLEAN:
cdata = vdata;




*cdata = (*in++)?1:0;
break;
case LTC_SSHDATA_UINT32:
u32data = vdata;
LOAD32H(*u32data, in);
in += 4;
break;
case LTC_SSHDATA_UINT64:
u64data = vdata;
LOAD64H(*u64data, in);
in += 8;
break;
case LTC_SSHDATA_STRING:
case LTC_SSHDATA_NAMELIST:
sdata = vdata;
bufsize = va_arg(args, unsigned long*);
if (bufsize == NULL) {
err = CRYPT_INVALID_ARG;
goto error;
}
if (size + 1 >= *bufsize) {
err = CRYPT_BUFFER_OVERFLOW;
goto error;
}
if (size > 0) {
XMEMCPY(sdata, (const char *)in, size);
}
sdata[size] = '\0';
*bufsize = size;
in += size;
break;
case LTC_SSHDATA_MPINT:
if (size == 0) {
if ((err = mp_set(vdata, 0)) != CRYPT_OK) { goto error; }
} else if ((in[0] & 0x80) != 0) {

err = CRYPT_INVALID_PACKET;
goto error;
} else {
if ((err = mp_read_unsigned_bin(vdata, (unsigned char *)in, size)) != CRYPT_OK) { goto error; }
}
in += size;
break;

case LTC_SSHDATA_EOL:

err = CRYPT_INVALID_ARG;
goto error;
}
}
err = CRYPT_OK;

*inlen -= remaining;

error:
va_end(args);
return err;
}

#endif




