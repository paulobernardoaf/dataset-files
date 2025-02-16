



















#include "downmix_info.h"
#include "frame.h"

AVDownmixInfo *av_downmix_info_update_side_data(AVFrame *frame)
{
AVFrameSideData *side_data;

side_data = av_frame_get_side_data(frame, AV_FRAME_DATA_DOWNMIX_INFO);

if (!side_data)
side_data = av_frame_new_side_data(frame, AV_FRAME_DATA_DOWNMIX_INFO,
sizeof(AVDownmixInfo));

if (!side_data)
return NULL;

return (AVDownmixInfo*)side_data->data;
}
