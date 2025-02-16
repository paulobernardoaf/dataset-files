






















#if !defined(AVCOMMON_COMPAT_H)
#define AVCOMMON_COMPAT_H 1

#define AVPROVIDER(lib) ((lib##_VERSION_MICRO < 100) ? "libav" : "ffmpeg")

#if defined(HAVE_LIBAVCODEC_AVCODEC_H)
#include <libavcodec/avcodec.h>





#define LIBAVCODEC_VERSION_CHECK( a, b, c, d, e ) ( (LIBAVCODEC_VERSION_MICRO < 100 && LIBAVCODEC_VERSION_INT >= AV_VERSION_INT( a, b, c ) ) || (LIBAVCODEC_VERSION_MICRO >= 100 && LIBAVCODEC_VERSION_INT >= AV_VERSION_INT( a, d, e ) ) )



#if !defined(AV_CODEC_FLAG_OUTPUT_CORRUPT)
#define AV_CODEC_FLAG_OUTPUT_CORRUPT CODEC_FLAG_OUTPUT_CORRUPT
#endif
#if !defined(AV_CODEC_FLAG_GRAY)
#define AV_CODEC_FLAG_GRAY CODEC_FLAG_GRAY
#endif
#if !defined(AV_CODEC_FLAG_DR1)
#define AV_CODEC_FLAG_DR1 CODEC_FLAG_DR1
#endif
#if !defined(AV_CODEC_FLAG_DELAY)
#define AV_CODEC_FLAG_DELAY CODEC_FLAG_DELAY
#endif
#if !defined(AV_CODEC_FLAG2_FAST)
#define AV_CODEC_FLAG2_FAST CODEC_FLAG2_FAST
#endif
#if !defined(FF_INPUT_BUFFER_PADDING_SIZE)
#define FF_INPUT_BUFFER_PADDING_SIZE AV_INPUT_BUFFER_PADDING_SIZE
#endif
#if !defined(AV_CODEC_FLAG_INTERLACED_DCT)
#define AV_CODEC_FLAG_INTERLACED_DCT CODEC_FLAG_INTERLACED_DCT
#endif
#if !defined(AV_CODEC_FLAG_INTERLACED_ME)
#define AV_CODEC_FLAG_INTERLACED_ME CODEC_FLAG_INTERLACED_ME
#endif
#if !defined(AV_CODEC_FLAG_GLOBAL_HEADER)
#define AV_CODEC_FLAG_GLOBAL_HEADER CODEC_FLAG_GLOBAL_HEADER
#endif
#if !defined(AV_CODEC_FLAG_LOW_DELAY)
#define AV_CODEC_FLAG_LOW_DELAY CODEC_FLAG_LOW_DELAY
#endif
#if !defined(AV_CODEC_CAP_SMALL_LAST_FRAME)
#define AV_CODEC_CAP_SMALL_LAST_FRAME CODEC_CAP_SMALL_LAST_FRAME
#endif
#if !defined(AV_INPUT_BUFFER_MIN_SIZE)
#define AV_INPUT_BUFFER_MIN_SIZE FF_MIN_BUFFER_SIZE
#endif
#if !defined(FF_MAX_B_FRAMES)
#define FF_MAX_B_FRAMES 16 
#endif

#endif 

#if defined(HAVE_LIBAVUTIL_AVUTIL_H)
#include <libavutil/avutil.h>





#define LIBAVUTIL_VERSION_CHECK( a, b, c, d, e ) ( (LIBAVUTIL_VERSION_MICRO < 100 && LIBAVUTIL_VERSION_INT >= AV_VERSION_INT( a, b, c ) ) || (LIBAVUTIL_VERSION_MICRO >= 100 && LIBAVUTIL_VERSION_INT >= AV_VERSION_INT( a, d, e ) ) )



#endif 

#if LIBAVUTIL_VERSION_MAJOR >= 55
#define FF_API_AUDIOCONVERT 1
#endif


#if !defined(PixelFormat)
#define PixelFormat AVPixelFormat
#endif

#if defined(HAVE_LIBAVFORMAT_AVFORMAT_H)
#include <libavformat/avformat.h>

#define LIBAVFORMAT_VERSION_CHECK( a, b, c, d, e ) ( (LIBAVFORMAT_VERSION_MICRO < 100 && LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT( a, b, c ) ) || (LIBAVFORMAT_VERSION_MICRO >= 100 && LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT( a, d, e ) ) )



#endif

#endif
