

















#if !defined(AVUTIL_HWCONTEXT_QSV_H)
#define AVUTIL_HWCONTEXT_QSV_H

#include <mfx/mfxvideo.h>












typedef struct AVQSVDeviceContext {
mfxSession session;
} AVQSVDeviceContext;




typedef struct AVQSVFramesContext {
mfxFrameSurface1 *surfaces;
int nb_surfaces;




int frame_type;
} AVQSVFramesContext;

#endif 

