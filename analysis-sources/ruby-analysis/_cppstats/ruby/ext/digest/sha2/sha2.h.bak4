



































#if !defined(__SHA2_H__)
#define __SHA2_H__

#if defined(__cplusplus)
extern "C" {
#endif







#include <sys/types.h>

#if defined(RUBY)
#if defined(HAVE_PROTOTYPES)
#undef NOPROTO
#else
#define NOPROTO
#endif 
#if !defined(BYTE_ORDER)
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#if defined(WORDS_BIGENDIAN)
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif 
#define SHA2_USE_INTTYPES_H
#else 
#if defined(SHA2_USE_INTTYPES_H)

#include <inttypes.h>

#endif 
#endif 



#define SHA256_BLOCK_LENGTH 64
#define SHA256_DIGEST_LENGTH 32
#define SHA256_DIGEST_STRING_LENGTH (SHA256_DIGEST_LENGTH * 2 + 1)
#define SHA384_BLOCK_LENGTH 128
#define SHA384_DIGEST_LENGTH 48
#define SHA384_DIGEST_STRING_LENGTH (SHA384_DIGEST_LENGTH * 2 + 1)
#define SHA512_BLOCK_LENGTH 128
#define SHA512_DIGEST_LENGTH 64
#define SHA512_DIGEST_STRING_LENGTH (SHA512_DIGEST_LENGTH * 2 + 1)







#if !defined(SHA2_USE_INTTYPES_H)
#if defined(HAVE_U_INT8_T)
typedef u_int8_t uint8_t; 
typedef u_int32_t uint32_t; 
typedef u_int64_t uint64_t; 
#else
typedef unsigned char uint8_t; 
typedef unsigned int uint32_t; 
typedef unsigned long long uint64_t; 
#endif
#endif

















typedef struct _SHA256_CTX {
uint32_t state[8];
uint64_t bitcount;
uint8_t buffer[SHA256_BLOCK_LENGTH];
} SHA256_CTX;
typedef struct _SHA512_CTX {
uint64_t state[8];
uint64_t bitcount[2];
uint8_t buffer[SHA512_BLOCK_LENGTH];
} SHA512_CTX;

typedef SHA512_CTX SHA384_CTX;



#if defined(RUBY)
#define SHA256_Init rb_Digest_SHA256_Init
#define SHA256_Update rb_Digest_SHA256_Update
#define SHA256_Finish rb_Digest_SHA256_Finish
#define SHA256_Data rb_Digest_SHA256_Data
#define SHA256_End rb_Digest_SHA256_End
#define SHA256_Last rb_Digest_SHA256_Last
#define SHA256_Transform rb_Digest_SHA256_Transform
#define SHA256_Final(d, c) SHA256_Finish(c, d)

#define SHA384_Init rb_Digest_SHA384_Init
#define SHA384_Update rb_Digest_SHA384_Update
#define SHA384_Finish rb_Digest_SHA384_Finish
#define SHA384_Data rb_Digest_SHA384_Data
#define SHA384_End rb_Digest_SHA384_End
#define SHA384_Last rb_Digest_SHA384_Last
#define SHA384_Transform rb_Digest_SHA384_Transform
#define SHA384_Final(d, c) SHA384_Finish(c, d)

#define SHA512_Init rb_Digest_SHA512_Init
#define SHA512_Update rb_Digest_SHA512_Update
#define SHA512_Finish rb_Digest_SHA512_Finish
#define SHA512_Data rb_Digest_SHA512_Data
#define SHA512_End rb_Digest_SHA512_End
#define SHA512_Last rb_Digest_SHA512_Last
#define SHA512_Transform rb_Digest_SHA512_Transform
#define SHA512_Final(d, c) SHA512_Finish(c, d)
#endif 

#if !defined(NOPROTO)

int SHA256_Init(SHA256_CTX *);
void SHA256_Update(SHA256_CTX*, const uint8_t*, size_t);
int SHA256_Final(uint8_t[SHA256_DIGEST_LENGTH], SHA256_CTX*);
char* SHA256_End(SHA256_CTX*, char[SHA256_DIGEST_STRING_LENGTH]);
char* SHA256_Data(const uint8_t*, size_t, char[SHA256_DIGEST_STRING_LENGTH]);

int SHA384_Init(SHA384_CTX*);
void SHA384_Update(SHA384_CTX*, const uint8_t*, size_t);
int SHA384_Final(uint8_t[SHA384_DIGEST_LENGTH], SHA384_CTX*);
char* SHA384_End(SHA384_CTX*, char[SHA384_DIGEST_STRING_LENGTH]);
char* SHA384_Data(const uint8_t*, size_t, char[SHA384_DIGEST_STRING_LENGTH]);

int SHA512_Init(SHA512_CTX*);
void SHA512_Update(SHA512_CTX*, const uint8_t*, size_t);
int SHA512_Final(uint8_t[SHA512_DIGEST_LENGTH], SHA512_CTX*);
char* SHA512_End(SHA512_CTX*, char[SHA512_DIGEST_STRING_LENGTH]);
char* SHA512_Data(const uint8_t*, size_t, char[SHA512_DIGEST_STRING_LENGTH]);

#else 

int SHA256_Init();
void SHA256_Update();
#if defined(RUBY)
int SHA256_Finish();
#else
int SHA256_Final();
#endif 
char* SHA256_End();
char* SHA256_Data();

int SHA384_Init();
void SHA384_Update();
#if defined(RUBY)
int SHA384_Finish();
#else
int SHA384_Final();
#endif 
char* SHA384_End();
char* SHA384_Data();

int SHA512_Init();
void SHA512_Update();
#if defined(RUBY)
int SHA512_Finish();
#else
int SHA512_Final();
#endif 
char* SHA512_End();
char* SHA512_Data();

#endif 

#if defined(__cplusplus)
}
#endif 

#endif 

