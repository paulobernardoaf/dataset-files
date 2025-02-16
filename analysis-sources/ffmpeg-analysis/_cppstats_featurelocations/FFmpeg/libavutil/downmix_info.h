



















#if !defined(AVUTIL_DOWNMIX_INFO_H)
#define AVUTIL_DOWNMIX_INFO_H

#include "frame.h"



















enum AVDownmixType {
AV_DOWNMIX_TYPE_UNKNOWN, 
AV_DOWNMIX_TYPE_LORO, 
AV_DOWNMIX_TYPE_LTRT, 
AV_DOWNMIX_TYPE_DPLII, 
AV_DOWNMIX_TYPE_NB 
};







typedef struct AVDownmixInfo {



enum AVDownmixType preferred_downmix_type;





double center_mix_level;





double center_mix_level_ltrt;





double surround_mix_level;





double surround_mix_level_ltrt;





double lfe_mix_level;
} AVDownmixInfo;











AVDownmixInfo *av_downmix_info_update_side_data(AVFrame *frame);









#endif 
