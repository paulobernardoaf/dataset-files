#include "avcodec.h"
#include "cbs.h"
#include "get_bits.h"
#include "put_bits.h"
typedef struct CodedBitstreamType {
enum AVCodecID codec_id;
size_t priv_data_size;
int (*split_fragment)(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag,
int header);
int (*read_unit)(CodedBitstreamContext *ctx,
CodedBitstreamUnit *unit);
int (*write_unit)(CodedBitstreamContext *ctx,
CodedBitstreamUnit *unit,
PutBitContext *pbc);
int (*assemble_fragment)(CodedBitstreamContext *ctx,
CodedBitstreamFragment *frag);
void (*close)(CodedBitstreamContext *ctx);
} CodedBitstreamType;
void ff_cbs_trace_header(CodedBitstreamContext *ctx,
const char *name);
void ff_cbs_trace_syntax_element(CodedBitstreamContext *ctx, int position,
const char *name, const int *subscripts,
const char *bitstring, int64_t value);
int ff_cbs_read_unsigned(CodedBitstreamContext *ctx, GetBitContext *gbc,
int width, const char *name,
const int *subscripts, uint32_t *write_to,
uint32_t range_min, uint32_t range_max);
int ff_cbs_write_unsigned(CodedBitstreamContext *ctx, PutBitContext *pbc,
int width, const char *name,
const int *subscripts, uint32_t value,
uint32_t range_min, uint32_t range_max);
int ff_cbs_read_signed(CodedBitstreamContext *ctx, GetBitContext *gbc,
int width, const char *name,
const int *subscripts, int32_t *write_to,
int32_t range_min, int32_t range_max);
int ff_cbs_write_signed(CodedBitstreamContext *ctx, PutBitContext *pbc,
int width, const char *name,
const int *subscripts, int32_t value,
int32_t range_min, int32_t range_max);
#define MAX_UINT_BITS(length) ((UINT64_C(1) << (length)) - 1)
#define MAX_INT_BITS(length) ((INT64_C(1) << ((length) - 1)) - 1)
#define MIN_INT_BITS(length) (-(INT64_C(1) << ((length) - 1)))
extern const CodedBitstreamType ff_cbs_type_av1;
extern const CodedBitstreamType ff_cbs_type_h264;
extern const CodedBitstreamType ff_cbs_type_h265;
extern const CodedBitstreamType ff_cbs_type_jpeg;
extern const CodedBitstreamType ff_cbs_type_mpeg2;
extern const CodedBitstreamType ff_cbs_type_vp9;
