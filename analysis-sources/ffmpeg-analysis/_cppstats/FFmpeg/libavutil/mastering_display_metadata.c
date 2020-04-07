#include <stdint.h>
#include <string.h>
#include "mastering_display_metadata.h"
#include "mem.h"
AVMasteringDisplayMetadata *av_mastering_display_metadata_alloc(void)
{
return av_mallocz(sizeof(AVMasteringDisplayMetadata));
}
AVMasteringDisplayMetadata *av_mastering_display_metadata_create_side_data(AVFrame *frame)
{
AVFrameSideData *side_data = av_frame_new_side_data(frame,
AV_FRAME_DATA_MASTERING_DISPLAY_METADATA,
sizeof(AVMasteringDisplayMetadata));
if (!side_data)
return NULL;
memset(side_data->data, 0, sizeof(AVMasteringDisplayMetadata));
return (AVMasteringDisplayMetadata *)side_data->data;
}
AVContentLightMetadata *av_content_light_metadata_alloc(size_t *size)
{
AVContentLightMetadata *metadata = av_mallocz(sizeof(AVContentLightMetadata));
if (size)
*size = sizeof(*metadata);
return metadata;
}
AVContentLightMetadata *av_content_light_metadata_create_side_data(AVFrame *frame)
{
AVFrameSideData *side_data = av_frame_new_side_data(frame,
AV_FRAME_DATA_CONTENT_LIGHT_LEVEL,
sizeof(AVContentLightMetadata));
if (!side_data)
return NULL;
memset(side_data->data, 0, sizeof(AVContentLightMetadata));
return (AVContentLightMetadata *)side_data->data;
}
