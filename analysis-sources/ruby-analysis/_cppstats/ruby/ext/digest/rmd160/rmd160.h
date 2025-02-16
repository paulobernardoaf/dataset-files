#include "../defs.h"
typedef struct {
uint32_t state[5]; 
uint32_t length[2]; 
uint8_t bbuffer[64]; 
uint32_t buflen; 
} RMD160_CTX;
#if defined(RUBY)
#define RMD160_Init rb_Digest_RMD160_Init
#define RMD160_Transform rb_Digest_RMD160_Transform
#define RMD160_Update rb_Digest_RMD160_Update
#define RMD160_Finish rb_Digest_RMD160_Finish
#endif
__BEGIN_DECLS
int RMD160_Init _((RMD160_CTX *));
void RMD160_Transform _((uint32_t[5], const uint32_t[16]));
void RMD160_Update _((RMD160_CTX *, const uint8_t *, size_t));
int RMD160_Finish _((RMD160_CTX *, uint8_t[20]));
__END_DECLS
#define RMD160_BLOCK_LENGTH 64
#define RMD160_DIGEST_LENGTH 20
#define RMD160_DIGEST_STRING_LENGTH (RMD160_DIGEST_LENGTH * 2 + 1)
