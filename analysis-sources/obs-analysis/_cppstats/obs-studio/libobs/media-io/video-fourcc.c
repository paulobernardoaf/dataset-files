#include "../util/c99defs.h"
#include "video-io.h"
#define MAKE_FOURCC(a, b, c, d) ((uint32_t)(((d) << 24) | ((c) << 16) | ((b) << 8) | (a)))
enum video_format video_format_from_fourcc(uint32_t fourcc)
{
switch (fourcc) {
case MAKE_FOURCC('U', 'Y', 'V', 'Y'):
case MAKE_FOURCC('H', 'D', 'Y', 'C'):
case MAKE_FOURCC('U', 'Y', 'N', 'V'):
case MAKE_FOURCC('U', 'Y', 'N', 'Y'):
case MAKE_FOURCC('u', 'y', 'v', '1'):
case MAKE_FOURCC('2', 'v', 'u', 'y'):
case MAKE_FOURCC('2', 'V', 'u', 'y'):
return VIDEO_FORMAT_UYVY;
case MAKE_FOURCC('Y', 'U', 'Y', '2'):
case MAKE_FOURCC('Y', '4', '2', '2'):
case MAKE_FOURCC('V', '4', '2', '2'):
case MAKE_FOURCC('V', 'Y', 'U', 'Y'):
case MAKE_FOURCC('Y', 'U', 'N', 'V'):
case MAKE_FOURCC('y', 'u', 'v', '2'):
case MAKE_FOURCC('y', 'u', 'v', 's'):
return VIDEO_FORMAT_YUY2;
case MAKE_FOURCC('Y', 'V', 'Y', 'U'):
return VIDEO_FORMAT_YVYU;
case MAKE_FOURCC('Y', '8', '0', '0'):
return VIDEO_FORMAT_Y800;
}
return VIDEO_FORMAT_NONE;
}
