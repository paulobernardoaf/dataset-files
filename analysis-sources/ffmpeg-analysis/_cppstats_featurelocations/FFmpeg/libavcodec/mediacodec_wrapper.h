





















#if !defined(AVCODEC_MEDIACODEC_WRAPPER_H)
#define AVCODEC_MEDIACODEC_WRAPPER_H

#include <stdint.h>
#include <sys/types.h>

#include "avcodec.h"



























int ff_AMediaCodecProfile_getProfileFromAVCodecContext(AVCodecContext *avctx);

char *ff_AMediaCodecList_getCodecNameByType(const char *mime, int profile, int encoder, void *log_ctx);

struct FFAMediaFormat;
typedef struct FFAMediaFormat FFAMediaFormat;

FFAMediaFormat *ff_AMediaFormat_new(void);
int ff_AMediaFormat_delete(FFAMediaFormat* format);

char* ff_AMediaFormat_toString(FFAMediaFormat* format);

int ff_AMediaFormat_getInt32(FFAMediaFormat* format, const char *name, int32_t *out);
int ff_AMediaFormat_getInt64(FFAMediaFormat* format, const char *name, int64_t *out);
int ff_AMediaFormat_getFloat(FFAMediaFormat* format, const char *name, float *out);
int ff_AMediaFormat_getBuffer(FFAMediaFormat* format, const char *name, void** data, size_t *size);
int ff_AMediaFormat_getString(FFAMediaFormat* format, const char *name, const char **out);

void ff_AMediaFormat_setInt32(FFAMediaFormat* format, const char* name, int32_t value);
void ff_AMediaFormat_setInt64(FFAMediaFormat* format, const char* name, int64_t value);
void ff_AMediaFormat_setFloat(FFAMediaFormat* format, const char* name, float value);
void ff_AMediaFormat_setString(FFAMediaFormat* format, const char* name, const char* value);
void ff_AMediaFormat_setBuffer(FFAMediaFormat* format, const char* name, void* data, size_t size);

struct FFAMediaCodec;
typedef struct FFAMediaCodec FFAMediaCodec;
typedef struct FFAMediaCodecCryptoInfo FFAMediaCodecCryptoInfo;

struct FFAMediaCodecBufferInfo {
int32_t offset;
int32_t size;
int64_t presentationTimeUs;
uint32_t flags;
};
typedef struct FFAMediaCodecBufferInfo FFAMediaCodecBufferInfo;

char *ff_AMediaCodec_getName(FFAMediaCodec *codec);

FFAMediaCodec* ff_AMediaCodec_createCodecByName(const char *name);
FFAMediaCodec* ff_AMediaCodec_createDecoderByType(const char *mime_type);
FFAMediaCodec* ff_AMediaCodec_createEncoderByType(const char *mime_type);

int ff_AMediaCodec_configure(FFAMediaCodec* codec, const FFAMediaFormat* format, void* surface, void *crypto, uint32_t flags);
int ff_AMediaCodec_start(FFAMediaCodec* codec);
int ff_AMediaCodec_stop(FFAMediaCodec* codec);
int ff_AMediaCodec_flush(FFAMediaCodec* codec);
int ff_AMediaCodec_delete(FFAMediaCodec* codec);

uint8_t* ff_AMediaCodec_getInputBuffer(FFAMediaCodec* codec, size_t idx, size_t *out_size);
uint8_t* ff_AMediaCodec_getOutputBuffer(FFAMediaCodec* codec, size_t idx, size_t *out_size);

ssize_t ff_AMediaCodec_dequeueInputBuffer(FFAMediaCodec* codec, int64_t timeoutUs);
int ff_AMediaCodec_queueInputBuffer(FFAMediaCodec* codec, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags);

ssize_t ff_AMediaCodec_dequeueOutputBuffer(FFAMediaCodec* codec, FFAMediaCodecBufferInfo *info, int64_t timeoutUs);
FFAMediaFormat* ff_AMediaCodec_getOutputFormat(FFAMediaCodec* codec);

int ff_AMediaCodec_releaseOutputBuffer(FFAMediaCodec* codec, size_t idx, int render);
int ff_AMediaCodec_releaseOutputBufferAtTime(FFAMediaCodec *codec, size_t idx, int64_t timestampNs);

int ff_AMediaCodec_infoTryAgainLater(FFAMediaCodec *codec, ssize_t idx);
int ff_AMediaCodec_infoOutputBuffersChanged(FFAMediaCodec *codec, ssize_t idx);
int ff_AMediaCodec_infoOutputFormatChanged(FFAMediaCodec *codec, ssize_t indx);

int ff_AMediaCodec_getBufferFlagCodecConfig (FFAMediaCodec *codec);
int ff_AMediaCodec_getBufferFlagEndOfStream(FFAMediaCodec *codec);
int ff_AMediaCodec_getBufferFlagKeyFrame(FFAMediaCodec *codec);

int ff_AMediaCodec_getConfigureFlagEncode(FFAMediaCodec *codec);

int ff_AMediaCodec_cleanOutputBuffers(FFAMediaCodec *codec);

int ff_Build_SDK_INT(AVCodecContext *avctx);

#endif 
