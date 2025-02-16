#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_picture.h>
#include <string.h>
#include <stdint.h>
#include "qcom.h"
#define TILE_WIDTH 64
#define TILE_HEIGHT 32
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)
static size_t tile_pos(size_t x, size_t y, size_t w, size_t h)
{
size_t flim = x + (y & ~1) * w;
if (y & 1) {
flim += (x & ~3) + 2;
} else if ((h & 1) == 0 || y != (h - 1)) {
flim += (x + 2) & ~3;
}
return flim;
}
void qcom_convert(const uint8_t *src, picture_t *pic)
{
size_t width = pic->format.i_width;
size_t pitch = pic->p[0].i_pitch;
size_t height = pic->format.i_height;
const size_t tile_w = (width - 1) / TILE_WIDTH + 1;
const size_t tile_w_align = (tile_w + 1) & ~1;
const size_t tile_h_luma = (height - 1) / TILE_HEIGHT + 1;
const size_t tile_h_chroma = (height / 2 - 1) / TILE_HEIGHT + 1;
size_t luma_size = tile_w_align * tile_h_luma * TILE_SIZE;
#define TILE_GROUP_SIZE (4 * TILE_SIZE)
if((luma_size % TILE_GROUP_SIZE) != 0)
luma_size = (((luma_size - 1) / TILE_GROUP_SIZE) + 1) * TILE_GROUP_SIZE;
for(size_t y = 0; y < tile_h_luma; y++) {
size_t row_width = width;
for(size_t x = 0; x < tile_w; x++) {
const uint8_t *src_luma = src
+ tile_pos(x, y,tile_w_align, tile_h_luma) * TILE_SIZE;
const uint8_t *src_chroma = src + luma_size
+ tile_pos(x, y/2, tile_w_align, tile_h_chroma) * TILE_SIZE;
if (y & 1)
src_chroma += TILE_SIZE/2;
size_t tile_width = row_width;
if (tile_width > TILE_WIDTH)
tile_width = TILE_WIDTH;
size_t tile_height = height;
if (tile_height > TILE_HEIGHT)
tile_height = TILE_HEIGHT;
size_t luma_idx = y * TILE_HEIGHT * pitch + x * TILE_WIDTH;
size_t chroma_idx = (luma_idx / pitch) * pitch/2 + (luma_idx % pitch);
tile_height /= 2; 
while (tile_height--) {
memcpy(&pic->p[0].p_pixels[luma_idx], src_luma, tile_width);
src_luma += TILE_WIDTH;
luma_idx += pitch;
memcpy(&pic->p[0].p_pixels[luma_idx], src_luma, tile_width);
src_luma += TILE_WIDTH;
luma_idx += pitch;
memcpy(&pic->p[1].p_pixels[chroma_idx], src_chroma, tile_width);
src_chroma += TILE_WIDTH;
chroma_idx += pitch;
}
row_width -= TILE_WIDTH;
}
height -= TILE_HEIGHT;
}
}
