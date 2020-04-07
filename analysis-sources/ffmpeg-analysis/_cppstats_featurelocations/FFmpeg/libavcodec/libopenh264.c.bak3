




















#include <string.h>
#include <wels/codec_api.h>
#include <wels/codec_ver.h>

#include "libavutil/log.h"

#include "libopenh264.h"


static int libopenh264_to_ffmpeg_log_level(int libopenh264_log_level)
{
if (libopenh264_log_level >= WELS_LOG_DETAIL) return AV_LOG_TRACE;
else if (libopenh264_log_level >= WELS_LOG_DEBUG) return AV_LOG_DEBUG;
else if (libopenh264_log_level >= WELS_LOG_INFO) return AV_LOG_VERBOSE;
else if (libopenh264_log_level >= WELS_LOG_WARNING) return AV_LOG_WARNING;
else if (libopenh264_log_level >= WELS_LOG_ERROR) return AV_LOG_ERROR;
else return AV_LOG_QUIET;
}

void ff_libopenh264_trace_callback(void *ctx, int level, const char *msg)
{


int equiv_ffmpeg_log_level = libopenh264_to_ffmpeg_log_level(level);
av_log(ctx, equiv_ffmpeg_log_level, "%s\n", msg);
}

int ff_libopenh264_check_version(void *logctx)
{



#if !defined(_WIN32) || !defined(__GNUC__) || !ARCH_X86_32 || AV_GCC_VERSION_AT_LEAST(4, 7)
OpenH264Version libver = WelsGetCodecVersion();
if (memcmp(&libver, &g_stCodecVersion, sizeof(libver))) {
av_log(logctx, AV_LOG_ERROR, "Incorrect library version loaded\n");
return AVERROR(EINVAL);
}
#endif
return 0;
}
