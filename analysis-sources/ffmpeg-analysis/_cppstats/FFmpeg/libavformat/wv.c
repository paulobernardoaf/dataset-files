#include <stdint.h>
#include <string.h>
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "wv.h"
int ff_wv_parse_header(WvHeader *wv, const uint8_t *data)
{
memset(wv, 0, sizeof(*wv));
if (AV_RL32(data) != MKTAG('w', 'v', 'p', 'k'))
return AVERROR_INVALIDDATA;
wv->blocksize = AV_RL32(data + 4);
if (wv->blocksize < 24 || wv->blocksize > WV_BLOCK_LIMIT)
return AVERROR_INVALIDDATA;
wv->blocksize -= 24;
wv->version = AV_RL16(data + 8);
wv->total_samples = AV_RL32(data + 12);
wv->block_idx = AV_RL32(data + 16);
wv->samples = AV_RL32(data + 20);
wv->flags = AV_RL32(data + 24);
wv->crc = AV_RL32(data + 28);
wv->initial = !!(wv->flags & WV_FLAG_INITIAL_BLOCK);
wv->final = !!(wv->flags & WV_FLAG_FINAL_BLOCK);
return 0;
}
