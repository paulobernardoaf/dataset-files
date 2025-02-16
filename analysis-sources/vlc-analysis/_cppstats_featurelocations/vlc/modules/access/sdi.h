



















#if defined(__cplusplus)
extern "C" {
#endif

#include <vlc_common.h>
#include <vlc_block.h>

#include <inttypes.h>

void v210_convert(uint16_t *dst, const uint32_t *bytes, const int width, const int height);

block_t *vanc_to_cc(vlc_object_t *, uint16_t *, size_t);
#define vanc_to_cc(obj, buf, words) vanc_to_cc(VLC_OBJECT(obj), buf, words)

#if defined(__cplusplus)
}
#endif
