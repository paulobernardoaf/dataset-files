#include "libavutil/attributes.h"
static av_always_inline int ff_data_identifier_is_teletext(int data_identifier)
{
return (data_identifier >= 0x10 && data_identifier <= 0x1F ||
data_identifier >= 0x99 && data_identifier <= 0x9B);
}
static av_always_inline int ff_data_unit_id_is_teletext(int data_unit_id)
{
return (data_unit_id == 0x02 || data_unit_id == 0x03);
}
