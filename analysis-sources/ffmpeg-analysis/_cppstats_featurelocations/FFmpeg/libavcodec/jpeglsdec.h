


























#if !defined(AVCODEC_JPEGLSDEC_H)
#define AVCODEC_JPEGLSDEC_H

#include "mjpeg.h"
#include "mjpegdec.h"




int ff_jpegls_decode_lse(MJpegDecodeContext *s);

int ff_jpegls_decode_picture(MJpegDecodeContext *s, int near,
int point_transform, int ilv);

#endif 
