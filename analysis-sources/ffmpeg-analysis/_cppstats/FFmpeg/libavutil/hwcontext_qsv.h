#include <mfx/mfxvideo.h>
typedef struct AVQSVDeviceContext {
mfxSession session;
} AVQSVDeviceContext;
typedef struct AVQSVFramesContext {
mfxFrameSurface1 *surfaces;
int nb_surfaces;
int frame_type;
} AVQSVFramesContext;
