







#include "tomcrypt_test.h"






#if defined(LTC_SSH)

#define BUFSIZE 64
























static const unsigned char byte1[] = {0x01};
static const unsigned char byte2[] = {0x71};
static const unsigned char uint32[] = {0x29, 0xb7, 0xf4, 0xaa};
static const unsigned char uint64[] = {0x09, 0xa3, 0x78, 0xf9, 0xb2, 0xe3, 0x32, 0xa7};
static const unsigned char string[] = {0x00, 0x00, 0x00, 0x07, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67};
static const unsigned char mpint1[] = {0x00, 0x00, 0x00, 0x00};
static const unsigned char mpint2[] = {0x00, 0x00, 0x00, 0x08, 0x09, 0xa3, 0x78, 0xf9, 0xb2, 0xe3, 0x32, 0xa7};
static const unsigned char mpint3[] = {0x00, 0x00, 0x00, 0x02, 0x00, 0x80};
static const unsigned char nlist1[] = {0x00, 0x00, 0x00, 0x00};
static const unsigned char nlist2[] = {0x00, 0x00, 0x00, 0x04, 0x7a, 0x6c, 0x69, 0x62};
static const unsigned char nlist3[] = {0x00, 0x00, 0x00, 0x09, 0x7a, 0x6c, 0x69, 0x62, 0x2c, 0x6e, 0x6f, 0x6e, 0x65};






static int _ssh_encoding_test(void)
{
unsigned char buffer[BUFSIZE];
unsigned long buflen;
unsigned long len;
void *v, *zero;
int err;


buflen = 3;
zeromem(buffer, BUFSIZE);
SHOULD_FAIL(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_UINT32, 0x29b7f4aa,
LTC_SSHDATA_EOL, NULL));



buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_BYTE, 0x01,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, byte1, sizeof(byte1), "enc-byte", 1);

buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_BYTE, 0x71,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, byte2, sizeof(byte2), "enc-byte", 2);
if (XMEMCMP(buffer, byte2, buflen) != 0) return CRYPT_FAIL_TESTVECTOR;


buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_BOOLEAN, 0x01,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, byte1, sizeof(byte1), "enc-boolean", 1);

buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_BOOLEAN, 0x71,
LTC_SSHDATA_EOL, NULL));

COMPARE_TESTVECTOR(buffer, buflen, byte1, sizeof(byte1), "enc-boolean", 2);


buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_UINT32, 0x29b7f4aa,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, uint32, sizeof(uint32), "enc-uint32", 1);


buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_UINT64, CONST64(0x09a378f9b2e332a7),
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, uint64, sizeof(uint64), "enc-uint64", 1);



buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
len = strlen("testing");
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_STRING, "testing", len,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, string, sizeof(string), "enc-string", 1);



if ((err = mp_init_multi(&zero, &v, NULL)) != CRYPT_OK) {
return err;
}

buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
DO(mp_set(zero, 0));
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_MPINT, zero,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, mpint1, sizeof(mpint1), "enc-mpint", 1);

buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
DO(mp_read_radix(v, "9a378f9b2e332a7", 16));
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_MPINT, v,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, mpint2, sizeof(mpint2), "enc-mpint", 2);

buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
DO(mp_set(v, 0x80));
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_MPINT, v,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, mpint3, sizeof(mpint3), "enc-mpint", 3);

mp_clear_multi(v, zero, NULL);



buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
len = strlen("");
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_NAMELIST, "", len,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, nlist1, sizeof(nlist1), "enc-nlist", 1);

buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
len = strlen("zlib");
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_NAMELIST, "zlib", len,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, nlist2, sizeof(nlist2), "enc-nlist", 2);

buflen = BUFSIZE;
zeromem(buffer, BUFSIZE);
len = strlen("zlib,none");
DO(ssh_encode_sequence_multi(buffer, &buflen,
LTC_SSHDATA_NAMELIST, "zlib,none", len,
LTC_SSHDATA_EOL, NULL));
COMPARE_TESTVECTOR(buffer, buflen, nlist3, sizeof(nlist3), "enc-nlist", 3);

return CRYPT_OK;
}





static int _ssh_decoding_test(void)
{
char strbuf[BUFSIZE];
void *u, *v;
unsigned long size;
ulong32 tmp32;
ulong64 tmp64;
unsigned char tmp8;
unsigned long len;
int err;


len = sizeof(strbuf);
strbuf[0] = 0;
DO(ssh_decode_sequence_multi((unsigned char*)strbuf, &len,
LTC_SSHDATA_BYTE, &tmp8,
LTC_SSHDATA_EOL, NULL));
ENSURE(tmp8 == 0x00);
ENSURE(len == 1);



len = sizeof(byte1);
DO(ssh_decode_sequence_multi(byte1, &len,
LTC_SSHDATA_BYTE, &tmp8,
LTC_SSHDATA_EOL, NULL));
ENSURE(tmp8 == 0x01);
ENSURE(len == 1);

len = sizeof(byte2);
DO(ssh_decode_sequence_multi(byte2, &len,
LTC_SSHDATA_BYTE, &tmp8,
LTC_SSHDATA_EOL, NULL));
ENSURE(tmp8 == 0x71);
ENSURE(len == 1);


len = sizeof(byte1);
DO(ssh_decode_sequence_multi(byte1, &len,
LTC_SSHDATA_BOOLEAN, &tmp8,
LTC_SSHDATA_EOL, NULL));
ENSURE(tmp8 == 0x01);
ENSURE(len == 1);

len = sizeof(byte2);
DO(ssh_decode_sequence_multi(byte2, &len,
LTC_SSHDATA_BOOLEAN, &tmp8,
LTC_SSHDATA_EOL, NULL));
ENSURE(tmp8 == 0x01);
ENSURE(len == 1);


len = sizeof(uint32);
DO(ssh_decode_sequence_multi(uint32, &len,
LTC_SSHDATA_UINT32, &tmp32,
LTC_SSHDATA_EOL, NULL));
ENSURE(tmp32 == 0x29b7f4aa);
ENSURE(len == 4);


len = sizeof(uint64);
DO(ssh_decode_sequence_multi(uint64, &len,
LTC_SSHDATA_UINT64, &tmp64,
LTC_SSHDATA_EOL, NULL));
if (tmp64 != CONST64(0x09a378f9b2e332a7)) return CRYPT_FAIL_TESTVECTOR;
ENSURE(len == 8);


zeromem(strbuf, BUFSIZE);
size = BUFSIZE;
len = sizeof(string);
DO(ssh_decode_sequence_multi(string, &len,
LTC_SSHDATA_STRING, strbuf, &size,
LTC_SSHDATA_EOL, NULL));
ENSURE(strlen("testing") == size);
ENSURE(XSTRCMP(strbuf, "testing") == 0);
ENSURE(strlen("testing") + 4 == len);


if ((err = mp_init_multi(&u, &v, NULL)) != CRYPT_OK) {
return err;
}

len = sizeof(mpint1);
DO(ssh_decode_sequence_multi(mpint1, &len,
LTC_SSHDATA_MPINT, v,
LTC_SSHDATA_EOL, NULL));
ENSURE(mp_cmp_d(v, 0) == LTC_MP_EQ);
ENSURE(sizeof(mpint1) == len);

len = sizeof(mpint2);
DO(ssh_decode_sequence_multi(mpint2, &len,
LTC_SSHDATA_MPINT, v,
LTC_SSHDATA_EOL, NULL));
DO(mp_read_radix(u, "9a378f9b2e332a7", 16));
ENSURE(mp_cmp(u, v) == LTC_MP_EQ);
ENSURE(sizeof(mpint2) == len);

len = sizeof(mpint3);
DO(ssh_decode_sequence_multi(mpint3, &len,
LTC_SSHDATA_MPINT, v,
LTC_SSHDATA_EOL, NULL));
ENSURE(mp_cmp_d(v, 0x80) == LTC_MP_EQ);
ENSURE(sizeof(mpint3) == len);

mp_clear_multi(v, u, NULL);


zeromem(strbuf, BUFSIZE);
size = BUFSIZE;
len = sizeof(nlist1);
DO(ssh_decode_sequence_multi(nlist1, &len,
LTC_SSHDATA_NAMELIST, strbuf, &size,
LTC_SSHDATA_EOL, NULL));
ENSURE(strlen("") == size);
ENSURE(XSTRCMP(strbuf, "") == 0);

zeromem(strbuf, BUFSIZE);
size = BUFSIZE;
len = sizeof(nlist2);
DO(ssh_decode_sequence_multi(nlist2, &len,
LTC_SSHDATA_NAMELIST, strbuf, &size,
LTC_SSHDATA_EOL, NULL));
ENSURE(strlen("zlib") == size);
ENSURE(XSTRCMP(strbuf, "zlib") == 0);
ENSURE(strlen("zlib") + 4 == len);

zeromem(strbuf, BUFSIZE);
size = BUFSIZE;
len = sizeof(nlist3);
DO(ssh_decode_sequence_multi(nlist3, &len,
LTC_SSHDATA_NAMELIST, strbuf, &size,
LTC_SSHDATA_EOL, NULL));
ENSURE(strlen("zlib,none") == size);
ENSURE(XSTRCMP(strbuf, "zlib,none") == 0);
ENSURE(strlen("zlib,none") + 4 == len);


return CRYPT_OK;
}





int ssh_test(void)
{
if (ltc_mp.name == NULL) return CRYPT_NOP;

DO(_ssh_encoding_test());
DO(_ssh_decoding_test());

return CRYPT_OK;
}

#else

int ssh_test(void)
{
return CRYPT_NOP;
}

#endif





