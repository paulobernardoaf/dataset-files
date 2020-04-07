#include <stddef.h>
#include <stdint.h>
#include "libavutil/buffer.h"
#include "avcodec.h"
struct CodedBitstreamType;
typedef uint32_t CodedBitstreamUnitType;
typedef struct CodedBitstreamUnit {
CodedBitstreamUnitType type;
uint8_t *data;
size_t data_size;
size_t data_bit_padding;
AVBufferRef *data_ref;
void *content;
AVBufferRef *content_ref;
} CodedBitstreamUnit;
typedef struct CodedBitstreamFragment {
uint8_t *data;
size_t data_size;
size_t data_bit_padding;
AVBufferRef *data_ref;
int nb_units;
int nb_units_allocated;
CodedBitstreamUnit *units;
} CodedBitstreamFragment;
typedef struct CodedBitstreamContext {
void *log_ctx;
const struct CodedBitstreamType *codec;
void *priv_data;
CodedBitstreamUnitType *decompose_unit_types;
int nb_decompose_unit_types;
int trace_enable;
int trace_level;
uint8_t *write_buffer;
size_t write_buffer_size;
} CodedBitstreamContext;
extern const enum AVCodecID ff_cbs_all_codec_ids[];
int ff_cbs_init(CodedBitstreamContext **ctx,
enum AVCodecID codec_id, void *log_ctx);
void ff_cbs_close(CodedBitstreamContext **ctx);
int ff_cbs_read_extradata(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag,
const AVCodecParameters *par);
int ff_cbs_read_packet(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag,
const AVPacket *pkt);
int ff_cbs_read(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag,
const uint8_t *data, size_t size);
int ff_cbs_write_fragment_data(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag);
int ff_cbs_write_extradata(CodedBitstreamContext *ctx,
AVCodecParameters *par,
CodedBitstreamFragment *frag);
int ff_cbs_write_packet(CodedBitstreamContext *ctx,
AVPacket *pkt,
CodedBitstreamFragment *frag);
void ff_cbs_fragment_reset(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag);
void ff_cbs_fragment_free(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag);
int ff_cbs_alloc_unit_content(CodedBitstreamContext *ctx,
CodedBitstreamUnit *unit,
size_t size,
void (*free)(void *opaque, uint8_t *content));
int ff_cbs_alloc_unit_data(CodedBitstreamContext *ctx,
CodedBitstreamUnit *unit,
size_t size);
int ff_cbs_insert_unit_content(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag,
int position,
CodedBitstreamUnitType type,
void *content,
AVBufferRef *content_buf);
int ff_cbs_insert_unit_data(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag,
int position,
CodedBitstreamUnitType type,
uint8_t *data, size_t data_size,
AVBufferRef *data_buf);
void ff_cbs_delete_unit(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag,
int position);
