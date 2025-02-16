
























#if !defined(AVUTIL_STEREO3D_H)
#define AVUTIL_STEREO3D_H

#include <stdint.h>

#include "frame.h"



















enum AVStereo3DType {



AV_STEREO3D_2D,











AV_STEREO3D_SIDEBYSIDE,











AV_STEREO3D_TOPBOTTOM,












AV_STEREO3D_FRAMESEQUENCE,











AV_STEREO3D_CHECKERBOARD,












AV_STEREO3D_SIDEBYSIDE_QUINCUNX,











AV_STEREO3D_LINES,











AV_STEREO3D_COLUMNS,
};




enum AVStereo3DView {



AV_STEREO3D_VIEW_PACKED,




AV_STEREO3D_VIEW_LEFT,




AV_STEREO3D_VIEW_RIGHT,
};




#define AV_STEREO3D_FLAG_INVERT (1 << 0)








typedef struct AVStereo3D {



enum AVStereo3DType type;




int flags;




enum AVStereo3DView view;
} AVStereo3D;







AVStereo3D *av_stereo3d_alloc(void);








AVStereo3D *av_stereo3d_create_side_data(AVFrame *frame);








const char *av_stereo3d_type_name(unsigned int type);








int av_stereo3d_from_name(const char *name);






#endif 
