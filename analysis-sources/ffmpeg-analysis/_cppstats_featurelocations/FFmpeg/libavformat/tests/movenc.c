



















#include "config.h"

#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/md5.h"

#include "libavformat/avformat.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if !HAVE_GETOPT
#include "compat/getopt.c"
#endif

#define HASH_SIZE 16

static const uint8_t h264_extradata[] = {
0x01, 0x4d, 0x40, 0x1e, 0xff, 0xe1, 0x00, 0x02, 0x67, 0x4d, 0x01, 0x00, 0x02, 0x68, 0xef
};
static const uint8_t aac_extradata[] = {
0x12, 0x10
};


static const char *format = "mp4";
AVFormatContext *ctx;
uint8_t iobuf[32768];
AVDictionary *opts;

int write_file;
const char *cur_name;
FILE* out;
int out_size;
struct AVMD5* md5;
uint8_t hash[HASH_SIZE];

AVStream *video_st, *audio_st;
int64_t audio_dts, video_dts;

int bframes;
int64_t duration;
int64_t audio_duration;
int frames;
int gop_size;
int64_t next_p_pts;
enum AVPictureType last_picture;
int skip_write;
int skip_write_audio;
int clear_duration;
int force_iobuf_size;
int do_interleave;
int fake_pkt_duration;

int num_warnings;

int check_faults;


static void count_warnings(void *avcl, int level, const char *fmt, va_list vl)
{
if (level == AV_LOG_WARNING)
num_warnings++;
}

static void init_count_warnings(void)
{
av_log_set_callback(count_warnings);
num_warnings = 0;
}

static void reset_count_warnings(void)
{
av_log_set_callback(av_log_default_callback);
}

static int io_write(void *opaque, uint8_t *buf, int size)
{
out_size += size;
av_md5_update(md5, buf, size);
if (out)
fwrite(buf, 1, size, out);
return size;
}

static int io_write_data_type(void *opaque, uint8_t *buf, int size,
enum AVIODataMarkerType type, int64_t time)
{
char timebuf[30], content[5] = { 0 };
const char *str;
switch (type) {
case AVIO_DATA_MARKER_HEADER: str = "header"; break;
case AVIO_DATA_MARKER_SYNC_POINT: str = "sync"; break;
case AVIO_DATA_MARKER_BOUNDARY_POINT: str = "boundary"; break;
case AVIO_DATA_MARKER_UNKNOWN: str = "unknown"; break;
case AVIO_DATA_MARKER_TRAILER: str = "trailer"; break;
default: str = "unknown"; break;
}
if (time == AV_NOPTS_VALUE)
snprintf(timebuf, sizeof(timebuf), "nopts");
else
snprintf(timebuf, sizeof(timebuf), "%"PRId64, time);


if (type != AVIO_DATA_MARKER_UNKNOWN &&
type != AVIO_DATA_MARKER_TRAILER &&
(type != AVIO_DATA_MARKER_HEADER || out_size == 0) &&
size >= 8)
memcpy(content, &buf[4], 4);
else
snprintf(content, sizeof(content), "-");
printf("write_data len %d, time %s, type %s atom %s\n", size, timebuf, str, content);
return io_write(opaque, buf, size);
}

static void init_out(const char *name)
{
char buf[100];
cur_name = name;
snprintf(buf, sizeof(buf), "%s.%s", cur_name, format);

av_md5_init(md5);
if (write_file) {
out = fopen(buf, "wb");
if (!out)
perror(buf);
}
out_size = 0;
}

static void close_out(void)
{
int i;
av_md5_final(md5, hash);
for (i = 0; i < HASH_SIZE; i++)
printf("%02x", hash[i]);
printf(" %d %s\n", out_size, cur_name);
if (out)
fclose(out);
out = NULL;
}

static void check_func(int value, int line, const char *msg, ...)
{
if (!value) {
va_list ap;
va_start(ap, msg);
printf("%d: ", line);
vprintf(msg, ap);
printf("\n");
check_faults++;
va_end(ap);
}
}
#define check(value, ...) check_func(value, __LINE__, __VA_ARGS__)

static void init_fps(int bf, int audio_preroll, int fps)
{
AVStream *st;
int iobuf_size = force_iobuf_size ? force_iobuf_size : sizeof(iobuf);
ctx = avformat_alloc_context();
if (!ctx)
exit(1);
ctx->oformat = av_guess_format(format, NULL, NULL);
if (!ctx->oformat)
exit(1);
ctx->pb = avio_alloc_context(iobuf, iobuf_size, AVIO_FLAG_WRITE, NULL, NULL, io_write, NULL);
if (!ctx->pb)
exit(1);
ctx->pb->write_data_type = io_write_data_type;
ctx->flags |= AVFMT_FLAG_BITEXACT;

st = avformat_new_stream(ctx, NULL);
if (!st)
exit(1);
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = AV_CODEC_ID_H264;
st->codecpar->width = 640;
st->codecpar->height = 480;
st->time_base.num = 1;
st->time_base.den = 30;
st->codecpar->extradata_size = sizeof(h264_extradata);
st->codecpar->extradata = av_mallocz(st->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
if (!st->codecpar->extradata)
exit(1);
memcpy(st->codecpar->extradata, h264_extradata, sizeof(h264_extradata));
video_st = st;

st = avformat_new_stream(ctx, NULL);
if (!st)
exit(1);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_AAC;
st->codecpar->sample_rate = 44100;
st->codecpar->channels = 2;
st->time_base.num = 1;
st->time_base.den = 44100;
st->codecpar->extradata_size = sizeof(aac_extradata);
st->codecpar->extradata = av_mallocz(st->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
if (!st->codecpar->extradata)
exit(1);
memcpy(st->codecpar->extradata, aac_extradata, sizeof(aac_extradata));
audio_st = st;

if (avformat_write_header(ctx, &opts) < 0)
exit(1);
av_dict_free(&opts);

frames = 0;
gop_size = 30;
duration = video_st->time_base.den / fps;
audio_duration = 1024LL * audio_st->time_base.den / audio_st->codecpar->sample_rate;
if (audio_preroll)
audio_preroll = 2048LL * audio_st->time_base.den / audio_st->codecpar->sample_rate;

bframes = bf;
video_dts = bframes ? -duration : 0;
audio_dts = -audio_preroll;
}

static void init(int bf, int audio_preroll)
{
init_fps(bf, audio_preroll, 30);
}

static void mux_frames(int n, int c)
{
int end_frames = frames + n;
while (1) {
AVPacket pkt;
uint8_t pktdata[8] = { 0 };
av_init_packet(&pkt);

if (av_compare_ts(audio_dts, audio_st->time_base, video_dts, video_st->time_base) < 0) {
pkt.dts = pkt.pts = audio_dts;
pkt.stream_index = 1;
pkt.duration = audio_duration;
audio_dts += audio_duration;
} else {
if (frames == end_frames)
break;
pkt.dts = video_dts;
pkt.stream_index = 0;
pkt.duration = duration;
if ((frames % gop_size) == 0) {
pkt.flags |= AV_PKT_FLAG_KEY;
last_picture = AV_PICTURE_TYPE_I;
pkt.pts = pkt.dts + duration;
video_dts = pkt.pts;
} else {
if (last_picture == AV_PICTURE_TYPE_P) {
last_picture = AV_PICTURE_TYPE_B;
pkt.pts = pkt.dts;
video_dts = next_p_pts;
} else {
last_picture = AV_PICTURE_TYPE_P;
if (((frames + 1) % gop_size) == 0) {
pkt.pts = pkt.dts + duration;
video_dts = pkt.pts;
} else {
next_p_pts = pkt.pts = pkt.dts + 2 * duration;
video_dts += duration;
}
}
}
if (!bframes)
pkt.pts = pkt.dts;
if (fake_pkt_duration)
pkt.duration = fake_pkt_duration;
frames++;
}

if (clear_duration)
pkt.duration = 0;
AV_WB32(pktdata + 4, pkt.pts);
pkt.data = pktdata;
pkt.size = 8;
if (skip_write)
continue;
if (skip_write_audio && pkt.stream_index == 1)
continue;

if (c) {
pkt.pts += (1LL<<32);
pkt.dts += (1LL<<32);
}

if (do_interleave)
av_interleaved_write_frame(ctx, &pkt);
else
av_write_frame(ctx, &pkt);
}
}

static void mux_gops(int n)
{
mux_frames(gop_size * n, 0);
}

static void skip_gops(int n)
{
skip_write = 1;
mux_gops(n);
skip_write = 0;
}

static void signal_init_ts(void)
{
AVPacket pkt;
av_init_packet(&pkt);
pkt.size = 0;
pkt.data = NULL;

pkt.stream_index = 0;
pkt.dts = video_dts;
pkt.pts = 0;
av_write_frame(ctx, &pkt);

pkt.stream_index = 1;
pkt.dts = pkt.pts = audio_dts;
av_write_frame(ctx, &pkt);
}

static void finish(void)
{
av_write_trailer(ctx);
avio_context_free(&ctx->pb);
avformat_free_context(ctx);
ctx = NULL;
}

static void help(void)
{
printf("movenc-test [-w]\n"
"-w write output into files\n");
}

int main(int argc, char **argv)
{
int c;
uint8_t header[HASH_SIZE];
uint8_t content[HASH_SIZE];
int empty_moov_pos;
int prev_pos;

for (;;) {
c = getopt(argc, argv, "wh");
if (c == -1)
break;
switch (c) {
case 'w':
write_file = 1;
break;
default:
case 'h':
help();
return 0;
}
}

md5 = av_md5_alloc();
if (!md5)
return 1;




init_out("non-empty-moov");
av_dict_set(&opts, "movflags", "frag_keyframe", 0);
init(0, 0);
mux_gops(2);
finish();
close_out();



init_out("non-empty-moov-elst");
av_dict_set(&opts, "movflags", "frag_keyframe", 0);
av_dict_set(&opts, "use_editlist", "1", 0);
init(1, 1);
mux_gops(2);
finish();
close_out();





init_out("non-empty-moov-no-elst");
av_dict_set(&opts, "movflags", "frag_keyframe", 0);
av_dict_set(&opts, "use_editlist", "0", 0);
init(1, 0);
mux_gops(2);
finish();
close_out();

format = "ismv";

init_out("ismv");
av_dict_set(&opts, "movflags", "frag_keyframe", 0);
init(1, 1);
mux_gops(2);
finish();
close_out();
format = "mp4";



init_out("empty-moov");
av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov", 0);
av_dict_set(&opts, "use_editlist", "0", 0);
init(0, 0);
mux_gops(2);
finish();
close_out();
memcpy(content, hash, HASH_SIZE);




init_out("empty-moov-no-elst");
av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov", 0);
init(1, 0);
mux_gops(2);
finish();
close_out();





init_count_warnings();
init_out("empty-moov-no-elst-no-adjust");
av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov", 0);
av_dict_set(&opts, "avoid_negative_ts", "0", 0);
init(1, 0);
mux_gops(2);
finish();
close_out();

reset_count_warnings();
check(num_warnings > 0, "No warnings printed for unhandled start offset");



init_out("delay-moov");
av_dict_set(&opts, "movflags", "frag_keyframe+delay_moov", 0);
av_dict_set(&opts, "use_editlist", "0", 0);
init(0, 0);
mux_gops(2);
finish();
close_out();
check(!memcmp(hash, content, HASH_SIZE), "delay_moov differs from empty_moov");


init_out("delay-moov-elst");
av_dict_set(&opts, "movflags", "frag_keyframe+delay_moov", 0);
init(1, 1);
mux_gops(2);
finish();
close_out();


skip_write_audio = 1;
init_out("delay-moov-empty-track");
av_dict_set(&opts, "movflags", "frag_keyframe+delay_moov", 0);
init(0, 0);
mux_gops(2);


check(out_size == 0, "delay_moov flushed prematurely");


finish();
close_out();
check(out_size > 0, "delay_moov didn't output anything");



init_out("delay-moov-empty-track-flush");
av_dict_set(&opts, "movflags", "frag_custom+delay_moov", 0);
init(0, 0);
mux_gops(1);
av_write_frame(ctx, NULL); 
check(out_size > 0, "No moov written");
av_write_frame(ctx, NULL);
mux_gops(1);
av_write_frame(ctx, NULL);
finish();
close_out();

skip_write_audio = 0;





init_out("empty-moov-header");
av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov", 0);
av_dict_set(&opts, "use_editlist", "0", 0);
init(0, 0);
close_out();
memcpy(header, hash, HASH_SIZE);
init_out("empty-moov-content");
mux_gops(2);

check(out_size > 0, "No automatic flush?");
empty_moov_pos = prev_pos = out_size;

av_write_frame(ctx, NULL);
check(out_size > prev_pos, "No second fragment flushed?");
prev_pos = out_size;

av_write_frame(ctx, NULL);
check(out_size == prev_pos, "More data written?");
close_out();
memcpy(content, hash, HASH_SIZE);

finish();

init_out("delay-moov-header");
av_dict_set(&opts, "movflags", "frag_custom+delay_moov", 0);
av_dict_set(&opts, "use_editlist", "0", 0);
init(0, 0);
check(out_size == 0, "Output written during init with delay_moov");
mux_gops(1); 
av_write_frame(ctx, NULL); 
close_out();
check(!memcmp(hash, header, HASH_SIZE), "delay_moov header differs from empty_moov");
init_out("delay-moov-content");
av_write_frame(ctx, NULL); 
check(out_size == empty_moov_pos, "Manually flushed content differs from automatically flushed, %d vs %d", out_size, empty_moov_pos);
mux_gops(1); 
av_write_frame(ctx, NULL); 
close_out();
check(!memcmp(hash, content, HASH_SIZE), "delay_moov content differs from empty_moov");
finish();





av_dict_set(&opts, "movflags", "frag_custom+empty_moov+dash", 0);
init(0, 0);
mux_gops(1);
av_write_frame(ctx, NULL); 
init_out("empty-moov-second-frag");
mux_gops(1);
av_write_frame(ctx, NULL); 
close_out();
memcpy(content, hash, HASH_SIZE);
finish();



av_dict_set(&opts, "movflags", "frag_custom+empty_moov+dash+frag_discont", 0);
av_dict_set(&opts, "fragment_index", "2", 0);
av_dict_set(&opts, "avoid_negative_ts", "0", 0);
av_dict_set(&opts, "use_editlist", "0", 0);
init(0, 0);
skip_gops(1);
init_out("empty-moov-second-frag-discont");
mux_gops(1);
av_write_frame(ctx, NULL); 
close_out();
check(!memcmp(hash, content, HASH_SIZE), "discontinuously written fragment differs");
finish();



av_dict_set(&opts, "movflags", "frag_custom+delay_moov+dash+frag_discont", 0);
av_dict_set(&opts, "fragment_index", "2", 0);
init(0, 0);
skip_gops(1);
mux_gops(1);
av_write_frame(ctx, NULL); 
init_out("delay-moov-second-frag-discont");
av_write_frame(ctx, NULL); 
close_out();
check(!memcmp(hash, content, HASH_SIZE), "discontinuously written fragment differs");
finish();





av_dict_set(&opts, "movflags", "frag_custom+delay_moov+dash", 0);
init(1, 0);
mux_gops(1);
init_out("delay-moov-elst-init");
av_write_frame(ctx, NULL); 
close_out();
memcpy(header, hash, HASH_SIZE);
av_write_frame(ctx, NULL); 
init_out("delay-moov-elst-second-frag");
mux_gops(1);
av_write_frame(ctx, NULL); 
close_out();
memcpy(content, hash, HASH_SIZE);
finish();

av_dict_set(&opts, "movflags", "frag_custom+delay_moov+dash+frag_discont", 0);
av_dict_set(&opts, "fragment_index", "2", 0);
init(1, 0);
skip_gops(1);
mux_gops(1); 
init_out("delay-moov-elst-init-discont");
av_write_frame(ctx, NULL); 
close_out();
check(!memcmp(hash, header, HASH_SIZE), "discontinuously written header differs");
init_out("delay-moov-elst-second-frag-discont");
av_write_frame(ctx, NULL); 
close_out();
check(!memcmp(hash, content, HASH_SIZE), "discontinuously written fragment differs");
finish();




av_dict_set(&opts, "movflags", "frag_custom+delay_moov+dash", 0);
init(1, 1);
mux_gops(1);
init_out("delay-moov-elst-signal-init");
av_write_frame(ctx, NULL); 
close_out();
memcpy(header, hash, HASH_SIZE);
av_write_frame(ctx, NULL); 
init_out("delay-moov-elst-signal-second-frag");
mux_gops(1);
av_write_frame(ctx, NULL); 
close_out();
memcpy(content, hash, HASH_SIZE);
finish();

av_dict_set(&opts, "movflags", "frag_custom+delay_moov+dash+frag_discont", 0);
av_dict_set(&opts, "fragment_index", "2", 0);
init(1, 1);
signal_init_ts();
skip_gops(1);
mux_gops(1); 
init_out("delay-moov-elst-signal-init-discont");
av_write_frame(ctx, NULL); 
close_out();
check(!memcmp(hash, header, HASH_SIZE), "discontinuously written header differs");
init_out("delay-moov-elst-signal-second-frag-discont");
av_write_frame(ctx, NULL); 
close_out();
check(!memcmp(hash, content, HASH_SIZE), "discontinuously written fragment differs");
finish();



av_dict_set(&opts, "movflags", "frag_custom+delay_moov+dash+frag_discont", 0);
av_dict_set(&opts, "fragment_index", "2", 0);
init(1, 1);
signal_init_ts();
skip_gops(1);
mux_frames(gop_size, 1); 
init_out("delay-moov-elst-signal-init-discont-largets");
av_write_frame(ctx, NULL); 
close_out();
init_out("delay-moov-elst-signal-second-frag-discont-largets");
av_write_frame(ctx, NULL); 
close_out();
finish();













init_out("vfr");
av_dict_set(&opts, "movflags", "frag_keyframe+delay_moov+dash", 0);
init_fps(1, 1, 3);
mux_frames(gop_size/2, 0);
duration /= 10;
mux_frames(gop_size/2, 0);
mux_gops(1);
finish();
close_out();








init_count_warnings();
clear_duration = 1;
init_out("vfr-noduration");
av_dict_set(&opts, "movflags", "frag_keyframe+delay_moov+dash", 0);
init_fps(1, 1, 3);
mux_frames(gop_size/2, 0);
duration /= 10;
mux_frames(gop_size/2, 0);
mux_gops(1);
finish();
close_out();
clear_duration = 0;
reset_count_warnings();
check(num_warnings > 0, "No warnings printed for filled in durations");



force_iobuf_size = 1500;
init_out("large_frag");
av_dict_set(&opts, "movflags", "frag_keyframe+delay_moov", 0);
init_fps(1, 1, 3);
mux_gops(2);
finish();
close_out();
force_iobuf_size = 0;





do_interleave = 1;
init_out("vfr-noduration-interleave");
av_dict_set(&opts, "movflags", "frag_keyframe+delay_moov", 0);
av_dict_set(&opts, "frag_duration", "650000", 0);
init_fps(1, 1, 30);
mux_frames(gop_size/2, 0);



fake_pkt_duration = duration;
duration *= 10;
mux_frames(1, 0);
fake_pkt_duration = 0;
duration /= 10;
mux_frames(gop_size/2 - 1, 0);
mux_gops(1);
finish();
close_out();
clear_duration = 0;
do_interleave = 0;




init_out("delay-moov-elst-neg-cts");
av_dict_set(&opts, "movflags", "frag_keyframe+delay_moov+negative_cts_offsets", 0);
init(1, 1);
mux_gops(2);
finish();
close_out();




init_out("empty-moov-neg-cts");
av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov+negative_cts_offsets", 0);
init(1, 0);
mux_gops(2);
finish();
close_out();

av_free(md5);

return check_faults > 0 ? 1 : 0;
}
