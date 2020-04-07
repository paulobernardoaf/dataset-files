typedef enum DecklinkPtsSource {
PTS_SRC_AUDIO = 1,
PTS_SRC_VIDEO = 2,
PTS_SRC_REFERENCE = 3,
PTS_SRC_WALLCLOCK = 4,
PTS_SRC_ABS_WALLCLOCK = 5,
PTS_SRC_NB
} DecklinkPtsSource;
struct decklink_cctx {
const AVClass *cclass;
void *ctx;
int list_devices;
int list_formats;
int64_t teletext_lines;
double preroll;
int audio_channels;
int audio_depth;
int duplex_mode;
DecklinkPtsSource audio_pts_source;
DecklinkPtsSource video_pts_source;
int audio_input;
int video_input;
int tc_format;
int draw_bars;
char *format_code;
int raw_format;
int64_t queue_size;
int copyts;
int64_t timestamp_align;
int timing_offset;
int wait_for_tc;
};
