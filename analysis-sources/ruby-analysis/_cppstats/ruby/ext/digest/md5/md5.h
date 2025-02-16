#include "../defs.h"
typedef struct md5_state_s {
uint32_t count[2]; 
uint32_t state[4]; 
uint8_t buffer[64]; 
} MD5_CTX;
#if defined(RUBY)
#define MD5_Init rb_Digest_MD5_Init
#define MD5_Update rb_Digest_MD5_Update
#define MD5_Finish rb_Digest_MD5_Finish
#endif
int MD5_Init _((MD5_CTX *pms));
void MD5_Update _((MD5_CTX *pms, const uint8_t *data, size_t nbytes));
int MD5_Finish _((MD5_CTX *pms, uint8_t *digest));
#define MD5_BLOCK_LENGTH 64
#define MD5_DIGEST_LENGTH 16
#define MD5_DIGEST_STRING_LENGTH (MD5_DIGEST_LENGTH * 2 + 1)
