




















#include "avcodec.h"
#include "bsf.h"
#include "internal.h"

static int chomp_filter(AVBSFContext *ctx, AVPacket *pkt)
{
int ret;

ret = ff_bsf_get_packet_ref(ctx, pkt);
if (ret < 0)
return ret;

while (pkt->size > 0 && !pkt->data[pkt->size - 1])
pkt->size--;

return 0;
}




const AVBitStreamFilter ff_chomp_bsf = {
.name = "chomp",
.filter = chomp_filter,
};
