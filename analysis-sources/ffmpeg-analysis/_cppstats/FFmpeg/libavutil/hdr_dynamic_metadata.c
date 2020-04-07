#include "hdr_dynamic_metadata.h"
#include "mem.h"
AVDynamicHDRPlus *av_dynamic_hdr_plus_alloc(size_t *size)
{
AVDynamicHDRPlus *hdr_plus = av_mallocz(sizeof(AVDynamicHDRPlus));
if (!hdr_plus)
return NULL;
if (size)
*size = sizeof(*hdr_plus);
return hdr_plus;
}
AVDynamicHDRPlus *av_dynamic_hdr_plus_create_side_data(AVFrame *frame)
{
AVFrameSideData *side_data = av_frame_new_side_data(frame,
AV_FRAME_DATA_DYNAMIC_HDR_PLUS,
sizeof(AVDynamicHDRPlus));
if (!side_data)
return NULL;
memset(side_data->data, 0, sizeof(AVDynamicHDRPlus));
return (AVDynamicHDRPlus *)side_data->data;
}
