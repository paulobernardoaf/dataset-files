#include "libavutil/aes_ctr.h"
#include "avformat.h"
#include "avio.h"
#define CENC_KID_SIZE (16)
struct MOVTrack;
typedef struct {
struct AVAESCTR* aes_ctr;
uint8_t* auxiliary_info;
size_t auxiliary_info_size;
size_t auxiliary_info_alloc_size;
uint32_t auxiliary_info_entries;
int use_subsamples;
uint16_t subsample_count;
size_t auxiliary_info_subsample_start;
uint8_t* auxiliary_info_sizes;
size_t auxiliary_info_sizes_alloc_size;
} MOVMuxCencContext;
int ff_mov_cenc_init(MOVMuxCencContext* ctx, uint8_t* encryption_key, int use_subsamples, int bitexact);
void ff_mov_cenc_free(MOVMuxCencContext* ctx);
int ff_mov_cenc_write_packet(MOVMuxCencContext* ctx, AVIOContext *pb, const uint8_t *buf_in, int size);
int ff_mov_cenc_avc_parse_nal_units(MOVMuxCencContext* ctx, AVIOContext *pb, const uint8_t *buf_in, int size);
int ff_mov_cenc_avc_write_nal_units(AVFormatContext *s, MOVMuxCencContext* ctx, int nal_length_size,
AVIOContext *pb, const uint8_t *buf_in, int size);
void ff_mov_cenc_write_stbl_atoms(MOVMuxCencContext* ctx, AVIOContext *pb);
int ff_mov_cenc_write_sinf_tag(struct MOVTrack* track, AVIOContext *pb, uint8_t* kid);
