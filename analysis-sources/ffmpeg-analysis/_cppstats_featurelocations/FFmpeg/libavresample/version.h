

















#if !defined(AVRESAMPLE_VERSION_H)
#define AVRESAMPLE_VERSION_H







#include "libavutil/version.h"

#define LIBAVRESAMPLE_VERSION_MAJOR 4
#define LIBAVRESAMPLE_VERSION_MINOR 0
#define LIBAVRESAMPLE_VERSION_MICRO 0

#define LIBAVRESAMPLE_VERSION_INT AV_VERSION_INT(LIBAVRESAMPLE_VERSION_MAJOR, LIBAVRESAMPLE_VERSION_MINOR, LIBAVRESAMPLE_VERSION_MICRO)


#define LIBAVRESAMPLE_VERSION AV_VERSION(LIBAVRESAMPLE_VERSION_MAJOR, LIBAVRESAMPLE_VERSION_MINOR, LIBAVRESAMPLE_VERSION_MICRO)


#define LIBAVRESAMPLE_BUILD LIBAVRESAMPLE_VERSION_INT

#define LIBAVRESAMPLE_IDENT "Lavr" AV_STRINGIFY(LIBAVRESAMPLE_VERSION)







#endif 
