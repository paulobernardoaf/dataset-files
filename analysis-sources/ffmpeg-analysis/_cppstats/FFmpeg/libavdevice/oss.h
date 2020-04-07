#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#define OSS_AUDIO_BLOCK_SIZE 4096
typedef struct OSSAudioData {
AVClass *class;
int fd;
int sample_rate;
int channels;
int frame_size; 
enum AVCodecID codec_id;
unsigned int flip_left : 1;
uint8_t buffer[OSS_AUDIO_BLOCK_SIZE];
int buffer_ptr;
} OSSAudioData;
int ff_oss_audio_open(AVFormatContext *s1, int is_output,
const char *audio_device);
int ff_oss_audio_close(OSSAudioData *s);
