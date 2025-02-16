







#include "tomcrypt_private.h"
#include <stdarg.h>






#if defined(LTC_SSH)








int ssh_encode_sequence_multi(unsigned char *out, unsigned long *outlen, ...)
{
int err;
va_list args;
ulong32 size;
ssh_data_type type;
void *vdata;
const char *sdata;
int idata;
ulong32 u32data;
ulong64 u64data;

LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);


size = 0;
va_start(args, outlen);
while ((type = (ssh_data_type)va_arg(args, int)) != LTC_SSHDATA_EOL) {
switch (type) {
case LTC_SSHDATA_BYTE:
case LTC_SSHDATA_BOOLEAN: 
LTC_UNUSED_PARAM( va_arg(args, int) );
size++;
break;
case LTC_SSHDATA_UINT32:
LTC_UNUSED_PARAM( va_arg(args, ulong32) );
size += 4;
break;
case LTC_SSHDATA_UINT64:
LTC_UNUSED_PARAM( va_arg(args, ulong64) );
size += 8;
break;
case LTC_SSHDATA_STRING:
case LTC_SSHDATA_NAMELIST:
LTC_UNUSED_PARAM( va_arg(args, char*) );
size += va_arg(args, unsigned long);
size += 4;
break;
case LTC_SSHDATA_MPINT:
vdata = va_arg(args, void*);

size += 4;
if (mp_iszero(vdata) != LTC_MP_YES) {
size += mp_unsigned_bin_size(vdata);
if ((mp_count_bits(vdata) & 7) == 0) size++; 
}
break;

case LTC_SSHDATA_EOL: 
err = CRYPT_INVALID_ARG;
goto error;
}
}
va_end(args);


if (*outlen < size) {
*outlen = size;
err = CRYPT_BUFFER_OVERFLOW;
goto errornoargs;
}
*outlen = size;


va_start(args, outlen);
while ((type = (ssh_data_type)va_arg(args, int)) != LTC_SSHDATA_EOL) {
switch (type) {
case LTC_SSHDATA_BYTE:
idata = va_arg(args, int);

*out++ = (unsigned char)(idata & 255);
break;
case LTC_SSHDATA_BOOLEAN:
idata = va_arg(args, int);





*out++ = (idata)?1:0;
break;
case LTC_SSHDATA_UINT32:
u32data = va_arg(args, ulong32);
STORE32H(u32data, out);
out += 4;
break;
case LTC_SSHDATA_UINT64:
u64data = va_arg(args, ulong64);
STORE64H(u64data, out);
out += 8;
break;
case LTC_SSHDATA_STRING:
case LTC_SSHDATA_NAMELIST:
sdata = va_arg(args, char*);
size = va_arg(args, unsigned long);
STORE32H(size, out);
out += 4;
XMEMCPY(out, sdata, size);
out += size;
break;
case LTC_SSHDATA_MPINT:
vdata = va_arg(args, void*);
if (mp_iszero(vdata) == LTC_MP_YES) {
STORE32H(0, out);
out += 4;
} else {
size = mp_unsigned_bin_size(vdata);
if ((mp_count_bits(vdata) & 7) == 0) {

STORE32H(size+1, out);
out += 4;
*out++ = 0;
} else {
STORE32H(size, out);
out += 4;
}
if ((err = mp_to_unsigned_bin(vdata, out)) != CRYPT_OK) {
err = CRYPT_ERROR;
goto error;
}
out += size;
}
break;

case LTC_SSHDATA_EOL: 
err = CRYPT_INVALID_ARG;
goto error;
}
}
err = CRYPT_OK;

error:
va_end(args);
errornoargs:
return err;
}

#endif




