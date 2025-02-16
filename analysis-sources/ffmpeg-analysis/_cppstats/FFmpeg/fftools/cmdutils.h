#include <stdint.h>
#include "config.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#if defined(_WIN32)
#undef main 
#endif
extern const char program_name[];
extern const int program_birth_year;
extern AVCodecContext *avcodec_opts[AVMEDIA_TYPE_NB];
extern AVFormatContext *avformat_opts;
extern AVDictionary *sws_dict;
extern AVDictionary *swr_opts;
extern AVDictionary *format_opts, *codec_opts, *resample_opts;
extern int hide_banner;
void register_exit(void (*cb)(int ret));
void exit_program(int ret) av_noreturn;
void init_dynload(void);
void init_opts(void);
void uninit_opts(void);
void log_callback_help(void* ptr, int level, const char* fmt, va_list vl);
int opt_cpuflags(void *optctx, const char *opt, const char *arg);
int opt_default(void *optctx, const char *opt, const char *arg);
int opt_loglevel(void *optctx, const char *opt, const char *arg);
int opt_report(void *optctx, const char *opt, const char *arg);
int opt_max_alloc(void *optctx, const char *opt, const char *arg);
int opt_codec_debug(void *optctx, const char *opt, const char *arg);
int opt_timelimit(void *optctx, const char *opt, const char *arg);
double parse_number_or_die(const char *context, const char *numstr, int type,
double min, double max);
int64_t parse_time_or_die(const char *context, const char *timestr,
int is_duration);
typedef struct SpecifierOpt {
char *specifier; 
union {
uint8_t *str;
int i;
int64_t i64;
uint64_t ui64;
float f;
double dbl;
} u;
} SpecifierOpt;
typedef struct OptionDef {
const char *name;
int flags;
#define HAS_ARG 0x0001
#define OPT_BOOL 0x0002
#define OPT_EXPERT 0x0004
#define OPT_STRING 0x0008
#define OPT_VIDEO 0x0010
#define OPT_AUDIO 0x0020
#define OPT_INT 0x0080
#define OPT_FLOAT 0x0100
#define OPT_SUBTITLE 0x0200
#define OPT_INT64 0x0400
#define OPT_EXIT 0x0800
#define OPT_DATA 0x1000
#define OPT_PERFILE 0x2000 
#define OPT_OFFSET 0x4000 
#define OPT_SPEC 0x8000 
#define OPT_TIME 0x10000
#define OPT_DOUBLE 0x20000
#define OPT_INPUT 0x40000
#define OPT_OUTPUT 0x80000
union {
void *dst_ptr;
int (*func_arg)(void *, const char *, const char *);
size_t off;
} u;
const char *help;
const char *argname;
} OptionDef;
void show_help_options(const OptionDef *options, const char *msg, int req_flags,
int rej_flags, int alt_flags);
#if CONFIG_AVDEVICE
#define CMDUTILS_COMMON_OPTIONS_AVDEVICE { "sources" , OPT_EXIT | HAS_ARG, { .func_arg = show_sources }, "list sources of the input device", "device" }, { "sinks" , OPT_EXIT | HAS_ARG, { .func_arg = show_sinks }, "list sinks of the output device", "device" }, 
#else
#define CMDUTILS_COMMON_OPTIONS_AVDEVICE
#endif
#define CMDUTILS_COMMON_OPTIONS { "L", OPT_EXIT, { .func_arg = show_license }, "show license" }, { "h", OPT_EXIT, { .func_arg = show_help }, "show help", "topic" }, { "?", OPT_EXIT, { .func_arg = show_help }, "show help", "topic" }, { "help", OPT_EXIT, { .func_arg = show_help }, "show help", "topic" }, { "-help", OPT_EXIT, { .func_arg = show_help }, "show help", "topic" }, { "version", OPT_EXIT, { .func_arg = show_version }, "show version" }, { "buildconf", OPT_EXIT, { .func_arg = show_buildconf }, "show build configuration" }, { "formats", OPT_EXIT, { .func_arg = show_formats }, "show available formats" }, { "muxers", OPT_EXIT, { .func_arg = show_muxers }, "show available muxers" }, { "demuxers", OPT_EXIT, { .func_arg = show_demuxers }, "show available demuxers" }, { "devices", OPT_EXIT, { .func_arg = show_devices }, "show available devices" }, { "codecs", OPT_EXIT, { .func_arg = show_codecs }, "show available codecs" }, { "decoders", OPT_EXIT, { .func_arg = show_decoders }, "show available decoders" }, { "encoders", OPT_EXIT, { .func_arg = show_encoders }, "show available encoders" }, { "bsfs", OPT_EXIT, { .func_arg = show_bsfs }, "show available bit stream filters" }, { "protocols", OPT_EXIT, { .func_arg = show_protocols }, "show available protocols" }, { "filters", OPT_EXIT, { .func_arg = show_filters }, "show available filters" }, { "pix_fmts", OPT_EXIT, { .func_arg = show_pix_fmts }, "show available pixel formats" }, { "layouts", OPT_EXIT, { .func_arg = show_layouts }, "show standard channel layouts" }, { "sample_fmts", OPT_EXIT, { .func_arg = show_sample_fmts }, "show available audio sample formats" }, { "colors", OPT_EXIT, { .func_arg = show_colors }, "show available color names" }, { "loglevel", HAS_ARG, { .func_arg = opt_loglevel }, "set logging level", "loglevel" }, { "v", HAS_ARG, { .func_arg = opt_loglevel }, "set logging level", "loglevel" }, { "report", 0, { .func_arg = opt_report }, "generate a report" }, { "max_alloc", HAS_ARG, { .func_arg = opt_max_alloc }, "set maximum size of a single allocated block", "bytes" }, { "cpuflags", HAS_ARG | OPT_EXPERT, { .func_arg = opt_cpuflags }, "force specific cpu flags", "flags" }, { "hide_banner", OPT_BOOL | OPT_EXPERT, {&hide_banner}, "do not show program banner", "hide_banner" }, CMDUTILS_COMMON_OPTIONS_AVDEVICE 
void show_help_children(const AVClass *class, int flags);
void show_help_default(const char *opt, const char *arg);
int show_help(void *optctx, const char *opt, const char *arg);
void parse_options(void *optctx, int argc, char **argv, const OptionDef *options,
void (* parse_arg_function)(void *optctx, const char*));
int parse_option(void *optctx, const char *opt, const char *arg,
const OptionDef *options);
typedef struct Option {
const OptionDef *opt;
const char *key;
const char *val;
} Option;
typedef struct OptionGroupDef {
const char *name;
const char *sep;
int flags;
} OptionGroupDef;
typedef struct OptionGroup {
const OptionGroupDef *group_def;
const char *arg;
Option *opts;
int nb_opts;
AVDictionary *codec_opts;
AVDictionary *format_opts;
AVDictionary *resample_opts;
AVDictionary *sws_dict;
AVDictionary *swr_opts;
} OptionGroup;
typedef struct OptionGroupList {
const OptionGroupDef *group_def;
OptionGroup *groups;
int nb_groups;
} OptionGroupList;
typedef struct OptionParseContext {
OptionGroup global_opts;
OptionGroupList *groups;
int nb_groups;
OptionGroup cur_group;
} OptionParseContext;
int parse_optgroup(void *optctx, OptionGroup *g);
int split_commandline(OptionParseContext *octx, int argc, char *argv[],
const OptionDef *options,
const OptionGroupDef *groups, int nb_groups);
void uninit_parse_context(OptionParseContext *octx);
void parse_loglevel(int argc, char **argv, const OptionDef *options);
int locate_option(int argc, char **argv, const OptionDef *options,
const char *optname);
int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec);
AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
AVFormatContext *s, AVStream *st, AVCodec *codec);
AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
AVDictionary *codec_opts);
void print_error(const char *filename, int err);
void show_banner(int argc, char **argv, const OptionDef *options);
int show_version(void *optctx, const char *opt, const char *arg);
int show_buildconf(void *optctx, const char *opt, const char *arg);
int show_license(void *optctx, const char *opt, const char *arg);
int show_formats(void *optctx, const char *opt, const char *arg);
int show_muxers(void *optctx, const char *opt, const char *arg);
int show_demuxers(void *optctx, const char *opt, const char *arg);
int show_devices(void *optctx, const char *opt, const char *arg);
#if CONFIG_AVDEVICE
int show_sinks(void *optctx, const char *opt, const char *arg);
int show_sources(void *optctx, const char *opt, const char *arg);
#endif
int show_codecs(void *optctx, const char *opt, const char *arg);
int show_decoders(void *optctx, const char *opt, const char *arg);
int show_encoders(void *optctx, const char *opt, const char *arg);
int show_filters(void *optctx, const char *opt, const char *arg);
int show_bsfs(void *optctx, const char *opt, const char *arg);
int show_protocols(void *optctx, const char *opt, const char *arg);
int show_pix_fmts(void *optctx, const char *opt, const char *arg);
int show_layouts(void *optctx, const char *opt, const char *arg);
int show_sample_fmts(void *optctx, const char *opt, const char *arg);
int show_colors(void *optctx, const char *opt, const char *arg);
int read_yesno(void);
FILE *get_preset_file(char *filename, size_t filename_size,
const char *preset_name, int is_path, const char *codec_name);
void *grow_array(void *array, int elem_size, int *size, int new_size);
#define media_type_string av_get_media_type_string
#define GROW_ARRAY(array, nb_elems)array = grow_array(array, sizeof(*array), &nb_elems, nb_elems + 1)
#define GET_PIX_FMT_NAME(pix_fmt)const char *name = av_get_pix_fmt_name(pix_fmt);
#define GET_CODEC_NAME(id)const char *name = avcodec_descriptor_get(id)->name;
#define GET_SAMPLE_FMT_NAME(sample_fmt)const char *name = av_get_sample_fmt_name(sample_fmt)
#define GET_SAMPLE_RATE_NAME(rate)char name[16];snprintf(name, sizeof(name), "%d", rate);
#define GET_CH_LAYOUT_NAME(ch_layout)char name[16];snprintf(name, sizeof(name), "0x%"PRIx64, ch_layout);
#define GET_CH_LAYOUT_DESC(ch_layout)char name[128];av_get_channel_layout_string(name, sizeof(name), 0, ch_layout);
double get_rotation(AVStream *st);
