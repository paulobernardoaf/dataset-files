















#pragma once

#include "ff-callbacks.h"
#include "ff-circular-queue.h"
#include "ff-clock.h"
#include "ff-packet-queue.h"
#include "ff-timer.h"

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct ff_decoder {
AVCodecContext *codec;
AVStream *stream;

pthread_t decoder_thread;
struct ff_timer refresh_timer;

struct ff_callbacks *callbacks;
struct ff_packet_queue packet_queue;
struct ff_circular_queue frame_queue;
unsigned int packet_queue_size;

double timer_next_wake;
double previous_pts; 
double previous_pts_diff; 
double predicted_pts; 
double current_pts; 
int64_t current_pts_time; 
int64_t start_pts;

bool hwaccel_decoder;
enum AVDiscard frame_drop;
struct ff_clock *clock;
enum ff_av_sync_type natural_sync_clock;

bool first_frame;
bool eof;
bool abort;
bool finished;
};

typedef struct ff_decoder ff_decoder_t;

struct ff_decoder *ff_decoder_init(AVCodecContext *codec_context,
AVStream *stream,
unsigned int packet_queue_size,
unsigned int frame_queue_size);
bool ff_decoder_start(struct ff_decoder *decoder);
void ff_decoder_free(struct ff_decoder *decoder);

bool ff_decoder_full(struct ff_decoder *decoder);
bool ff_decoder_accept(struct ff_decoder *decoder, struct ff_packet *packet);

double ff_decoder_clock(void *opaque);

void ff_decoder_schedule_refresh(struct ff_decoder *decoder, int delay);
void ff_decoder_refresh(void *opaque);

double ff_decoder_get_best_effort_pts(struct ff_decoder *decoder,
AVFrame *frame);

bool ff_decoder_set_frame_drop_state(struct ff_decoder *decoder,
int64_t start_time, int64_t pts);

#if defined(__cplusplus)
}
#endif
