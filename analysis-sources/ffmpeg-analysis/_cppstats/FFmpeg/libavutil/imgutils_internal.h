#include <stddef.h>
#include <stdint.h>
int ff_image_copy_plane_uc_from_x86(uint8_t *dst, ptrdiff_t dst_linesize,
const uint8_t *src, ptrdiff_t src_linesize,
ptrdiff_t bytewidth, int height);
