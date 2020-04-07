#include <stddef.h>
#include <stdint.h>
#define TEXTURE_BLOCK_W 4
#define TEXTURE_BLOCK_H 4
typedef struct TextureDSPContext {
int (*dxt1_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*dxt1a_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*dxt2_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*dxt3_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*dxt4_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*dxt5_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*dxt5y_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*dxt5ys_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*rgtc1s_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*rgtc1u_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*rgtc1u_gray_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*rgtc1u_alpha_block)(uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*rgtc2s_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*rgtc2u_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*dxn3dc_block) (uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
} TextureDSPContext;
void ff_texturedsp_init(TextureDSPContext *c);
void ff_texturedspenc_init(TextureDSPContext *c);
