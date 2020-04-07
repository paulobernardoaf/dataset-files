#include <tomcrypt.h>
#if !defined(LTC_RIJNDAEL)
#error Cannot compile this demo; Rijndael (AES) required
#endif
#if !defined(LTC_CBC_MODE)
#error Cannot compile this demo; CBC mode required
#endif
#if !defined(LTC_PKCS_5)
#error Cannot compile this demo; PKCS5 required
#endif
#if !defined(LTC_RNG_GET_BYTES)
#error Cannot compile this demo; random generator required
#endif
#if !defined(LTC_MD5)
#error Cannot compile this demo; MD5 required
#endif
#define OPENSSL_ITERATIONS 1
#define KEY_LENGTH (256>>3)
#define IV_LENGTH (128>>3)
#define SALT_LENGTH 8
static char salt_header[] = { 'S', 'a', 'l', 't', 'e', 'd', '_', '_' };
#include <errno.h>
#include <stdio.h>
#include <string.h>
union paddable {
unsigned char unpad[1024];
unsigned char pad[1024+MAXBLOCKSIZE];
};
void barf(const char *pname, const char *err)
{
printf("Usage: %s <enc|dec> infile outfile passphrase [salt]\n", pname);
printf("\n");
printf(" #encrypts infile->outfile, random salt\n");
printf(" %s enc infile outfile \"passphrase\"\n", pname);
printf("\n");
printf(" #encrypts infile->outfile, salt from cmdline\n");
printf(" %s enc infile outfile pass 0123456789abcdef\n", pname);
printf("\n");
printf(" #decrypts infile->outfile, pulls salt from infile\n");
printf(" %s dec infile outfile pass\n", pname);
printf("\n");
printf(" #decrypts infile->outfile, salt specified\n");
printf(" #(don't try to read the salt from infile)\n");
printf(" %s dec infile outfile pass 0123456789abcdef"
"\n", pname);
printf("\n");
printf("Application Error: %s\n", err);
if(errno)
perror(" System Error");
exit(-1);
}
int parse_hex_salt(unsigned char *in, unsigned char *out)
{
int idx;
for(idx=0; idx<SALT_LENGTH; idx++)
if(sscanf((char*)in+idx*2, "%02hhx", out+idx) != 1)
return CRYPT_ERROR;
return CRYPT_OK;
}
int parse_openssl_header(FILE *in, unsigned char *out)
{
unsigned char tmp[SALT_LENGTH];
if(fread(tmp, 1, sizeof(tmp), in) != sizeof(tmp))
return CRYPT_ERROR;
if(memcmp(tmp, salt_header, sizeof(tmp)))
return CRYPT_ERROR;
if(fread(tmp, 1, sizeof(tmp), in) != sizeof(tmp))
return CRYPT_ERROR;
memcpy(out, tmp, sizeof(tmp));
return CRYPT_OK;
}
void dump_bytes(unsigned char *in, unsigned long len)
{
unsigned long idx;
for(idx=0; idx<len; idx++)
printf("%02hhX", *(in+idx));
}
static size_t _pkcs7_pad(union paddable *buf, size_t nb, int block_length,
int is_padding)
{
unsigned long length;
if(is_padding) {
length = sizeof(buf->pad);
if (padding_pad(buf->pad, nb, &length, block_length) != CRYPT_OK)
return 0;
return length;
} else {
length = nb;
if (padding_depad(buf->pad, &length, 0) != CRYPT_OK)
return 0;
return length;
}
}
int do_crypt(FILE *infd, FILE *outfd, unsigned char *key, unsigned char *iv,
int encrypt)
{
union paddable inbuf, outbuf;
int cipher, ret;
symmetric_CBC cbc;
size_t nb;
cipher = register_cipher(&aes_desc);
if(cipher == -1)
return CRYPT_INVALID_CIPHER;
ret = cbc_start(cipher, iv, key, KEY_LENGTH, 0, &cbc);
if( ret != CRYPT_OK )
return -1;
do {
nb = fread(inbuf.unpad, 1, sizeof(inbuf.unpad), infd);
if(!nb)
return encrypt ? CRYPT_OK : CRYPT_ERROR;
if(ferror(infd))
return CRYPT_ERROR;
if(encrypt) {
if(feof(infd))
nb = _pkcs7_pad(&inbuf, nb,
aes_desc.block_length, 1);
ret = cbc_encrypt(inbuf.pad, outbuf.pad, nb, &cbc);
if(ret != CRYPT_OK)
return ret;
} else {
ret = cbc_decrypt(inbuf.unpad, outbuf.unpad, nb, &cbc);
if( ret != CRYPT_OK )
return ret;
if(feof(infd))
nb = _pkcs7_pad(&outbuf, nb,
aes_desc.block_length, 0);
if(nb == 0)
return CRYPT_ERROR;
}
if(fwrite(outbuf.unpad, 1, nb, outfd) != nb)
return CRYPT_ERROR;
} while(!feof(infd));
cbc_done(&cbc);
return CRYPT_OK;
}
#define BARF(a) { if(infd) fclose(infd); if(outfd) { fclose(outfd); remove(argv[3]); } barf(argv[0], a); }
int main(int argc, char *argv[]) {
unsigned char salt[SALT_LENGTH];
FILE *infd = NULL, *outfd = NULL;
int encrypt = -1;
int hash = -1;
int ret;
unsigned char keyiv[KEY_LENGTH + IV_LENGTH];
unsigned long keyivlen = (KEY_LENGTH + IV_LENGTH);
unsigned char *key, *iv;
if(argc < 5 || argc > 6)
BARF("Invalid number of arguments");
if (!strncmp(argv[1], "enc", 3))
encrypt = 1;
else if(!strncmp(argv[1], "dec", 3))
encrypt = 0;
else
BARF("Bad command name");
infd = fopen(argv[2], "rb");
if(infd == NULL)
BARF("Could not open infile");
outfd = fopen(argv[3], "wb");
if(outfd == NULL)
BARF("Could not open outfile");
if(argc == 6) {
if(parse_hex_salt((unsigned char*) argv[5], salt) != CRYPT_OK)
BARF("Bad user-specified salt");
} else if(!strncmp(argv[1], "enc", 3)) {
if(rng_get_bytes(salt, sizeof(salt), NULL) != sizeof(salt))
BARF("Not enough random data");
} else {
if(parse_openssl_header(infd, salt) != CRYPT_OK)
BARF("Invalid OpenSSL header in infile");
}
hash = register_hash(&md5_desc);
if(hash == -1)
BARF("Could not register MD5 hash");
zeromem(keyiv, sizeof(keyiv));
key = keyiv + 0; 
iv = keyiv + KEY_LENGTH; 
ret = pkcs_5_alg1_openssl((unsigned char*)argv[4], XSTRLEN(argv[4]), salt,
OPENSSL_ITERATIONS, hash, keyiv, &keyivlen );
if(ret != CRYPT_OK)
BARF("Could not derive key/iv from passphrase");
printf("salt="); dump_bytes(salt, sizeof(salt)); printf("\n");
printf("key="); dump_bytes(key, KEY_LENGTH); printf("\n");
printf("iv ="); dump_bytes(iv, IV_LENGTH ); printf("\n");
if(!strncmp(argv[1], "enc", 3)) {
if(fwrite(salt_header, 1, sizeof(salt_header), outfd) !=
sizeof(salt_header) )
BARF("Error writing salt header to outfile");
if(fwrite(salt, 1, sizeof(salt), outfd) != sizeof(salt))
BARF("Error writing salt to outfile");
}
if(do_crypt(infd, outfd, key, iv, encrypt) != CRYPT_OK)
BARF("Error during crypt operation");
fclose(infd); fclose(outfd);
return 0;
}
