






















#if !defined(AVFORMAT_ISOM_H)
#define AVFORMAT_ISOM_H

#include <stddef.h>
#include <stdint.h>

#include "libavutil/encryption_info.h"
#include "libavutil/mastering_display_metadata.h"
#include "libavutil/spherical.h"
#include "libavutil/stereo3d.h"

#include "avio.h"
#include "internal.h"
#include "dv.h"


extern const AVCodecTag ff_mp4_obj_type[];
extern const AVCodecTag ff_codec_movvideo_tags[];
extern const AVCodecTag ff_codec_movaudio_tags[];
extern const AVCodecTag ff_codec_movsubtitle_tags[];
extern const AVCodecTag ff_codec_movdata_tags[];

int ff_mov_iso639_to_lang(const char lang[4], int mp4);
int ff_mov_lang_to_iso639(unsigned code, char to[4]);

struct AVAESCTR;






typedef struct MOVStts {
unsigned int count;
int duration;
} MOVStts;

typedef struct MOVStsc {
int first;
int count;
int id;
} MOVStsc;

typedef struct MOVElst {
int64_t duration;
int64_t time;
float rate;
} MOVElst;

typedef struct MOVDref {
uint32_t type;
char *path;
char *dir;
char volume[28];
char filename[64];
int16_t nlvl_to, nlvl_from;
} MOVDref;

typedef struct MOVAtom {
uint32_t type;
int64_t size; 
} MOVAtom;

struct MOVParseTableEntry;

typedef struct MOVFragment {
int found_tfhd;
unsigned track_id;
uint64_t base_data_offset;
uint64_t moof_offset;
uint64_t implicit_offset;
unsigned stsd_id;
unsigned duration;
unsigned size;
unsigned flags;
} MOVFragment;

typedef struct MOVTrackExt {
unsigned track_id;
unsigned stsd_id;
unsigned duration;
unsigned size;
unsigned flags;
} MOVTrackExt;

typedef struct MOVSbgp {
unsigned int count;
unsigned int index;
} MOVSbgp;

typedef struct MOVEncryptionIndex {


unsigned int nb_encrypted_samples;
AVEncryptionInfo **encrypted_samples;

uint8_t* auxiliary_info_sizes;
size_t auxiliary_info_sample_count;
uint8_t auxiliary_info_default_size;
uint64_t* auxiliary_offsets; 
size_t auxiliary_offsets_count;
} MOVEncryptionIndex;

typedef struct MOVFragmentStreamInfo {
int id;
int64_t sidx_pts;
int64_t first_tfra_pts;
int64_t tfdt_dts;
int index_entry;
MOVEncryptionIndex *encryption_index;
} MOVFragmentStreamInfo;

typedef struct MOVFragmentIndexItem {
int64_t moof_offset;
int headers_read;
int current;
int nb_stream_info;
MOVFragmentStreamInfo * stream_info;
} MOVFragmentIndexItem;

typedef struct MOVFragmentIndex {
int allocated_size;
int complete;
int current;
int nb_items;
MOVFragmentIndexItem * item;
} MOVFragmentIndex;

typedef struct MOVIndexRange {
int64_t start;
int64_t end;
} MOVIndexRange;

typedef struct MOVStreamContext {
AVIOContext *pb;
int pb_is_copied;
int ffindex; 
int next_chunk;
unsigned int chunk_count;
int64_t *chunk_offsets;
unsigned int stts_count;
MOVStts *stts_data;
unsigned int sdtp_count;
uint8_t *sdtp_data;
unsigned int ctts_count;
unsigned int ctts_allocated_size;
MOVStts *ctts_data;
unsigned int stsc_count;
MOVStsc *stsc_data;
unsigned int stsc_index;
int stsc_sample;
unsigned int stps_count;
unsigned *stps_data; 
MOVElst *elst_data;
unsigned int elst_count;
int ctts_index;
int ctts_sample;
unsigned int sample_size; 
unsigned int stsz_sample_size; 
unsigned int sample_count;
int *sample_sizes;
int keyframe_absent;
unsigned int keyframe_count;
int *keyframes;
int time_scale;
int64_t time_offset; 
int64_t min_corrected_pts; 
int current_sample;
int64_t current_index;
MOVIndexRange* index_ranges;
MOVIndexRange* current_index_range;
unsigned int bytes_per_frame;
unsigned int samples_per_frame;
int dv_audio_container;
int pseudo_stream_id; 
int16_t audio_cid; 
unsigned drefs_count;
MOVDref *drefs;
int dref_id;
int timecode_track;
int width; 
int height; 
int dts_shift; 
uint32_t palette[256];
int has_palette;
int64_t data_size;
uint32_t tmcd_flags; 
int64_t track_end; 
int start_pad; 
unsigned int rap_group_count;
MOVSbgp *rap_group;

int nb_frames_for_fps;
int64_t duration_for_fps;


uint8_t **extradata;
int *extradata_size;
int last_stsd_index;
int stsd_count;
int stsd_version;

int32_t *display_matrix;
AVStereo3D *stereo3d;
AVSphericalMapping *spherical;
size_t spherical_size;
AVMasteringDisplayMetadata *mastering;
AVContentLightMetadata *coll;
size_t coll_size;

uint32_t format;

int has_sidx; 
struct {
struct AVAESCTR* aes_ctr;
unsigned int per_sample_iv_size; 
AVEncryptionInfo *default_encrypted_sample;
MOVEncryptionIndex *encryption_index;
} cenc;
} MOVStreamContext;

typedef struct MOVContext {
const AVClass *class; 
AVFormatContext *fc;
int time_scale;
int64_t duration; 
int found_moov; 
int found_mdat; 
int found_hdlr_mdta; 
int trak_index; 
char **meta_keys;
unsigned meta_keys_count;
DVDemuxContext *dv_demux;
AVFormatContext *dv_fctx;
int isom; 
MOVFragment fragment; 
MOVTrackExt *trex_data;
unsigned trex_count;
int itunes_metadata; 
int handbrake_version;
int *chapter_tracks;
unsigned int nb_chapter_tracks;
int use_absolute_path;
int ignore_editlist;
int advanced_editlist;
int ignore_chapters;
int seek_individually;
int64_t next_root_atom; 
int export_all;
int export_xmp;
int *bitrates; 
int bitrates_count;
int moov_retry;
int use_mfra_for;
int has_looked_for_mfra;
MOVFragmentIndex frag_index;
int atom_depth;
unsigned int aax_mode; 
uint8_t file_key[20];
uint8_t file_iv[20];
void *activation_bytes;
int activation_bytes_size;
void *audible_fixed_key;
int audible_fixed_key_size;
struct AVAES *aes_decrypt;
uint8_t *decryption_key;
int decryption_key_len;
int enable_drefs;
int32_t movie_display_matrix[3][3]; 
} MOVContext;

int ff_mp4_read_descr_len(AVIOContext *pb);
int ff_mp4_read_descr(AVFormatContext *fc, AVIOContext *pb, int *tag);
int ff_mp4_read_dec_config_descr(AVFormatContext *fc, AVStream *st, AVIOContext *pb);
void ff_mp4_parse_es_descr(AVIOContext *pb, int *es_id);

#define MP4ODescrTag 0x01
#define MP4IODescrTag 0x02
#define MP4ESDescrTag 0x03
#define MP4DecConfigDescrTag 0x04
#define MP4DecSpecificDescrTag 0x05
#define MP4SLDescrTag 0x06

#define MOV_TFHD_BASE_DATA_OFFSET 0x01
#define MOV_TFHD_STSD_ID 0x02
#define MOV_TFHD_DEFAULT_DURATION 0x08
#define MOV_TFHD_DEFAULT_SIZE 0x10
#define MOV_TFHD_DEFAULT_FLAGS 0x20
#define MOV_TFHD_DURATION_IS_EMPTY 0x010000
#define MOV_TFHD_DEFAULT_BASE_IS_MOOF 0x020000

#define MOV_TRUN_DATA_OFFSET 0x01
#define MOV_TRUN_FIRST_SAMPLE_FLAGS 0x04
#define MOV_TRUN_SAMPLE_DURATION 0x100
#define MOV_TRUN_SAMPLE_SIZE 0x200
#define MOV_TRUN_SAMPLE_FLAGS 0x400
#define MOV_TRUN_SAMPLE_CTS 0x800

#define MOV_FRAG_SAMPLE_FLAG_DEGRADATION_PRIORITY_MASK 0x0000ffff
#define MOV_FRAG_SAMPLE_FLAG_IS_NON_SYNC 0x00010000
#define MOV_FRAG_SAMPLE_FLAG_PADDING_MASK 0x000e0000
#define MOV_FRAG_SAMPLE_FLAG_REDUNDANCY_MASK 0x00300000
#define MOV_FRAG_SAMPLE_FLAG_DEPENDED_MASK 0x00c00000
#define MOV_FRAG_SAMPLE_FLAG_DEPENDS_MASK 0x03000000

#define MOV_FRAG_SAMPLE_FLAG_DEPENDS_NO 0x02000000
#define MOV_FRAG_SAMPLE_FLAG_DEPENDS_YES 0x01000000

#define MOV_TKHD_FLAG_ENABLED 0x0001
#define MOV_TKHD_FLAG_IN_MOVIE 0x0002
#define MOV_TKHD_FLAG_IN_PREVIEW 0x0004
#define MOV_TKHD_FLAG_IN_POSTER 0x0008

#define MOV_SAMPLE_DEPENDENCY_UNKNOWN 0x0
#define MOV_SAMPLE_DEPENDENCY_YES 0x1
#define MOV_SAMPLE_DEPENDENCY_NO 0x2


#define TAG_IS_AVCI(tag) ((tag) == MKTAG('a', 'i', '5', 'p') || (tag) == MKTAG('a', 'i', '5', 'q') || (tag) == MKTAG('a', 'i', '5', '2') || (tag) == MKTAG('a', 'i', '5', '3') || (tag) == MKTAG('a', 'i', '5', '5') || (tag) == MKTAG('a', 'i', '5', '6') || (tag) == MKTAG('a', 'i', '1', 'p') || (tag) == MKTAG('a', 'i', '1', 'q') || (tag) == MKTAG('a', 'i', '1', '2') || (tag) == MKTAG('a', 'i', '1', '3') || (tag) == MKTAG('a', 'i', '1', '5') || (tag) == MKTAG('a', 'i', '1', '6') || (tag) == MKTAG('a', 'i', 'v', 'x') || (tag) == MKTAG('A', 'V', 'i', 'n'))
















int ff_mov_read_esds(AVFormatContext *fc, AVIOContext *pb);

int ff_mov_read_stsd_entries(MOVContext *c, AVIOContext *pb, int entries);
void ff_mov_write_chan(AVIOContext *pb, int64_t channel_layout);

#define FF_MOV_FLAG_MFRA_AUTO -1
#define FF_MOV_FLAG_MFRA_DTS 1
#define FF_MOV_FLAG_MFRA_PTS 2





static inline enum AVCodecID ff_mov_get_lpcm_codec_id(int bps, int flags)
{





return ff_get_pcm_codec_id(bps, flags & 1, flags & 2, flags & 4 ? -1 : 0);
}

#endif 
