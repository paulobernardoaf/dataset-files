#include "libavutil/fifo.h"
#include "avformat.h"
typedef struct AudioInterleaveContext {
AVFifoBuffer *fifo;
unsigned fifo_size; 
int64_t n; 
int64_t nb_samples; 
uint64_t dts; 
int sample_size; 
int samples_per_frame; 
AVRational time_base; 
} AudioInterleaveContext;
int ff_audio_interleave_init(AVFormatContext *s, const int samples_per_frame, AVRational time_base);
void ff_audio_interleave_close(AVFormatContext *s);
int ff_audio_rechunk_interleave(AVFormatContext *s, AVPacket *out, AVPacket *pkt, int flush,
int (*get_packet)(AVFormatContext *, AVPacket *, AVPacket *, int),
int (*compare_ts)(AVFormatContext *, const AVPacket *, const AVPacket *));
