#include <stdint.h>
#include "attributes.h"
#include "version.h"
extern const int av_ripemd_size;
struct AVRIPEMD;
struct AVRIPEMD *av_ripemd_alloc(void);
int av_ripemd_init(struct AVRIPEMD* context, int bits);
#if FF_API_CRYPTO_SIZE_T
void av_ripemd_update(struct AVRIPEMD* context, const uint8_t* data, unsigned int len);
#else
void av_ripemd_update(struct AVRIPEMD* context, const uint8_t* data, size_t len);
#endif
void av_ripemd_final(struct AVRIPEMD* context, uint8_t *digest);
