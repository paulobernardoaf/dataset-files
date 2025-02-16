





























































#if !defined(STB_VORBIS_INCLUDE_STB_VORBIS_H)
#define STB_VORBIS_INCLUDE_STB_VORBIS_H

#if defined(STB_VORBIS_NO_CRT) && !defined(STB_VORBIS_NO_STDIO)
#define STB_VORBIS_NO_STDIO
#endif

#if !defined(STB_VORBIS_NO_STDIO)
#include <stdio.h>
#endif


#if defined(PLATFORM_ANDROID)
#include "utils.h" 
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(STB_VORBIS_STATIC)
#define STBVDEF static
#else
#define STBVDEF extern
#endif






























typedef struct
{
char *alloc_buffer;
int alloc_buffer_length_in_bytes;
} stb_vorbis_alloc;




typedef struct stb_vorbis stb_vorbis;

typedef struct
{
unsigned int sample_rate;
int channels;

unsigned int setup_memory_required;
unsigned int setup_temp_memory_required;
unsigned int temp_memory_required;

int max_frame_size;
} stb_vorbis_info;


STBVDEF stb_vorbis_info stb_vorbis_get_info(stb_vorbis *f);


STBVDEF int stb_vorbis_get_error(stb_vorbis *f);


STBVDEF void stb_vorbis_close(stb_vorbis *f);






STBVDEF int stb_vorbis_get_sample_offset(stb_vorbis *f);



STBVDEF unsigned int stb_vorbis_get_file_offset(stb_vorbis *f);



#if !defined(STB_VORBIS_NO_PUSHDATA_API)








STBVDEF stb_vorbis *stb_vorbis_open_pushdata(
const unsigned char * datablock, int datablock_length_in_bytes,
int *datablock_memory_consumed_in_bytes,
int *error,
const stb_vorbis_alloc *alloc_buffer);









STBVDEF int stb_vorbis_decode_frame_pushdata(
stb_vorbis *f,
const unsigned char *datablock, int datablock_length_in_bytes,
int *channels, 
float ***output, 
int *samples 
);























STBVDEF void stb_vorbis_flush_pushdata(stb_vorbis *f);










#endif




#if !defined(STB_VORBIS_NO_PULLDATA_API)







#if !defined(STB_VORBIS_NO_STDIO) && !defined(STB_VORBIS_NO_INTEGER_CONVERSION)
STBVDEF int stb_vorbis_decode_filename(const char *filename, int *channels, int *sample_rate, short **output);
#endif
#if !defined(STB_VORBIS_NO_INTEGER_CONVERSION)
STBVDEF int stb_vorbis_decode_memory(const unsigned char *mem, int len, int *channels, int *sample_rate, short **output);
#endif





STBVDEF stb_vorbis * stb_vorbis_open_memory(const unsigned char *data, int len,
int *error, const stb_vorbis_alloc *alloc_buffer);



#if !defined(STB_VORBIS_NO_STDIO)
STBVDEF stb_vorbis * stb_vorbis_open_filename(const char *filename,
int *error, const stb_vorbis_alloc *alloc_buffer);



STBVDEF stb_vorbis * stb_vorbis_open_file(FILE *f, int close_handle_on_close,
int *error, const stb_vorbis_alloc *alloc_buffer);








STBVDEF stb_vorbis * stb_vorbis_open_file_section(FILE *f, int close_handle_on_close,
int *error, const stb_vorbis_alloc *alloc_buffer, unsigned int len);





#endif

STBVDEF int stb_vorbis_seek_frame(stb_vorbis *f, unsigned int sample_number);
STBVDEF int stb_vorbis_seek(stb_vorbis *f, unsigned int sample_number);







STBVDEF int stb_vorbis_seek_start(stb_vorbis *f);


STBVDEF unsigned int stb_vorbis_stream_length_in_samples(stb_vorbis *f);
STBVDEF float stb_vorbis_stream_length_in_seconds(stb_vorbis *f);


STBVDEF int stb_vorbis_get_frame_float(stb_vorbis *f, int *channels, float ***output);









#if !defined(STB_VORBIS_NO_INTEGER_CONVERSION)
STBVDEF int stb_vorbis_get_frame_short_interleaved(stb_vorbis *f, int num_c, short *buffer, int num_shorts);
STBVDEF int stb_vorbis_get_frame_short (stb_vorbis *f, int num_c, short **buffer, int num_samples);
#endif

























STBVDEF int stb_vorbis_get_samples_float_interleaved(stb_vorbis *f, int channels, float *buffer, int num_floats);
STBVDEF int stb_vorbis_get_samples_float(stb_vorbis *f, int channels, float **buffer, int num_samples);





#if !defined(STB_VORBIS_NO_INTEGER_CONVERSION)
STBVDEF int stb_vorbis_get_samples_short_interleaved(stb_vorbis *f, int channels, short *buffer, int num_shorts);
STBVDEF int stb_vorbis_get_samples_short(stb_vorbis *f, int channels, short **buffer, int num_samples);
#endif






#endif



enum STBVorbisError
{
VORBIS__no_error,

VORBIS_need_more_data=1, 

VORBIS_invalid_api_mixing, 
VORBIS_outofmem, 
VORBIS_feature_not_supported, 
VORBIS_too_many_channels, 
VORBIS_file_open_failure, 
VORBIS_seek_without_length, 

VORBIS_unexpected_eof=10, 
VORBIS_seek_invalid, 





VORBIS_invalid_setup=20,
VORBIS_invalid_stream,


VORBIS_missing_capture_pattern=30,
VORBIS_invalid_stream_structure_version,
VORBIS_continued_packet_flag_invalid,
VORBIS_incorrect_stream_serial_number,
VORBIS_invalid_first_page,
VORBIS_bad_packet_type,
VORBIS_cant_find_last_page,
VORBIS_seek_failed
};


#if defined(__cplusplus)
}
#endif

#endif 





#if defined(STB_VORBIS_IMPLEMENTATION)







































#if !defined(STB_VORBIS_MAX_CHANNELS)
#define STB_VORBIS_MAX_CHANNELS 16 
#endif













#if !defined(STB_VORBIS_PUSHDATA_CRC_COUNT)
#define STB_VORBIS_PUSHDATA_CRC_COUNT 4
#endif






#if !defined(STB_VORBIS_FAST_HUFFMAN_LENGTH)
#define STB_VORBIS_FAST_HUFFMAN_LENGTH 10
#endif











#if !defined(STB_VORBIS_FAST_HUFFMAN_INT)
#define STB_VORBIS_FAST_HUFFMAN_SHORT
#endif

























#if defined(STB_VORBIS_CODEBOOK_SHORTS)
#error "STB_VORBIS_CODEBOOK_SHORTS is no longer supported as it produced incorrect results for some input formats"
#endif






















#if defined(STB_VORBIS_NO_PULLDATA_API)
#define STB_VORBIS_NO_INTEGER_CONVERSION
#define STB_VORBIS_NO_STDIO
#endif

#if defined(STB_VORBIS_NO_CRT) && !defined(STB_VORBIS_NO_STDIO)
#define STB_VORBIS_NO_STDIO 1
#endif

#if !defined(STB_VORBIS_NO_INTEGER_CONVERSION)
#if !defined(STB_VORBIS_NO_FAST_SCALED_FLOAT)




#if !defined(STB_VORBIS_BIG_ENDIAN)
#define STB_VORBIS_ENDIAN 0
#else
#define STB_VORBIS_ENDIAN 1
#endif

#endif
#endif


#if !defined(STB_VORBIS_NO_STDIO)
#include <stdio.h>
#endif

#if !defined(STB_VORBIS_NO_CRT)
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>


#if defined(_MSC_VER) || defined(__MINGW32__)
#include <malloc.h>
#endif
#if defined(__linux__) || defined(__linux) || defined(__EMSCRIPTEN__)
#include <alloca.h>
#endif
#else 
#define NULL 0
#define malloc(s) 0
#define free(s) ((void) 0)
#define realloc(s) 0
#endif 

#include <limits.h>

#if defined(__MINGW32__)





#if defined(__forceinline)
#undef __forceinline
#endif
#define __forceinline
#if !defined(alloca)
#define alloca(s) __builtin_alloca(s)
#endif
#elif !defined(_MSC_VER)
#if __GNUC__
#define __forceinline inline
#else
#define __forceinline
#endif
#endif

#if STB_VORBIS_MAX_CHANNELS > 256
#error "Value of STB_VORBIS_MAX_CHANNELS outside of allowed range"
#endif

#if STB_VORBIS_FAST_HUFFMAN_LENGTH > 24
#error "Value of STB_VORBIS_FAST_HUFFMAN_LENGTH outside of allowed range"
#endif


#if 0
#include <crtdbg.h>
#define STBV_CHECK(f) _CrtIsValidHeapPointer(f->channel_buffers[1])
#else
#define STBV_CHECK(f) ((void) 0)
#endif

#define STBV_MAX_BLOCKSIZE_LOG 13 
#define STBV_MAX_BLOCKSIZE (1 << STBV_MAX_BLOCKSIZE_LOG)


typedef unsigned char stbv_uint8;
typedef signed char stbv_int8;
typedef unsigned short stbv_uint16;
typedef signed short stbv_int16;
typedef unsigned int stbv_uint32;
typedef signed int stbv_int32;

#if !defined(TRUE)
#define TRUE 1
#define FALSE 0
#endif

typedef float stbv_codetype;














#define STBV_FAST_HUFFMAN_TABLE_SIZE (1 << STB_VORBIS_FAST_HUFFMAN_LENGTH)
#define STBV_FAST_HUFFMAN_TABLE_MASK (STBV_FAST_HUFFMAN_TABLE_SIZE - 1)

typedef struct
{
int dimensions, entries;
stbv_uint8 *codeword_lengths;
float minimum_value;
float delta_value;
stbv_uint8 value_bits;
stbv_uint8 lookup_type;
stbv_uint8 sequence_p;
stbv_uint8 sparse;
stbv_uint32 lookup_values;
stbv_codetype *multiplicands;
stbv_uint32 *codewords;
#if defined(STB_VORBIS_FAST_HUFFMAN_SHORT)
stbv_int16 fast_huffman[STBV_FAST_HUFFMAN_TABLE_SIZE];
#else
stbv_int32 fast_huffman[STBV_FAST_HUFFMAN_TABLE_SIZE];
#endif
stbv_uint32 *sorted_codewords;
int *sorted_values;
int sorted_entries;
} StbvCodebook;

typedef struct
{
stbv_uint8 order;
stbv_uint16 rate;
stbv_uint16 bark_map_size;
stbv_uint8 amplitude_bits;
stbv_uint8 amplitude_offset;
stbv_uint8 number_of_books;
stbv_uint8 book_list[16]; 
} StbvFloor0;

typedef struct
{
stbv_uint8 partitions;
stbv_uint8 partition_class_list[32]; 
stbv_uint8 class_dimensions[16]; 
stbv_uint8 class_subclasses[16]; 
stbv_uint8 class_masterbooks[16]; 
stbv_int16 subclass_books[16][8]; 
stbv_uint16 Xlist[31*8+2]; 
stbv_uint8 sorted_order[31*8+2];
stbv_uint8 stbv_neighbors[31*8+2][2];
stbv_uint8 floor1_multiplier;
stbv_uint8 rangebits;
int values;
} StbvFloor1;

typedef union
{
StbvFloor0 floor0;
StbvFloor1 floor1;
} StbvFloor;

typedef struct
{
stbv_uint32 begin, end;
stbv_uint32 part_size;
stbv_uint8 classifications;
stbv_uint8 classbook;
stbv_uint8 **classdata;
stbv_int16 (*residue_books)[8];
} StbvResidue;

typedef struct
{
stbv_uint8 magnitude;
stbv_uint8 angle;
stbv_uint8 mux;
} StbvMappingChannel;

typedef struct
{
stbv_uint16 coupling_steps;
StbvMappingChannel *chan;
stbv_uint8 submaps;
stbv_uint8 submap_floor[15]; 
stbv_uint8 submap_residue[15]; 
} StbvMapping;

typedef struct
{
stbv_uint8 blockflag;
stbv_uint8 mapping;
stbv_uint16 windowtype;
stbv_uint16 transformtype;
} StbvMode;

typedef struct
{
stbv_uint32 goal_crc; 
int bytes_left; 
stbv_uint32 crc_so_far; 
int bytes_done; 
stbv_uint32 sample_loc; 
} StbvCRCscan;

typedef struct
{
stbv_uint32 page_start, page_end;
stbv_uint32 last_decoded_sample;
} StbvProbedPage;

struct stb_vorbis
{

unsigned int sample_rate;
int channels;

unsigned int setup_memory_required;
unsigned int temp_memory_required;
unsigned int setup_temp_memory_required;


#if !defined(STB_VORBIS_NO_STDIO)
FILE *f;
stbv_uint32 f_start;
int close_on_free;
#endif

stbv_uint8 *stream;
stbv_uint8 *stream_start;
stbv_uint8 *stream_end;

stbv_uint32 stream_len;

stbv_uint8 push_mode;

stbv_uint32 first_audio_page_offset;

StbvProbedPage p_first, p_last;


stb_vorbis_alloc alloc;
int setup_offset;
int temp_offset;


int eof;
enum STBVorbisError error;




int blocksize[2];
int blocksize_0, blocksize_1;
int codebook_count;
StbvCodebook *codebooks;
int floor_count;
stbv_uint16 floor_types[64]; 
StbvFloor *floor_config;
int residue_count;
stbv_uint16 residue_types[64]; 
StbvResidue *residue_config;
int mapping_count;
StbvMapping *mapping;
int mode_count;
StbvMode mode_config[64]; 

stbv_uint32 total_samples;


float *channel_buffers[STB_VORBIS_MAX_CHANNELS];
float *outputs [STB_VORBIS_MAX_CHANNELS];

float *previous_window[STB_VORBIS_MAX_CHANNELS];
int previous_length;

#if !defined(STB_VORBIS_NO_DEFER_FLOOR)
stbv_int16 *finalY[STB_VORBIS_MAX_CHANNELS];
#else
float *floor_buffers[STB_VORBIS_MAX_CHANNELS];
#endif

stbv_uint32 current_loc; 
int current_loc_valid;




float *A[2],*B[2],*C[2];
float *window[2];
stbv_uint16 *stbv_bit_reverse[2];


stbv_uint32 serial; 
int last_page;
int segment_count;
stbv_uint8 segments[255];
stbv_uint8 page_flag;
stbv_uint8 bytes_in_seg;
stbv_uint8 first_decode;
int next_seg;
int last_seg; 
int last_seg_which; 
stbv_uint32 acc;
int valid_bits;
int packet_bytes;
int end_seg_with_known_loc;
stbv_uint32 known_loc_for_packet;
int discard_samples_deferred;
stbv_uint32 samples_output;


int page_crc_tests; 
#if !defined(STB_VORBIS_NO_PUSHDATA_API)
StbvCRCscan scan[STB_VORBIS_PUSHDATA_CRC_COUNT];
#endif


int channel_buffer_start;
int channel_buffer_end;
};

#if defined(STB_VORBIS_NO_PUSHDATA_API)
#define STBV_IS_PUSH_MODE(f) FALSE
#elif defined(STB_VORBIS_NO_PULLDATA_API)
#define STBV_IS_PUSH_MODE(f) TRUE
#else
#define STBV_IS_PUSH_MODE(f) ((f)->push_mode)
#endif

typedef struct stb_vorbis stbv_vorb;

static int stbv_error(stbv_vorb *f, enum STBVorbisError e)
{
f->error = e;
if (!f->eof && e != VORBIS_need_more_data) {
f->error=e; 
}
return 0;
}







#define stbv_array_size_required(count,size) (count*(sizeof(void *)+(size)))

#define stbv_temp_alloc(f,size) (f->alloc.alloc_buffer ? stbv_setup_temp_malloc(f,size) : alloca(size))
#define stbv_temp_free(f,p) 0
#define stbv_temp_alloc_save(f) ((f)->temp_offset)
#define stbv_temp_alloc_restore(f,p) ((f)->temp_offset = (p))

#define stbv_temp_block_array(f,count,size) stbv_make_block_array(stbv_temp_alloc(f,stbv_array_size_required(count,size)), count, size)


static void *stbv_make_block_array(void *mem, int count, int size)
{
int i;
void ** p = (void **) mem;
char *q = (char *) (p + count);
for (i=0; i < count; ++i) {
p[i] = q;
q += size;
}
return p;
}

static void *stbv_setup_malloc(stbv_vorb *f, int sz)
{
sz = (sz+3) & ~3;
f->setup_memory_required += sz;
if (f->alloc.alloc_buffer) {
void *p = (char *) f->alloc.alloc_buffer + f->setup_offset;
if (f->setup_offset + sz > f->temp_offset) return NULL;
f->setup_offset += sz;
return p;
}
return sz ? malloc(sz) : NULL;
}

static void stbv_setup_free(stbv_vorb *f, void *p)
{
if (f->alloc.alloc_buffer) return; 
free(p);
}

static void *stbv_setup_temp_malloc(stbv_vorb *f, int sz)
{
sz = (sz+3) & ~3;
if (f->alloc.alloc_buffer) {
if (f->temp_offset - sz < f->setup_offset) return NULL;
f->temp_offset -= sz;
return (char *) f->alloc.alloc_buffer + f->temp_offset;
}
return malloc(sz);
}

static void stbv_setup_temp_free(stbv_vorb *f, void *p, int sz)
{
if (f->alloc.alloc_buffer) {
f->temp_offset += (sz+3)&~3;
return;
}
free(p);
}

#define STBV_CRC32_POLY 0x04c11db7 

static stbv_uint32 stbv_crc_table[256];
static void stbv_crc32_init(void)
{
int i,j;
stbv_uint32 s;
for(i=0; i < 256; i++) {
for (s=(stbv_uint32) i << 24, j=0; j < 8; ++j)
s = (s << 1) ^ (s >= (1U<<31) ? STBV_CRC32_POLY : 0);
stbv_crc_table[i] = s;
}
}

static __forceinline stbv_uint32 stbv_crc32_update(stbv_uint32 crc, stbv_uint8 byte)
{
return (crc << 8) ^ stbv_crc_table[byte ^ (crc >> 24)];
}



static unsigned int stbv_bit_reverse(unsigned int n)
{
n = ((n & 0xAAAAAAAA) >> 1) | ((n & 0x55555555) << 1);
n = ((n & 0xCCCCCCCC) >> 2) | ((n & 0x33333333) << 2);
n = ((n & 0xF0F0F0F0) >> 4) | ((n & 0x0F0F0F0F) << 4);
n = ((n & 0xFF00FF00) >> 8) | ((n & 0x00FF00FF) << 8);
return (n >> 16) | (n << 16);
}

static float stbv_square(float x)
{
return x*x;
}




static int stbv_ilog(stbv_int32 n)
{
static signed char log2_4[16] = { 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

if (n < 0) return 0; 


if (n < (1 << 14))
if (n < (1 << 4)) return 0 + log2_4[n ];
else if (n < (1 << 9)) return 5 + log2_4[n >> 5];
else return 10 + log2_4[n >> 10];
else if (n < (1 << 24))
if (n < (1 << 19)) return 15 + log2_4[n >> 15];
else return 20 + log2_4[n >> 20];
else if (n < (1 << 29)) return 25 + log2_4[n >> 25];
else return 30 + log2_4[n >> 30];
}

#if !defined(M_PI)
#define M_PI 3.14159265358979323846264f 
#endif


#define NO_CODE 255






static float stbv_float32_unpack(stbv_uint32 x)
{

stbv_uint32 mantissa = x & 0x1fffff;
stbv_uint32 sign = x & 0x80000000;
stbv_uint32 exp = (x & 0x7fe00000) >> 21;
double res = sign ? -(double)mantissa : (double)mantissa;
return (float) ldexp((float)res, exp-788);
}









static void stbv_add_entry(StbvCodebook *c, stbv_uint32 huff_code, int symbol, int count, int len, stbv_uint32 *values)
{
if (!c->sparse) {
c->codewords [symbol] = huff_code;
} else {
c->codewords [count] = huff_code;
c->codeword_lengths[count] = len;
values [count] = symbol;
}
}

static int stbv_compute_codewords(StbvCodebook *c, stbv_uint8 *len, int n, stbv_uint32 *values)
{
int i,k,m=0;
stbv_uint32 available[32];

memset(available, 0, sizeof(available));

for (k=0; k < n; ++k) if (len[k] < NO_CODE) break;
if (k == n) { assert(c->sorted_entries == 0); return TRUE; }

stbv_add_entry(c, 0, k, m++, len[k], values);

for (i=1; i <= len[k]; ++i)
available[i] = 1U << (32-i);




for (i=k+1; i < n; ++i) {
stbv_uint32 res;
int z = len[i], y;
if (z == NO_CODE) continue;






while (z > 0 && !available[z]) --z;
if (z == 0) { return FALSE; }
res = available[z];
assert(z >= 0 && z < 32);
available[z] = 0;
stbv_add_entry(c, stbv_bit_reverse(res), i, m++, len[i], values);

if (z != len[i]) {
assert(len[i] >= 0 && len[i] < 32);
for (y=len[i]; y > z; --y) {
assert(available[y] == 0);
available[y] = res + (1 << (32-y));
}
}
}
return TRUE;
}



static void stbv_compute_accelerated_huffman(StbvCodebook *c)
{
int i, len;
for (i=0; i < STBV_FAST_HUFFMAN_TABLE_SIZE; ++i)
c->fast_huffman[i] = -1;

len = c->sparse ? c->sorted_entries : c->entries;
#if defined(STB_VORBIS_FAST_HUFFMAN_SHORT)
if (len > 32767) len = 32767; 
#endif
for (i=0; i < len; ++i) {
if (c->codeword_lengths[i] <= STB_VORBIS_FAST_HUFFMAN_LENGTH) {
stbv_uint32 z = c->sparse ? stbv_bit_reverse(c->sorted_codewords[i]) : c->codewords[i];

while (z < STBV_FAST_HUFFMAN_TABLE_SIZE) {
c->fast_huffman[z] = i;
z += 1 << c->codeword_lengths[i];
}
}
}
}

#if defined(_MSC_VER)
#define STBV_CDECL __cdecl
#else
#define STBV_CDECL
#endif

static int STBV_CDECL stbv_uint32_compare(const void *p, const void *q)
{
stbv_uint32 x = * (stbv_uint32 *) p;
stbv_uint32 y = * (stbv_uint32 *) q;
return x < y ? -1 : x > y;
}

static int stbv_include_in_sort(StbvCodebook *c, stbv_uint8 len)
{
if (c->sparse) { assert(len != NO_CODE); return TRUE; }
if (len == NO_CODE) return FALSE;
if (len > STB_VORBIS_FAST_HUFFMAN_LENGTH) return TRUE;
return FALSE;
}



static void stbv_compute_sorted_huffman(StbvCodebook *c, stbv_uint8 *lengths, stbv_uint32 *values)
{
int i, len;




if (!c->sparse) {
int k = 0;
for (i=0; i < c->entries; ++i)
if (stbv_include_in_sort(c, lengths[i])) 
c->sorted_codewords[k++] = stbv_bit_reverse(c->codewords[i]);
assert(k == c->sorted_entries);
} else {
for (i=0; i < c->sorted_entries; ++i)
c->sorted_codewords[i] = stbv_bit_reverse(c->codewords[i]);
}

qsort(c->sorted_codewords, c->sorted_entries, sizeof(c->sorted_codewords[0]), stbv_uint32_compare);
c->sorted_codewords[c->sorted_entries] = 0xffffffff;

len = c->sparse ? c->sorted_entries : c->entries;





for (i=0; i < len; ++i) {
int huff_len = c->sparse ? lengths[values[i]] : lengths[i];
if (stbv_include_in_sort(c,huff_len)) {
stbv_uint32 code = stbv_bit_reverse(c->codewords[i]);
int x=0, n=c->sorted_entries;
while (n > 1) {

int m = x + (n >> 1);
if (c->sorted_codewords[m] <= code) {
x = m;
n -= (n>>1);
} else {
n >>= 1;
}
}
assert(c->sorted_codewords[x] == code);
if (c->sparse) {
c->sorted_values[x] = values[i];
c->codeword_lengths[x] = huff_len;
} else {
c->sorted_values[x] = i;
}
}
}
}


static int stbv_vorbis_validate(stbv_uint8 *data)
{
static stbv_uint8 vorbis[6] = { 'v', 'o', 'r', 'b', 'i', 's' };
return memcmp(data, vorbis, 6) == 0;
}



static int stbv_lookup1_values(int entries, int dim)
{
int r = (int) floor(exp((float) log((float) entries) / dim));
if ((int) floor(pow((float) r+1, dim)) <= entries) 
++r; 
assert(pow((float) r+1, dim) > entries);
assert((int) floor(pow((float) r, dim)) <= entries); 
return r;
}


static void stbv_compute_twiddle_factors(int n, float *A, float *B, float *C)
{
int n4 = n >> 2, n8 = n >> 3;
int k,k2;

for (k=k2=0; k < n4; ++k,k2+=2) {
A[k2 ] = (float) cos(4*k*M_PI/n);
A[k2+1] = (float) -sin(4*k*M_PI/n);
B[k2 ] = (float) cos((k2+1)*M_PI/n/2) * 0.5f;
B[k2+1] = (float) sin((k2+1)*M_PI/n/2) * 0.5f;
}
for (k=k2=0; k < n8; ++k,k2+=2) {
C[k2 ] = (float) cos(2*(k2+1)*M_PI/n);
C[k2+1] = (float) -sin(2*(k2+1)*M_PI/n);
}
}

static void stbv_compute_window(int n, float *window)
{
int n2 = n >> 1, i;
for (i=0; i < n2; ++i)
window[i] = (float) sin(0.5 * M_PI * stbv_square((float) sin((i - 0 + 0.5) / n2 * 0.5 * M_PI)));
}

static void stbv_compute_bitreverse(int n, stbv_uint16 *rev)
{
int ld = stbv_ilog(n) - 1; 
int i, n8 = n >> 3;
for (i=0; i < n8; ++i)
rev[i] = (stbv_bit_reverse(i) >> (32-ld+3)) << 2;
}

static int stbv_init_blocksize(stbv_vorb *f, int b, int n)
{
int n2 = n >> 1, n4 = n >> 2, n8 = n >> 3;
f->A[b] = (float *) stbv_setup_malloc(f, sizeof(float) * n2);
f->B[b] = (float *) stbv_setup_malloc(f, sizeof(float) * n2);
f->C[b] = (float *) stbv_setup_malloc(f, sizeof(float) * n4);
if (!f->A[b] || !f->B[b] || !f->C[b]) return stbv_error(f, VORBIS_outofmem);
stbv_compute_twiddle_factors(n, f->A[b], f->B[b], f->C[b]);
f->window[b] = (float *) stbv_setup_malloc(f, sizeof(float) * n2);
if (!f->window[b]) return stbv_error(f, VORBIS_outofmem);
stbv_compute_window(n, f->window[b]);
f->stbv_bit_reverse[b] = (stbv_uint16 *) stbv_setup_malloc(f, sizeof(stbv_uint16) * n8);
if (!f->stbv_bit_reverse[b]) return stbv_error(f, VORBIS_outofmem);
stbv_compute_bitreverse(n, f->stbv_bit_reverse[b]);
return TRUE;
}

static void stbv_neighbors(stbv_uint16 *x, int n, int *plow, int *phigh)
{
int low = -1;
int high = 65536;
int i;
for (i=0; i < n; ++i) {
if (x[i] > low && x[i] < x[n]) { *plow = i; low = x[i]; }
if (x[i] < high && x[i] > x[n]) { *phigh = i; high = x[i]; }
}
}


typedef struct
{
stbv_uint16 x,id;
} stbv_floor_ordering;

static int STBV_CDECL stbv_point_compare(const void *p, const void *q)
{
stbv_floor_ordering *a = (stbv_floor_ordering *) p;
stbv_floor_ordering *b = (stbv_floor_ordering *) q;
return a->x < b->x ? -1 : a->x > b->x;
}





#if defined(STB_VORBIS_NO_STDIO)
#define STBV_USE_MEMORY(z) TRUE
#else
#define STBV_USE_MEMORY(z) ((z)->stream)
#endif

static stbv_uint8 stbv_get8(stbv_vorb *z)
{
if (STBV_USE_MEMORY(z)) {
if (z->stream >= z->stream_end) { z->eof = TRUE; return 0; }
return *z->stream++;
}

#if !defined(STB_VORBIS_NO_STDIO)
{
int c = fgetc(z->f);
if (c == EOF) { z->eof = TRUE; return 0; }
return c;
}
#endif
}

static stbv_uint32 stbv_get32(stbv_vorb *f)
{
stbv_uint32 x;
x = stbv_get8(f);
x += stbv_get8(f) << 8;
x += stbv_get8(f) << 16;
x += (stbv_uint32) stbv_get8(f) << 24;
return x;
}

static int stbv_getn(stbv_vorb *z, stbv_uint8 *data, int n)
{
if (STBV_USE_MEMORY(z)) {
if (z->stream+n > z->stream_end) { z->eof = 1; return 0; }
memcpy(data, z->stream, n);
z->stream += n;
return 1;
}

#if !defined(STB_VORBIS_NO_STDIO)
if (fread(data, n, 1, z->f) == 1)
return 1;
else {
z->eof = 1;
return 0;
}
#endif
}

static void stbv_skip(stbv_vorb *z, int n)
{
if (STBV_USE_MEMORY(z)) {
z->stream += n;
if (z->stream >= z->stream_end) z->eof = 1;
return;
}
#if !defined(STB_VORBIS_NO_STDIO)
{
long x = ftell(z->f);
fseek(z->f, x+n, SEEK_SET);
}
#endif
}

static int stbv_set_file_offset(stb_vorbis *f, unsigned int loc)
{
#if !defined(STB_VORBIS_NO_PUSHDATA_API)
if (f->push_mode) return 0;
#endif
f->eof = 0;
if (STBV_USE_MEMORY(f)) {
if (f->stream_start + loc >= f->stream_end || f->stream_start + loc < f->stream_start) {
f->stream = f->stream_end;
f->eof = 1;
return 0;
} else {
f->stream = f->stream_start + loc;
return 1;
}
}
#if !defined(STB_VORBIS_NO_STDIO)
if (loc + f->f_start < loc || loc >= 0x80000000) {
loc = 0x7fffffff;
f->eof = 1;
} else {
loc += f->f_start;
}
if (!fseek(f->f, loc, SEEK_SET))
return 1;
f->eof = 1;
fseek(f->f, f->f_start, SEEK_END);
return 0;
#endif
}


static stbv_uint8 stbv_ogg_page_header[4] = { 0x4f, 0x67, 0x67, 0x53 };

static int stbv_capture_pattern(stbv_vorb *f)
{
if (0x4f != stbv_get8(f)) return FALSE;
if (0x67 != stbv_get8(f)) return FALSE;
if (0x67 != stbv_get8(f)) return FALSE;
if (0x53 != stbv_get8(f)) return FALSE;
return TRUE;
}

#define STBV_PAGEFLAG_continued_packet 1
#define STBV_PAGEFLAG_first_page 2
#define STBV_PAGEFLAG_last_page 4

static int stbv_start_page_no_capturepattern(stbv_vorb *f)
{
stbv_uint32 loc0,loc1,n;

if (0 != stbv_get8(f)) return stbv_error(f, VORBIS_invalid_stream_structure_version);

f->page_flag = stbv_get8(f);

loc0 = stbv_get32(f); 
loc1 = stbv_get32(f);


stbv_get32(f);


n = stbv_get32(f);
f->last_page = n;

stbv_get32(f);

f->segment_count = stbv_get8(f);
if (!stbv_getn(f, f->segments, f->segment_count))
return stbv_error(f, VORBIS_unexpected_eof);

f->end_seg_with_known_loc = -2;
if (loc0 != ~0U || loc1 != ~0U) {
int i;

for (i=f->segment_count-1; i >= 0; --i)
if (f->segments[i] < 255)
break;

if (i >= 0) {
f->end_seg_with_known_loc = i;
f->known_loc_for_packet = loc0;
}
}
if (f->first_decode) {
int i,len;
StbvProbedPage p;
len = 0;
for (i=0; i < f->segment_count; ++i)
len += f->segments[i];
len += 27 + f->segment_count;
p.page_start = f->first_audio_page_offset;
p.page_end = p.page_start + len;
p.last_decoded_sample = loc0;
f->p_first = p;
}
f->next_seg = 0;
return TRUE;
}

static int stbv_start_page(stbv_vorb *f)
{
if (!stbv_capture_pattern(f)) return stbv_error(f, VORBIS_missing_capture_pattern);
return stbv_start_page_no_capturepattern(f);
}

static int stbv_start_packet(stbv_vorb *f)
{
while (f->next_seg == -1) {
if (!stbv_start_page(f)) return FALSE;
if (f->page_flag & STBV_PAGEFLAG_continued_packet)
return stbv_error(f, VORBIS_continued_packet_flag_invalid);
}
f->last_seg = FALSE;
f->valid_bits = 0;
f->packet_bytes = 0;
f->bytes_in_seg = 0;

return TRUE;
}

static int stbv_maybe_start_packet(stbv_vorb *f)
{
if (f->next_seg == -1) {
int x = stbv_get8(f);
if (f->eof) return FALSE; 
if (0x4f != x ) return stbv_error(f, VORBIS_missing_capture_pattern);
if (0x67 != stbv_get8(f)) return stbv_error(f, VORBIS_missing_capture_pattern);
if (0x67 != stbv_get8(f)) return stbv_error(f, VORBIS_missing_capture_pattern);
if (0x53 != stbv_get8(f)) return stbv_error(f, VORBIS_missing_capture_pattern);
if (!stbv_start_page_no_capturepattern(f)) return FALSE;
if (f->page_flag & STBV_PAGEFLAG_continued_packet) {


f->last_seg = FALSE;
f->bytes_in_seg = 0;
return stbv_error(f, VORBIS_continued_packet_flag_invalid);
}
}
return stbv_start_packet(f);
}

static int stbv_next_segment(stbv_vorb *f)
{
int len;
if (f->last_seg) return 0;
if (f->next_seg == -1) {
f->last_seg_which = f->segment_count-1; 
if (!stbv_start_page(f)) { f->last_seg = 1; return 0; }
if (!(f->page_flag & STBV_PAGEFLAG_continued_packet)) return stbv_error(f, VORBIS_continued_packet_flag_invalid);
}
len = f->segments[f->next_seg++];
if (len < 255) {
f->last_seg = TRUE;
f->last_seg_which = f->next_seg-1;
}
if (f->next_seg >= f->segment_count)
f->next_seg = -1;
assert(f->bytes_in_seg == 0);
f->bytes_in_seg = len;
return len;
}

#define STBV_EOP (-1)
#define STBV_INVALID_BITS (-1)

static int stbv_get8_packet_raw(stbv_vorb *f)
{
if (!f->bytes_in_seg) { 
if (f->last_seg) return STBV_EOP;
else if (!stbv_next_segment(f)) return STBV_EOP;
}
assert(f->bytes_in_seg > 0);
--f->bytes_in_seg;
++f->packet_bytes;
return stbv_get8(f);
}

static int stbv_get8_packet(stbv_vorb *f)
{
int x = stbv_get8_packet_raw(f);
f->valid_bits = 0;
return x;
}

static void stbv_flush_packet(stbv_vorb *f)
{
while (stbv_get8_packet_raw(f) != STBV_EOP);
}



static stbv_uint32 stbv_get_bits(stbv_vorb *f, int n)
{
stbv_uint32 z;

if (f->valid_bits < 0) return 0;
if (f->valid_bits < n) {
if (n > 24) {

z = stbv_get_bits(f, 24);
z += stbv_get_bits(f, n-24) << 24;
return z;
}
if (f->valid_bits == 0) f->acc = 0;
while (f->valid_bits < n) {
int z = stbv_get8_packet_raw(f);
if (z == STBV_EOP) {
f->valid_bits = STBV_INVALID_BITS;
return 0;
}
f->acc += z << f->valid_bits;
f->valid_bits += 8;
}
}
if (f->valid_bits < 0) return 0;
z = f->acc & ((1 << n)-1);
f->acc >>= n;
f->valid_bits -= n;
return z;
}





static __forceinline void stbv_prep_huffman(stbv_vorb *f)
{
if (f->valid_bits <= 24) {
if (f->valid_bits == 0) f->acc = 0;
do {
int z;
if (f->last_seg && !f->bytes_in_seg) return;
z = stbv_get8_packet_raw(f);
if (z == STBV_EOP) return;
f->acc += (unsigned) z << f->valid_bits;
f->valid_bits += 8;
} while (f->valid_bits <= 24);
}
}

enum
{
STBV_VORBIS_packet_id = 1,
STBV_VORBIS_packet_comment = 3,
STBV_VORBIS_packet_setup = 5
};

static int stbv_codebook_decode_scalar_raw(stbv_vorb *f, StbvCodebook *c)
{
int i;
stbv_prep_huffman(f);

if (c->codewords == NULL && c->sorted_codewords == NULL)
return -1;



if (c->entries > 8 ? c->sorted_codewords!=NULL : !c->codewords) {

stbv_uint32 code = stbv_bit_reverse(f->acc);
int x=0, n=c->sorted_entries, len;

while (n > 1) {

int m = x + (n >> 1);
if (c->sorted_codewords[m] <= code) {
x = m;
n -= (n>>1);
} else {
n >>= 1;
}
}

if (!c->sparse) x = c->sorted_values[x];

len = c->codeword_lengths[x];
if (f->valid_bits >= len) {
f->acc >>= len;
f->valid_bits -= len;
return x;
}

f->valid_bits = 0;
return -1;
}


assert(!c->sparse);
for (i=0; i < c->entries; ++i) {
if (c->codeword_lengths[i] == NO_CODE) continue;
if (c->codewords[i] == (f->acc & ((1 << c->codeword_lengths[i])-1))) {
if (f->valid_bits >= c->codeword_lengths[i]) {
f->acc >>= c->codeword_lengths[i];
f->valid_bits -= c->codeword_lengths[i];
return i;
}
f->valid_bits = 0;
return -1;
}
}

stbv_error(f, VORBIS_invalid_stream);
f->valid_bits = 0;
return -1;
}

#if !defined(STB_VORBIS_NO_INLINE_DECODE)

#define STBV_DECODE_RAW(var, f,c) if (f->valid_bits < STB_VORBIS_FAST_HUFFMAN_LENGTH) stbv_prep_huffman(f); var = f->acc & STBV_FAST_HUFFMAN_TABLE_MASK; var = c->fast_huffman[var]; if (var >= 0) { int n = c->codeword_lengths[var]; f->acc >>= n; f->valid_bits -= n; if (f->valid_bits < 0) { f->valid_bits = 0; var = -1; } } else { var = stbv_codebook_decode_scalar_raw(f,c); }













#else

static int stbv_codebook_decode_scalar(stbv_vorb *f, StbvCodebook *c)
{
int i;
if (f->valid_bits < STB_VORBIS_FAST_HUFFMAN_LENGTH)
stbv_prep_huffman(f);

i = f->acc & STBV_FAST_HUFFMAN_TABLE_MASK;
i = c->fast_huffman[i];
if (i >= 0) {
f->acc >>= c->codeword_lengths[i];
f->valid_bits -= c->codeword_lengths[i];
if (f->valid_bits < 0) { f->valid_bits = 0; return -1; }
return i;
}
return stbv_codebook_decode_scalar_raw(f,c);
}

#define STBV_DECODE_RAW(var,f,c) var = stbv_codebook_decode_scalar(f,c);

#endif

#define STBV_DECODE(var,f,c) STBV_DECODE_RAW(var,f,c) if (c->sparse) var = c->sorted_values[var];



#if !defined(STB_VORBIS_DIVIDES_IN_CODEBOOK)
#define DECODE_VQ(var,f,c) STBV_DECODE_RAW(var,f,c)
#else
#define DECODE_VQ(var,f,c) STBV_DECODE(var,f,c)
#endif








#define STBV_CODEBOOK_ELEMENT(c,off) (c->multiplicands[off])
#define STBV_CODEBOOK_ELEMENT_FAST(c,off) (c->multiplicands[off])
#define STBV_CODEBOOK_ELEMENT_BASE(c) (0)

static int stbv_codebook_decode_start(stbv_vorb *f, StbvCodebook *c)
{
int z = -1;


if (c->lookup_type == 0)
stbv_error(f, VORBIS_invalid_stream);
else {
DECODE_VQ(z,f,c);
if (c->sparse) assert(z < c->sorted_entries);
if (z < 0) { 
if (!f->bytes_in_seg)
if (f->last_seg)
return z;
stbv_error(f, VORBIS_invalid_stream);
}
}
return z;
}

static int stbv_codebook_decode(stbv_vorb *f, StbvCodebook *c, float *output, int len)
{
int i,z = stbv_codebook_decode_start(f,c);
if (z < 0) return FALSE;
if (len > c->dimensions) len = c->dimensions;

#if defined(STB_VORBIS_DIVIDES_IN_CODEBOOK)
if (c->lookup_type == 1) {
float last = STBV_CODEBOOK_ELEMENT_BASE(c);
int div = 1;
for (i=0; i < len; ++i) {
int off = (z / div) % c->lookup_values;
float val = STBV_CODEBOOK_ELEMENT_FAST(c,off) + last;
output[i] += val;
if (c->sequence_p) last = val + c->minimum_value;
div *= c->lookup_values;
}
return TRUE;
}
#endif

z *= c->dimensions;
if (c->sequence_p) {
float last = STBV_CODEBOOK_ELEMENT_BASE(c);
for (i=0; i < len; ++i) {
float val = STBV_CODEBOOK_ELEMENT_FAST(c,z+i) + last;
output[i] += val;
last = val + c->minimum_value;
}
} else {
float last = STBV_CODEBOOK_ELEMENT_BASE(c);
for (i=0; i < len; ++i) {
output[i] += STBV_CODEBOOK_ELEMENT_FAST(c,z+i) + last;
}
}

return TRUE;
}

static int stbv_codebook_decode_step(stbv_vorb *f, StbvCodebook *c, float *output, int len, int step)
{
int i,z = stbv_codebook_decode_start(f,c);
float last = STBV_CODEBOOK_ELEMENT_BASE(c);
if (z < 0) return FALSE;
if (len > c->dimensions) len = c->dimensions;

#if defined(STB_VORBIS_DIVIDES_IN_CODEBOOK)
if (c->lookup_type == 1) {
int div = 1;
for (i=0; i < len; ++i) {
int off = (z / div) % c->lookup_values;
float val = STBV_CODEBOOK_ELEMENT_FAST(c,off) + last;
output[i*step] += val;
if (c->sequence_p) last = val;
div *= c->lookup_values;
}
return TRUE;
}
#endif

z *= c->dimensions;
for (i=0; i < len; ++i) {
float val = STBV_CODEBOOK_ELEMENT_FAST(c,z+i) + last;
output[i*step] += val;
if (c->sequence_p) last = val;
}

return TRUE;
}

static int stbv_codebook_decode_deinterleave_repeat(stbv_vorb *f, StbvCodebook *c, float **outputs, int ch, int *c_inter_p, int *p_inter_p, int len, int total_decode)
{
int c_inter = *c_inter_p;
int p_inter = *p_inter_p;
int i,z, effective = c->dimensions;


if (c->lookup_type == 0) return stbv_error(f, VORBIS_invalid_stream);

while (total_decode > 0) {
float last = STBV_CODEBOOK_ELEMENT_BASE(c);
DECODE_VQ(z,f,c);
#if !defined(STB_VORBIS_DIVIDES_IN_CODEBOOK)
assert(!c->sparse || z < c->sorted_entries);
#endif
if (z < 0) {
if (!f->bytes_in_seg)
if (f->last_seg) return FALSE;
return stbv_error(f, VORBIS_invalid_stream);
}





if (c_inter + p_inter*ch + effective > len * ch) {
effective = len*ch - (p_inter*ch - c_inter);
}

#if defined(STB_VORBIS_DIVIDES_IN_CODEBOOK)
if (c->lookup_type == 1) {
int div = 1;
for (i=0; i < effective; ++i) {
int off = (z / div) % c->lookup_values;
float val = STBV_CODEBOOK_ELEMENT_FAST(c,off) + last;
if (outputs[c_inter])
outputs[c_inter][p_inter] += val;
if (++c_inter == ch) { c_inter = 0; ++p_inter; }
if (c->sequence_p) last = val;
div *= c->lookup_values;
}
} else
#endif
{
z *= c->dimensions;
if (c->sequence_p) {
for (i=0; i < effective; ++i) {
float val = STBV_CODEBOOK_ELEMENT_FAST(c,z+i) + last;
if (outputs[c_inter])
outputs[c_inter][p_inter] += val;
if (++c_inter == ch) { c_inter = 0; ++p_inter; }
last = val;
}
} else {
for (i=0; i < effective; ++i) {
float val = STBV_CODEBOOK_ELEMENT_FAST(c,z+i) + last;
if (outputs[c_inter])
outputs[c_inter][p_inter] += val;
if (++c_inter == ch) { c_inter = 0; ++p_inter; }
}
}
}

total_decode -= effective;
}
*c_inter_p = c_inter;
*p_inter_p = p_inter;
return TRUE;
}

static int stbv_predict_point(int x, int x0, int x1, int y0, int y1)
{
int dy = y1 - y0;
int adx = x1 - x0;

int err = abs(dy) * (x - x0);
int off = err / adx;
return dy < 0 ? y0 - off : y0 + off;
}


static float stbv_inverse_db_table[256] =
{
1.0649863e-07f, 1.1341951e-07f, 1.2079015e-07f, 1.2863978e-07f, 
1.3699951e-07f, 1.4590251e-07f, 1.5538408e-07f, 1.6548181e-07f, 
1.7623575e-07f, 1.8768855e-07f, 1.9988561e-07f, 2.1287530e-07f, 
2.2670913e-07f, 2.4144197e-07f, 2.5713223e-07f, 2.7384213e-07f, 
2.9163793e-07f, 3.1059021e-07f, 3.3077411e-07f, 3.5226968e-07f, 
3.7516214e-07f, 3.9954229e-07f, 4.2550680e-07f, 4.5315863e-07f, 
4.8260743e-07f, 5.1396998e-07f, 5.4737065e-07f, 5.8294187e-07f, 
6.2082472e-07f, 6.6116941e-07f, 7.0413592e-07f, 7.4989464e-07f, 
7.9862701e-07f, 8.5052630e-07f, 9.0579828e-07f, 9.6466216e-07f, 
1.0273513e-06f, 1.0941144e-06f, 1.1652161e-06f, 1.2409384e-06f, 
1.3215816e-06f, 1.4074654e-06f, 1.4989305e-06f, 1.5963394e-06f, 
1.7000785e-06f, 1.8105592e-06f, 1.9282195e-06f, 2.0535261e-06f, 
2.1869758e-06f, 2.3290978e-06f, 2.4804557e-06f, 2.6416497e-06f, 
2.8133190e-06f, 2.9961443e-06f, 3.1908506e-06f, 3.3982101e-06f, 
3.6190449e-06f, 3.8542308e-06f, 4.1047004e-06f, 4.3714470e-06f, 
4.6555282e-06f, 4.9580707e-06f, 5.2802740e-06f, 5.6234160e-06f, 
5.9888572e-06f, 6.3780469e-06f, 6.7925283e-06f, 7.2339451e-06f, 
7.7040476e-06f, 8.2047000e-06f, 8.7378876e-06f, 9.3057248e-06f, 
9.9104632e-06f, 1.0554501e-05f, 1.1240392e-05f, 1.1970856e-05f, 
1.2748789e-05f, 1.3577278e-05f, 1.4459606e-05f, 1.5399272e-05f, 
1.6400004e-05f, 1.7465768e-05f, 1.8600792e-05f, 1.9809576e-05f, 
2.1096914e-05f, 2.2467911e-05f, 2.3928002e-05f, 2.5482978e-05f, 
2.7139006e-05f, 2.8902651e-05f, 3.0780908e-05f, 3.2781225e-05f, 
3.4911534e-05f, 3.7180282e-05f, 3.9596466e-05f, 4.2169667e-05f, 
4.4910090e-05f, 4.7828601e-05f, 5.0936773e-05f, 5.4246931e-05f, 
5.7772202e-05f, 6.1526565e-05f, 6.5524908e-05f, 6.9783085e-05f, 
7.4317983e-05f, 7.9147585e-05f, 8.4291040e-05f, 8.9768747e-05f, 
9.5602426e-05f, 0.00010181521f, 0.00010843174f, 0.00011547824f, 
0.00012298267f, 0.00013097477f, 0.00013948625f, 0.00014855085f, 
0.00015820453f, 0.00016848555f, 0.00017943469f, 0.00019109536f, 
0.00020351382f, 0.00021673929f, 0.00023082423f, 0.00024582449f, 
0.00026179955f, 0.00027881276f, 0.00029693158f, 0.00031622787f, 
0.00033677814f, 0.00035866388f, 0.00038197188f, 0.00040679456f, 
0.00043323036f, 0.00046138411f, 0.00049136745f, 0.00052329927f, 
0.00055730621f, 0.00059352311f, 0.00063209358f, 0.00067317058f, 
0.00071691700f, 0.00076350630f, 0.00081312324f, 0.00086596457f, 
0.00092223983f, 0.00098217216f, 0.0010459992f, 0.0011139742f, 
0.0011863665f, 0.0012634633f, 0.0013455702f, 0.0014330129f, 
0.0015261382f, 0.0016253153f, 0.0017309374f, 0.0018434235f, 
0.0019632195f, 0.0020908006f, 0.0022266726f, 0.0023713743f, 
0.0025254795f, 0.0026895994f, 0.0028643847f, 0.0030505286f, 
0.0032487691f, 0.0034598925f, 0.0036847358f, 0.0039241906f, 
0.0041792066f, 0.0044507950f, 0.0047400328f, 0.0050480668f, 
0.0053761186f, 0.0057254891f, 0.0060975636f, 0.0064938176f, 
0.0069158225f, 0.0073652516f, 0.0078438871f, 0.0083536271f, 
0.0088964928f, 0.009474637f, 0.010090352f, 0.010746080f, 
0.011444421f, 0.012188144f, 0.012980198f, 0.013823725f, 
0.014722068f, 0.015678791f, 0.016697687f, 0.017782797f, 
0.018938423f, 0.020169149f, 0.021479854f, 0.022875735f, 
0.024362330f, 0.025945531f, 0.027631618f, 0.029427276f, 
0.031339626f, 0.033376252f, 0.035545228f, 0.037855157f, 
0.040315199f, 0.042935108f, 0.045725273f, 0.048696758f, 
0.051861348f, 0.055231591f, 0.058820850f, 0.062643361f, 
0.066714279f, 0.071049749f, 0.075666962f, 0.080584227f, 
0.085821044f, 0.091398179f, 0.097337747f, 0.10366330f, 
0.11039993f, 0.11757434f, 0.12521498f, 0.13335215f, 
0.14201813f, 0.15124727f, 0.16107617f, 0.17154380f, 
0.18269168f, 0.19456402f, 0.20720788f, 0.22067342f, 
0.23501402f, 0.25028656f, 0.26655159f, 0.28387361f, 
0.30232132f, 0.32196786f, 0.34289114f, 0.36517414f, 
0.38890521f, 0.41417847f, 0.44109412f, 0.46975890f, 
0.50028648f, 0.53279791f, 0.56742212f, 0.60429640f, 
0.64356699f, 0.68538959f, 0.72993007f, 0.77736504f, 
0.82788260f, 0.88168307f, 0.9389798f, 1.0f
};









#if !defined(STB_VORBIS_NO_DEFER_FLOOR)
#define STBV_LINE_OP(a,b) a *= b
#else
#define STBV_LINE_OP(a,b) a = b
#endif

#if defined(STB_VORBIS_DIVIDE_TABLE)
#define STBV_DIVTAB_NUMER 32
#define STBV_DIVTAB_DENOM 64
stbv_int8 stbv_integer_divide_table[STBV_DIVTAB_NUMER][STBV_DIVTAB_DENOM]; 
#endif

static __forceinline void stbv_draw_line(float *output, int x0, int y0, int x1, int y1, int n)
{
int dy = y1 - y0;
int adx = x1 - x0;
int ady = abs(dy);
int base;
int x=x0,y=y0;
int err = 0;
int sy;

#if defined(STB_VORBIS_DIVIDE_TABLE)
if (adx < STBV_DIVTAB_DENOM && ady < STBV_DIVTAB_NUMER) {
if (dy < 0) {
base = -stbv_integer_divide_table[ady][adx];
sy = base-1;
} else {
base = stbv_integer_divide_table[ady][adx];
sy = base+1;
}
} else {
base = dy / adx;
if (dy < 0)
sy = base - 1;
else
sy = base+1;
}
#else
base = dy / adx;
if (dy < 0)
sy = base - 1;
else
sy = base+1;
#endif
ady -= abs(base) * adx;
if (x1 > n) x1 = n;
if (x < x1) {
STBV_LINE_OP(output[x], stbv_inverse_db_table[y]);
for (++x; x < x1; ++x) {
err += ady;
if (err >= adx) {
err -= adx;
y += sy;
} else
y += base;
STBV_LINE_OP(output[x], stbv_inverse_db_table[y]);
}
}
}

static int stbv_residue_decode(stbv_vorb *f, StbvCodebook *book, float *target, int offset, int n, int rtype)
{
int k;
if (rtype == 0) {
int step = n / book->dimensions;
for (k=0; k < step; ++k)
if (!stbv_codebook_decode_step(f, book, target+offset+k, n-offset-k, step))
return FALSE;
} else {
for (k=0; k < n; ) {
if (!stbv_codebook_decode(f, book, target+offset, n-k))
return FALSE;
k += book->dimensions;
offset += book->dimensions;
}
}
return TRUE;
}



static void stbv_decode_residue(stbv_vorb *f, float *residue_buffers[], int ch, int n, int rn, stbv_uint8 *do_not_decode)
{
int i,j,pass;
StbvResidue *r = f->residue_config + rn;
int rtype = f->residue_types[rn];
int c = r->classbook;
int classwords = f->codebooks[c].dimensions;
unsigned int actual_size = rtype == 2 ? n*2 : n;
unsigned int limit_r_begin = (r->begin < actual_size ? r->begin : actual_size);
unsigned int limit_r_end = (r->end < actual_size ? r->end : actual_size);
int n_read = limit_r_end - limit_r_begin;
int part_read = n_read / r->part_size;
int temp_alloc_point = stbv_temp_alloc_save(f);
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
stbv_uint8 ***part_classdata = (stbv_uint8 ***) stbv_temp_block_array(f,f->channels, part_read * sizeof(**part_classdata));
#else
int **classifications = (int **) stbv_temp_block_array(f,f->channels, part_read * sizeof(**classifications));
#endif

STBV_CHECK(f);

for (i=0; i < ch; ++i)
if (!do_not_decode[i])
memset(residue_buffers[i], 0, sizeof(float) * n);

if (rtype == 2 && ch != 1) {
for (j=0; j < ch; ++j)
if (!do_not_decode[j])
break;
if (j == ch)
goto done;

for (pass=0; pass < 8; ++pass) {
int pcount = 0, class_set = 0;
if (ch == 2) {
while (pcount < part_read) {
int z = r->begin + pcount*r->part_size;
int c_inter = (z & 1), p_inter = z>>1;
if (pass == 0) {
StbvCodebook *c = f->codebooks+r->classbook;
int q;
STBV_DECODE(q,f,c);
if (q == STBV_EOP) goto done;
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
part_classdata[0][class_set] = r->classdata[q];
#else
for (i=classwords-1; i >= 0; --i) {
classifications[0][i+pcount] = q % r->classifications;
q /= r->classifications;
}
#endif
}
for (i=0; i < classwords && pcount < part_read; ++i, ++pcount) {
int z = r->begin + pcount*r->part_size;
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
int c = part_classdata[0][class_set][i];
#else
int c = classifications[0][pcount];
#endif
int b = r->residue_books[c][pass];
if (b >= 0) {
StbvCodebook *book = f->codebooks + b;
#if defined(STB_VORBIS_DIVIDES_IN_CODEBOOK)
if (!stbv_codebook_decode_deinterleave_repeat(f, book, residue_buffers, ch, &c_inter, &p_inter, n, r->part_size))
goto done;
#else

if (!stbv_codebook_decode_deinterleave_repeat(f, book, residue_buffers, ch, &c_inter, &p_inter, n, r->part_size))
goto done;
#endif
} else {
z += r->part_size;
c_inter = z & 1;
p_inter = z >> 1;
}
}
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
++class_set;
#endif
}
} else if (ch == 1) {
while (pcount < part_read) {
int z = r->begin + pcount*r->part_size;
int c_inter = 0, p_inter = z;
if (pass == 0) {
StbvCodebook *c = f->codebooks+r->classbook;
int q;
STBV_DECODE(q,f,c);
if (q == STBV_EOP) goto done;
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
part_classdata[0][class_set] = r->classdata[q];
#else
for (i=classwords-1; i >= 0; --i) {
classifications[0][i+pcount] = q % r->classifications;
q /= r->classifications;
}
#endif
}
for (i=0; i < classwords && pcount < part_read; ++i, ++pcount) {
int z = r->begin + pcount*r->part_size;
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
int c = part_classdata[0][class_set][i];
#else
int c = classifications[0][pcount];
#endif
int b = r->residue_books[c][pass];
if (b >= 0) {
StbvCodebook *book = f->codebooks + b;
if (!stbv_codebook_decode_deinterleave_repeat(f, book, residue_buffers, ch, &c_inter, &p_inter, n, r->part_size))
goto done;
} else {
z += r->part_size;
c_inter = 0;
p_inter = z;
}
}
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
++class_set;
#endif
}
} else {
while (pcount < part_read) {
int z = r->begin + pcount*r->part_size;
int c_inter = z % ch, p_inter = z/ch;
if (pass == 0) {
StbvCodebook *c = f->codebooks+r->classbook;
int q;
STBV_DECODE(q,f,c);
if (q == STBV_EOP) goto done;
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
part_classdata[0][class_set] = r->classdata[q];
#else
for (i=classwords-1; i >= 0; --i) {
classifications[0][i+pcount] = q % r->classifications;
q /= r->classifications;
}
#endif
}
for (i=0; i < classwords && pcount < part_read; ++i, ++pcount) {
int z = r->begin + pcount*r->part_size;
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
int c = part_classdata[0][class_set][i];
#else
int c = classifications[0][pcount];
#endif
int b = r->residue_books[c][pass];
if (b >= 0) {
StbvCodebook *book = f->codebooks + b;
if (!stbv_codebook_decode_deinterleave_repeat(f, book, residue_buffers, ch, &c_inter, &p_inter, n, r->part_size))
goto done;
} else {
z += r->part_size;
c_inter = z % ch;
p_inter = z / ch;
}
}
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
++class_set;
#endif
}
}
}
goto done;
}
STBV_CHECK(f);

for (pass=0; pass < 8; ++pass) {
int pcount = 0, class_set=0;
while (pcount < part_read) {
if (pass == 0) {
for (j=0; j < ch; ++j) {
if (!do_not_decode[j]) {
StbvCodebook *c = f->codebooks+r->classbook;
int temp;
STBV_DECODE(temp,f,c);
if (temp == STBV_EOP) goto done;
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
part_classdata[j][class_set] = r->classdata[temp];
#else
for (i=classwords-1; i >= 0; --i) {
classifications[j][i+pcount] = temp % r->classifications;
temp /= r->classifications;
}
#endif
}
}
}
for (i=0; i < classwords && pcount < part_read; ++i, ++pcount) {
for (j=0; j < ch; ++j) {
if (!do_not_decode[j]) {
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
int c = part_classdata[j][class_set][i];
#else
int c = classifications[j][pcount];
#endif
int b = r->residue_books[c][pass];
if (b >= 0) {
float *target = residue_buffers[j];
int offset = r->begin + pcount * r->part_size;
int n = r->part_size;
StbvCodebook *book = f->codebooks + b;
if (!stbv_residue_decode(f, book, target, offset, n, rtype))
goto done;
}
}
}
}
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
++class_set;
#endif
}
}
done:
STBV_CHECK(f);
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
stbv_temp_free(f,part_classdata);
#else
stbv_temp_free(f,classifications);
#endif
stbv_temp_alloc_restore(f,temp_alloc_point);
}


#if 0

void inverse_mdct_slow(float *buffer, int n)
{
int i,j;
int n2 = n >> 1;
float *x = (float *) malloc(sizeof(*x) * n2);
memcpy(x, buffer, sizeof(*x) * n2);
for (i=0; i < n; ++i) {
float acc = 0;
for (j=0; j < n2; ++j)







acc += x[j] * (float) cos(M_PI / 2 / n * (2 * i + 1 + n/2.0)*(2*j+1));
buffer[i] = acc;
}
free(x);
}
#elif 0

void inverse_mdct_slow(float *buffer, int n, stbv_vorb *f, int blocktype)
{
float mcos[16384];
int i,j;
int n2 = n >> 1, nmask = (n << 2) -1;
float *x = (float *) malloc(sizeof(*x) * n2);
memcpy(x, buffer, sizeof(*x) * n2);
for (i=0; i < 4*n; ++i)
mcos[i] = (float) cos(M_PI / 2 * i / n);

for (i=0; i < n; ++i) {
float acc = 0;
for (j=0; j < n2; ++j)
acc += x[j] * mcos[(2 * i + 1 + n2)*(2*j+1) & nmask];
buffer[i] = acc;
}
free(x);
}
#elif 0


void dct_iv_slow(float *buffer, int n)
{
float mcos[16384];
float x[2048];
int i,j;
int n2 = n >> 1, nmask = (n << 3) - 1;
memcpy(x, buffer, sizeof(*x) * n);
for (i=0; i < 8*n; ++i)
mcos[i] = (float) cos(M_PI / 4 * i / n);
for (i=0; i < n; ++i) {
float acc = 0;
for (j=0; j < n; ++j)
acc += x[j] * mcos[((2 * i + 1)*(2*j+1)) & nmask];
buffer[i] = acc;
}
}

void inverse_mdct_slow(float *buffer, int n, stbv_vorb *f, int blocktype)
{
int i, n4 = n >> 2, n2 = n >> 1, n3_4 = n - n4;
float temp[4096];

memcpy(temp, buffer, n2 * sizeof(float));
dct_iv_slow(temp, n2); 

for (i=0; i < n4 ; ++i) buffer[i] = temp[i+n4]; 
for ( ; i < n3_4; ++i) buffer[i] = -temp[n3_4 - i - 1]; 
for ( ; i < n ; ++i) buffer[i] = -temp[i - n3_4]; 
}
#endif

#if !defined(LIBVORBIS_MDCT)
#define LIBVORBIS_MDCT 0
#endif

#if LIBVORBIS_MDCT


typedef struct 
{
int n;
int log2n;

float *trig;
int *bitrev;

float scale;
} mdct_lookup;

extern void mdct_init(mdct_lookup *lookup, int n);
extern void mdct_clear(mdct_lookup *l);
extern void mdct_backward(mdct_lookup *init, float *in, float *out);

mdct_lookup M1,M2;

void stbv_inverse_mdct(float *buffer, int n, stbv_vorb *f, int blocktype)
{
mdct_lookup *M;
if (M1.n == n) M = &M1;
else if (M2.n == n) M = &M2;
else if (M1.n == 0) { mdct_init(&M1, n); M = &M1; }
else { 
if (M2.n) __asm int 3;
mdct_init(&M2, n);
M = &M2;
}

mdct_backward(M, buffer, buffer);
}
#endif





static void stbv_imdct_step3_iter0_loop(int n, float *e, int i_off, int k_off, float *A)
{
float *ee0 = e + i_off;
float *ee2 = ee0 + k_off;
int i;

assert((n & 3) == 0);
for (i=(n>>2); i > 0; --i) {
float k00_20, k01_21;
k00_20 = ee0[ 0] - ee2[ 0];
k01_21 = ee0[-1] - ee2[-1];
ee0[ 0] += ee2[ 0];
ee0[-1] += ee2[-1];
ee2[ 0] = k00_20 * A[0] - k01_21 * A[1];
ee2[-1] = k01_21 * A[0] + k00_20 * A[1];
A += 8;

k00_20 = ee0[-2] - ee2[-2];
k01_21 = ee0[-3] - ee2[-3];
ee0[-2] += ee2[-2];
ee0[-3] += ee2[-3];
ee2[-2] = k00_20 * A[0] - k01_21 * A[1];
ee2[-3] = k01_21 * A[0] + k00_20 * A[1];
A += 8;

k00_20 = ee0[-4] - ee2[-4];
k01_21 = ee0[-5] - ee2[-5];
ee0[-4] += ee2[-4];
ee0[-5] += ee2[-5];
ee2[-4] = k00_20 * A[0] - k01_21 * A[1];
ee2[-5] = k01_21 * A[0] + k00_20 * A[1];
A += 8;

k00_20 = ee0[-6] - ee2[-6];
k01_21 = ee0[-7] - ee2[-7];
ee0[-6] += ee2[-6];
ee0[-7] += ee2[-7];
ee2[-6] = k00_20 * A[0] - k01_21 * A[1];
ee2[-7] = k01_21 * A[0] + k00_20 * A[1];
A += 8;
ee0 -= 8;
ee2 -= 8;
}
}

static void stbv_imdct_step3_inner_r_loop(int lim, float *e, int d0, int k_off, float *A, int k1)
{
int i;
float k00_20, k01_21;

float *e0 = e + d0;
float *e2 = e0 + k_off;

for (i=lim >> 2; i > 0; --i) {
k00_20 = e0[-0] - e2[-0];
k01_21 = e0[-1] - e2[-1];
e0[-0] += e2[-0];
e0[-1] += e2[-1];
e2[-0] = (k00_20)*A[0] - (k01_21) * A[1];
e2[-1] = (k01_21)*A[0] + (k00_20) * A[1];

A += k1;

k00_20 = e0[-2] - e2[-2];
k01_21 = e0[-3] - e2[-3];
e0[-2] += e2[-2];
e0[-3] += e2[-3];
e2[-2] = (k00_20)*A[0] - (k01_21) * A[1];
e2[-3] = (k01_21)*A[0] + (k00_20) * A[1];

A += k1;

k00_20 = e0[-4] - e2[-4];
k01_21 = e0[-5] - e2[-5];
e0[-4] += e2[-4];
e0[-5] += e2[-5];
e2[-4] = (k00_20)*A[0] - (k01_21) * A[1];
e2[-5] = (k01_21)*A[0] + (k00_20) * A[1];

A += k1;

k00_20 = e0[-6] - e2[-6];
k01_21 = e0[-7] - e2[-7];
e0[-6] += e2[-6];
e0[-7] += e2[-7];
e2[-6] = (k00_20)*A[0] - (k01_21) * A[1];
e2[-7] = (k01_21)*A[0] + (k00_20) * A[1];

e0 -= 8;
e2 -= 8;

A += k1;
}
}

static void stbv_imdct_step3_inner_s_loop(int n, float *e, int i_off, int k_off, float *A, int a_off, int k0)
{
int i;
float A0 = A[0];
float A1 = A[0+1];
float A2 = A[0+a_off];
float A3 = A[0+a_off+1];
float A4 = A[0+a_off*2+0];
float A5 = A[0+a_off*2+1];
float A6 = A[0+a_off*3+0];
float A7 = A[0+a_off*3+1];

float k00,k11;

float *ee0 = e +i_off;
float *ee2 = ee0+k_off;

for (i=n; i > 0; --i) {
k00 = ee0[ 0] - ee2[ 0];
k11 = ee0[-1] - ee2[-1];
ee0[ 0] = ee0[ 0] + ee2[ 0];
ee0[-1] = ee0[-1] + ee2[-1];
ee2[ 0] = (k00) * A0 - (k11) * A1;
ee2[-1] = (k11) * A0 + (k00) * A1;

k00 = ee0[-2] - ee2[-2];
k11 = ee0[-3] - ee2[-3];
ee0[-2] = ee0[-2] + ee2[-2];
ee0[-3] = ee0[-3] + ee2[-3];
ee2[-2] = (k00) * A2 - (k11) * A3;
ee2[-3] = (k11) * A2 + (k00) * A3;

k00 = ee0[-4] - ee2[-4];
k11 = ee0[-5] - ee2[-5];
ee0[-4] = ee0[-4] + ee2[-4];
ee0[-5] = ee0[-5] + ee2[-5];
ee2[-4] = (k00) * A4 - (k11) * A5;
ee2[-5] = (k11) * A4 + (k00) * A5;

k00 = ee0[-6] - ee2[-6];
k11 = ee0[-7] - ee2[-7];
ee0[-6] = ee0[-6] + ee2[-6];
ee0[-7] = ee0[-7] + ee2[-7];
ee2[-6] = (k00) * A6 - (k11) * A7;
ee2[-7] = (k11) * A6 + (k00) * A7;

ee0 -= k0;
ee2 -= k0;
}
}

static __forceinline void stbv_iter_54(float *z)
{
float k00,k11,k22,k33;
float y0,y1,y2,y3;

k00 = z[ 0] - z[-4];
y0 = z[ 0] + z[-4];
y2 = z[-2] + z[-6];
k22 = z[-2] - z[-6];

z[-0] = y0 + y2; 
z[-2] = y0 - y2; 



k33 = z[-3] - z[-7];

z[-4] = k00 + k33; 
z[-6] = k00 - k33; 



k11 = z[-1] - z[-5];
y1 = z[-1] + z[-5];
y3 = z[-3] + z[-7];

z[-1] = y1 + y3; 
z[-3] = y1 - y3; 
z[-5] = k11 - k22; 
z[-7] = k11 + k22; 
}

static void stbv_imdct_step3_inner_s_loop_ld654(int n, float *e, int i_off, float *A, int base_n)
{
int a_off = base_n >> 3;
float A2 = A[0+a_off];
float *z = e + i_off;
float *base = z - 16 * n;

while (z > base) {
float k00,k11;

k00 = z[-0] - z[-8];
k11 = z[-1] - z[-9];
z[-0] = z[-0] + z[-8];
z[-1] = z[-1] + z[-9];
z[-8] = k00;
z[-9] = k11 ;

k00 = z[ -2] - z[-10];
k11 = z[ -3] - z[-11];
z[ -2] = z[ -2] + z[-10];
z[ -3] = z[ -3] + z[-11];
z[-10] = (k00+k11) * A2;
z[-11] = (k11-k00) * A2;

k00 = z[-12] - z[ -4]; 
k11 = z[ -5] - z[-13];
z[ -4] = z[ -4] + z[-12];
z[ -5] = z[ -5] + z[-13];
z[-12] = k11;
z[-13] = k00;

k00 = z[-14] - z[ -6]; 
k11 = z[ -7] - z[-15];
z[ -6] = z[ -6] + z[-14];
z[ -7] = z[ -7] + z[-15];
z[-14] = (k00+k11) * A2;
z[-15] = (k00-k11) * A2;

stbv_iter_54(z);
stbv_iter_54(z-8);
z -= 16;
}
}

static void stbv_inverse_mdct(float *buffer, int n, stbv_vorb *f, int blocktype)
{
int n2 = n >> 1, n4 = n >> 2, n8 = n >> 3, l;
int ld;

int save_point = stbv_temp_alloc_save(f);
float *buf2 = (float *) stbv_temp_alloc(f, n2 * sizeof(*buf2));
float *u=NULL,*v=NULL;

float *A = f->A[blocktype];




















{
float *d,*e, *AA, *e_stop;
d = &buf2[n2-2];
AA = A;
e = &buffer[0];
e_stop = &buffer[n2];
while (e != e_stop) {
d[1] = (e[0] * AA[0] - e[2]*AA[1]);
d[0] = (e[0] * AA[1] + e[2]*AA[0]);
d -= 2;
AA += 2;
e += 4;
}

e = &buffer[n2-3];
while (d >= buf2) {
d[1] = (-e[2] * AA[0] - -e[0]*AA[1]);
d[0] = (-e[2] * AA[1] + -e[0]*AA[0]);
d -= 2;
AA += 2;
e -= 4;
}
}





u = buffer;
v = buf2;




{
float *AA = &A[n2-8];
float *d0,*d1, *e0, *e1;

e0 = &v[n4];
e1 = &v[0];

d0 = &u[n4];
d1 = &u[0];

while (AA >= A) {
float v40_20, v41_21;

v41_21 = e0[1] - e1[1];
v40_20 = e0[0] - e1[0];
d0[1] = e0[1] + e1[1];
d0[0] = e0[0] + e1[0];
d1[1] = v41_21*AA[4] - v40_20*AA[5];
d1[0] = v40_20*AA[4] + v41_21*AA[5];

v41_21 = e0[3] - e1[3];
v40_20 = e0[2] - e1[2];
d0[3] = e0[3] + e1[3];
d0[2] = e0[2] + e1[2];
d1[3] = v41_21*AA[0] - v40_20*AA[1];
d1[2] = v40_20*AA[0] + v41_21*AA[1];

AA -= 8;

d0 += 4;
d1 += 4;
e0 += 4;
e1 += 4;
}
}


ld = stbv_ilog(n) - 1; 









stbv_imdct_step3_iter0_loop(n >> 4, u, n2-1-n4*0, -(n >> 3), A);
stbv_imdct_step3_iter0_loop(n >> 4, u, n2-1-n4*1, -(n >> 3), A);


stbv_imdct_step3_inner_r_loop(n >> 5, u, n2-1 - n8*0, -(n >> 4), A, 16);
stbv_imdct_step3_inner_r_loop(n >> 5, u, n2-1 - n8*1, -(n >> 4), A, 16);
stbv_imdct_step3_inner_r_loop(n >> 5, u, n2-1 - n8*2, -(n >> 4), A, 16);
stbv_imdct_step3_inner_r_loop(n >> 5, u, n2-1 - n8*3, -(n >> 4), A, 16);

l=2;
for (; l < (ld-3)>>1; ++l) {
int k0 = n >> (l+2), k0_2 = k0>>1;
int lim = 1 << (l+1);
int i;
for (i=0; i < lim; ++i)
stbv_imdct_step3_inner_r_loop(n >> (l+4), u, n2-1 - k0*i, -k0_2, A, 1 << (l+3));
}

for (; l < ld-6; ++l) {
int k0 = n >> (l+2), k1 = 1 << (l+3), k0_2 = k0>>1;
int rlim = n >> (l+6), r;
int lim = 1 << (l+1);
int i_off;
float *A0 = A;
i_off = n2-1;
for (r=rlim; r > 0; --r) {
stbv_imdct_step3_inner_s_loop(lim, u, i_off, -k0_2, A0, k1, k0);
A0 += k1*4;
i_off -= 8;
}
}






stbv_imdct_step3_inner_s_loop_ld654(n >> 5, u, n2-1, A, n);





{
stbv_uint16 *bitrev = f->stbv_bit_reverse[blocktype];





float *d0 = &v[n4-4];
float *d1 = &v[n2-4];
while (d0 >= v) {
int k4;

k4 = bitrev[0];
d1[3] = u[k4+0];
d1[2] = u[k4+1];
d0[3] = u[k4+2];
d0[2] = u[k4+3];

k4 = bitrev[1];
d1[1] = u[k4+0];
d1[0] = u[k4+1];
d0[1] = u[k4+2];
d0[0] = u[k4+3];

d0 -= 4;
d1 -= 4;
bitrev += 2;
}
}




assert(v == buf2);



{
float *C = f->C[blocktype];
float *d, *e;

d = v;
e = v + n2 - 4;

while (d < e) {
float a02,a11,b0,b1,b2,b3;

a02 = d[0] - e[2];
a11 = d[1] + e[3];

b0 = C[1]*a02 + C[0]*a11;
b1 = C[1]*a11 - C[0]*a02;

b2 = d[0] + e[ 2];
b3 = d[1] - e[ 3];

d[0] = b2 + b0;
d[1] = b3 + b1;
e[2] = b2 - b0;
e[3] = b1 - b3;

a02 = d[2] - e[0];
a11 = d[3] + e[1];

b0 = C[3]*a02 + C[2]*a11;
b1 = C[3]*a11 - C[2]*a02;

b2 = d[2] + e[ 0];
b3 = d[3] - e[ 1];

d[2] = b2 + b0;
d[3] = b3 + b1;
e[0] = b2 - b0;
e[1] = b1 - b3;

C += 4;
d += 4;
e -= 4;
}
}











{
float *d0,*d1,*d2,*d3;

float *B = f->B[blocktype] + n2 - 8;
float *e = buf2 + n2 - 8;
d0 = &buffer[0];
d1 = &buffer[n2-4];
d2 = &buffer[n2];
d3 = &buffer[n-4];
while (e >= v) {
float p0,p1,p2,p3;

p3 = e[6]*B[7] - e[7]*B[6];
p2 = -e[6]*B[6] - e[7]*B[7]; 

d0[0] = p3;
d1[3] = - p3;
d2[0] = p2;
d3[3] = p2;

p1 = e[4]*B[5] - e[5]*B[4];
p0 = -e[4]*B[4] - e[5]*B[5]; 

d0[1] = p1;
d1[2] = - p1;
d2[1] = p0;
d3[2] = p0;

p3 = e[2]*B[3] - e[3]*B[2];
p2 = -e[2]*B[2] - e[3]*B[3]; 

d0[2] = p3;
d1[1] = - p3;
d2[2] = p2;
d3[1] = p2;

p1 = e[0]*B[1] - e[1]*B[0];
p0 = -e[0]*B[0] - e[1]*B[1]; 

d0[3] = p1;
d1[0] = - p1;
d2[3] = p0;
d3[0] = p0;

B -= 8;
e -= 8;
d0 += 4;
d2 += 4;
d1 -= 4;
d3 -= 4;
}
}

stbv_temp_free(f,buf2);
stbv_temp_alloc_restore(f,save_point);
}

#if 0

void inverse_mdct_naive(float *buffer, int n)
{
float s;
float A[1 << 12], B[1 << 12], C[1 << 11];
int i,k,k2,k4, n2 = n >> 1, n4 = n >> 2, n8 = n >> 3, l;
int n3_4 = n - n4, ld;


float u[1 << 13], X[1 << 13], v[1 << 13], w[1 << 13];


for (k=k2=0; k < n4; ++k,k2+=2) {
A[k2 ] = (float) cos(4*k*M_PI/n);
A[k2+1] = (float) -sin(4*k*M_PI/n);
B[k2 ] = (float) cos((k2+1)*M_PI/n/2);
B[k2+1] = (float) sin((k2+1)*M_PI/n/2);
}
for (k=k2=0; k < n8; ++k,k2+=2) {
C[k2 ] = (float) cos(2*(k2+1)*M_PI/n);
C[k2+1] = (float) -sin(2*(k2+1)*M_PI/n);
}









for (k=0; k < n2; ++k) u[k] = buffer[k];
for ( ; k < n ; ++k) u[k] = -buffer[n - k - 1];


for (k=k2=k4=0; k < n4; k+=1, k2+=2, k4+=4) {
v[n-k4-1] = (u[k4] - u[n-k4-1]) * A[k2] - (u[k4+2] - u[n-k4-3])*A[k2+1];
v[n-k4-3] = (u[k4] - u[n-k4-1]) * A[k2+1] + (u[k4+2] - u[n-k4-3])*A[k2];
}

for (k=k4=0; k < n8; k+=1, k4+=4) {
w[n2+3+k4] = v[n2+3+k4] + v[k4+3];
w[n2+1+k4] = v[n2+1+k4] + v[k4+1];
w[k4+3] = (v[n2+3+k4] - v[k4+3])*A[n2-4-k4] - (v[n2+1+k4]-v[k4+1])*A[n2-3-k4];
w[k4+1] = (v[n2+1+k4] - v[k4+1])*A[n2-4-k4] + (v[n2+3+k4]-v[k4+3])*A[n2-3-k4];
}

ld = stbv_ilog(n) - 1; 
for (l=0; l < ld-3; ++l) {
int k0 = n >> (l+2), k1 = 1 << (l+3);
int rlim = n >> (l+4), r4, r;
int s2lim = 1 << (l+2), s2;
for (r=r4=0; r < rlim; r4+=4,++r) {
for (s2=0; s2 < s2lim; s2+=2) {
u[n-1-k0*s2-r4] = w[n-1-k0*s2-r4] + w[n-1-k0*(s2+1)-r4];
u[n-3-k0*s2-r4] = w[n-3-k0*s2-r4] + w[n-3-k0*(s2+1)-r4];
u[n-1-k0*(s2+1)-r4] = (w[n-1-k0*s2-r4] - w[n-1-k0*(s2+1)-r4]) * A[r*k1]
- (w[n-3-k0*s2-r4] - w[n-3-k0*(s2+1)-r4]) * A[r*k1+1];
u[n-3-k0*(s2+1)-r4] = (w[n-3-k0*s2-r4] - w[n-3-k0*(s2+1)-r4]) * A[r*k1]
+ (w[n-1-k0*s2-r4] - w[n-1-k0*(s2+1)-r4]) * A[r*k1+1];
}
}
if (l+1 < ld-3) {

memcpy(w, u, sizeof(u));
}
}


for (i=0; i < n8; ++i) {
int j = stbv_bit_reverse(i) >> (32-ld+3);
assert(j < n8);
if (i == j) {


int i8 = i << 3;
v[i8+1] = u[i8+1];
v[i8+3] = u[i8+3];
v[i8+5] = u[i8+5];
v[i8+7] = u[i8+7];
} else if (i < j) {
int i8 = i << 3, j8 = j << 3;
v[j8+1] = u[i8+1], v[i8+1] = u[j8 + 1];
v[j8+3] = u[i8+3], v[i8+3] = u[j8 + 3];
v[j8+5] = u[i8+5], v[i8+5] = u[j8 + 5];
v[j8+7] = u[i8+7], v[i8+7] = u[j8 + 7];
}
}

for (k=0; k < n2; ++k) {
w[k] = v[k*2+1];
}

for (k=k2=k4=0; k < n8; ++k, k2 += 2, k4 += 4) {
u[n-1-k2] = w[k4];
u[n-2-k2] = w[k4+1];
u[n3_4 - 1 - k2] = w[k4+2];
u[n3_4 - 2 - k2] = w[k4+3];
}

for (k=k2=0; k < n8; ++k, k2 += 2) {
v[n2 + k2 ] = ( u[n2 + k2] + u[n-2-k2] + C[k2+1]*(u[n2+k2]-u[n-2-k2]) + C[k2]*(u[n2+k2+1]+u[n-2-k2+1]))/2;
v[n-2 - k2] = ( u[n2 + k2] + u[n-2-k2] - C[k2+1]*(u[n2+k2]-u[n-2-k2]) - C[k2]*(u[n2+k2+1]+u[n-2-k2+1]))/2;
v[n2+1+ k2] = ( u[n2+1+k2] - u[n-1-k2] + C[k2+1]*(u[n2+1+k2]+u[n-1-k2]) - C[k2]*(u[n2+k2]-u[n-2-k2]))/2;
v[n-1 - k2] = (-u[n2+1+k2] + u[n-1-k2] + C[k2+1]*(u[n2+1+k2]+u[n-1-k2]) - C[k2]*(u[n2+k2]-u[n-2-k2]))/2;
}

for (k=k2=0; k < n4; ++k,k2 += 2) {
X[k] = v[k2+n2]*B[k2 ] + v[k2+1+n2]*B[k2+1];
X[n2-1-k] = v[k2+n2]*B[k2+1] - v[k2+1+n2]*B[k2 ];
}





s = 0.5; 




for (i=0; i < n4 ; ++i) buffer[i] = s * X[i+n4];
for ( ; i < n3_4; ++i) buffer[i] = -s * X[n3_4 - i - 1];
for ( ; i < n ; ++i) buffer[i] = -s * X[i - n3_4];
}
#endif

static float *stbv_get_window(stbv_vorb *f, int len)
{
len <<= 1;
if (len == f->blocksize_0) return f->window[0];
if (len == f->blocksize_1) return f->window[1];
assert(0);
return NULL;
}

#if !defined(STB_VORBIS_NO_DEFER_FLOOR)
typedef stbv_int16 STBV_YTYPE;
#else
typedef int STBV_YTYPE;
#endif
static int stbv_do_floor(stbv_vorb *f, StbvMapping *map, int i, int n, float *target, STBV_YTYPE *finalY, stbv_uint8 *step2_flag)
{
int n2 = n >> 1;
int s = map->chan[i].mux, floor;
floor = map->submap_floor[s];
if (f->floor_types[floor] == 0) {
return stbv_error(f, VORBIS_invalid_stream);
} else {
StbvFloor1 *g = &f->floor_config[floor].floor1;
int j,q;
int lx = 0, ly = finalY[0] * g->floor1_multiplier;
for (q=1; q < g->values; ++q) {
j = g->sorted_order[q];
#if !defined(STB_VORBIS_NO_DEFER_FLOOR)
if (finalY[j] >= 0)
#else
if (step2_flag[j])
#endif
{
int hy = finalY[j] * g->floor1_multiplier;
int hx = g->Xlist[j];
if (lx != hx)
stbv_draw_line(target, lx,ly, hx,hy, n2);
STBV_CHECK(f);
lx = hx, ly = hy;
}
}
if (lx < n2) {

for (j=lx; j < n2; ++j)
STBV_LINE_OP(target[j], stbv_inverse_db_table[ly]);
STBV_CHECK(f);
}
}
return TRUE;
}















static int stbv_vorbis_decode_initial(stbv_vorb *f, int *p_left_start, int *p_left_end, int *p_right_start, int *p_right_end, int *mode)
{
StbvMode *m;
int i, n, prev, next, window_center;
f->channel_buffer_start = f->channel_buffer_end = 0;

retry:
if (f->eof) return FALSE;
if (!stbv_maybe_start_packet(f))
return FALSE;

if (stbv_get_bits(f,1) != 0) {
if (STBV_IS_PUSH_MODE(f))
return stbv_error(f,VORBIS_bad_packet_type);
while (STBV_EOP != stbv_get8_packet(f));
goto retry;
}

if (f->alloc.alloc_buffer)
assert(f->alloc.alloc_buffer_length_in_bytes == f->temp_offset);

i = stbv_get_bits(f, stbv_ilog(f->mode_count-1));
if (i == STBV_EOP) return FALSE;
if (i >= f->mode_count) return FALSE;
*mode = i;
m = f->mode_config + i;
if (m->blockflag) {
n = f->blocksize_1;
prev = stbv_get_bits(f,1);
next = stbv_get_bits(f,1);
} else {
prev = next = 0;
n = f->blocksize_0;
}



window_center = n >> 1;
if (m->blockflag && !prev) {
*p_left_start = (n - f->blocksize_0) >> 2;
*p_left_end = (n + f->blocksize_0) >> 2;
} else {
*p_left_start = 0;
*p_left_end = window_center;
}
if (m->blockflag && !next) {
*p_right_start = (n*3 - f->blocksize_0) >> 2;
*p_right_end = (n*3 + f->blocksize_0) >> 2;
} else {
*p_right_start = window_center;
*p_right_end = n;
}

return TRUE;
}

static int stbv_vorbis_decode_packet_rest(stbv_vorb *f, int *len, StbvMode *m, int left_start, int left_end, int right_start, int right_end, int *p_left)
{
StbvMapping *map;
int i,j,k,n,n2;
int zero_channel[256];
int really_zero_channel[256];



n = f->blocksize[m->blockflag];
map = &f->mapping[m->mapping];


n2 = n >> 1;

STBV_CHECK(f);

for (i=0; i < f->channels; ++i) {
int s = map->chan[i].mux, floor;
zero_channel[i] = FALSE;
floor = map->submap_floor[s];
if (f->floor_types[floor] == 0) {
return stbv_error(f, VORBIS_invalid_stream);
} else {
StbvFloor1 *g = &f->floor_config[floor].floor1;
if (stbv_get_bits(f, 1)) {
short *finalY;
stbv_uint8 step2_flag[256];
static int range_list[4] = { 256, 128, 86, 64 };
int range = range_list[g->floor1_multiplier-1];
int offset = 2;
finalY = f->finalY[i];
finalY[0] = stbv_get_bits(f, stbv_ilog(range)-1);
finalY[1] = stbv_get_bits(f, stbv_ilog(range)-1);
for (j=0; j < g->partitions; ++j) {
int pclass = g->partition_class_list[j];
int cdim = g->class_dimensions[pclass];
int cbits = g->class_subclasses[pclass];
int csub = (1 << cbits)-1;
int cval = 0;
if (cbits) {
StbvCodebook *c = f->codebooks + g->class_masterbooks[pclass];
STBV_DECODE(cval,f,c);
}
for (k=0; k < cdim; ++k) {
int book = g->subclass_books[pclass][cval & csub];
cval = cval >> cbits;
if (book >= 0) {
int temp;
StbvCodebook *c = f->codebooks + book;
STBV_DECODE(temp,f,c);
finalY[offset++] = temp;
} else
finalY[offset++] = 0;
}
}
if (f->valid_bits == STBV_INVALID_BITS) goto error; 
step2_flag[0] = step2_flag[1] = 1;
for (j=2; j < g->values; ++j) {
int low, high, pred, highroom, lowroom, room, val;
low = g->stbv_neighbors[j][0];
high = g->stbv_neighbors[j][1];

pred = stbv_predict_point(g->Xlist[j], g->Xlist[low], g->Xlist[high], finalY[low], finalY[high]);
val = finalY[j];
highroom = range - pred;
lowroom = pred;
if (highroom < lowroom)
room = highroom * 2;
else
room = lowroom * 2;
if (val) {
step2_flag[low] = step2_flag[high] = 1;
step2_flag[j] = 1;
if (val >= room)
if (highroom > lowroom)
finalY[j] = val - lowroom + pred;
else
finalY[j] = pred - val + highroom - 1;
else
if (val & 1)
finalY[j] = pred - ((val+1)>>1);
else
finalY[j] = pred + (val>>1);
} else {
step2_flag[j] = 0;
finalY[j] = pred;
}
}

#if defined(STB_VORBIS_NO_DEFER_FLOOR)
stbv_do_floor(f, map, i, n, f->floor_buffers[i], finalY, step2_flag);
#else

for (j=0; j < g->values; ++j) {
if (!step2_flag[j])
finalY[j] = -1;
}
#endif
} else {
error:
zero_channel[i] = TRUE;
}



}
}
STBV_CHECK(f);


if (f->alloc.alloc_buffer)
assert(f->alloc.alloc_buffer_length_in_bytes == f->temp_offset);


memcpy(really_zero_channel, zero_channel, sizeof(really_zero_channel[0]) * f->channels);
for (i=0; i < map->coupling_steps; ++i)
if (!zero_channel[map->chan[i].magnitude] || !zero_channel[map->chan[i].angle]) {
zero_channel[map->chan[i].magnitude] = zero_channel[map->chan[i].angle] = FALSE;
}

STBV_CHECK(f);

for (i=0; i < map->submaps; ++i) {
float *residue_buffers[STB_VORBIS_MAX_CHANNELS];
int r;
stbv_uint8 do_not_decode[256];
int ch = 0;
for (j=0; j < f->channels; ++j) {
if (map->chan[j].mux == i) {
if (zero_channel[j]) {
do_not_decode[ch] = TRUE;
residue_buffers[ch] = NULL;
} else {
do_not_decode[ch] = FALSE;
residue_buffers[ch] = f->channel_buffers[j];
}
++ch;
}
}
r = map->submap_residue[i];
stbv_decode_residue(f, residue_buffers, ch, n2, r, do_not_decode);
}

if (f->alloc.alloc_buffer)
assert(f->alloc.alloc_buffer_length_in_bytes == f->temp_offset);
STBV_CHECK(f);


for (i = map->coupling_steps-1; i >= 0; --i) {
int n2 = n >> 1;
float *m = f->channel_buffers[map->chan[i].magnitude];
float *a = f->channel_buffers[map->chan[i].angle ];
for (j=0; j < n2; ++j) {
float a2,m2;
if (m[j] > 0)
if (a[j] > 0)
m2 = m[j], a2 = m[j] - a[j];
else
a2 = m[j], m2 = m[j] + a[j];
else
if (a[j] > 0)
m2 = m[j], a2 = m[j] + a[j];
else
a2 = m[j], m2 = m[j] - a[j];
m[j] = m2;
a[j] = a2;
}
}
STBV_CHECK(f);


#if !defined(STB_VORBIS_NO_DEFER_FLOOR)
for (i=0; i < f->channels; ++i) {
if (really_zero_channel[i]) {
memset(f->channel_buffers[i], 0, sizeof(*f->channel_buffers[i]) * n2);
} else {
stbv_do_floor(f, map, i, n, f->channel_buffers[i], f->finalY[i], NULL);
}
}
#else
for (i=0; i < f->channels; ++i) {
if (really_zero_channel[i]) {
memset(f->channel_buffers[i], 0, sizeof(*f->channel_buffers[i]) * n2);
} else {
for (j=0; j < n2; ++j)
f->channel_buffers[i][j] *= f->floor_buffers[i][j];
}
}
#endif


STBV_CHECK(f);
for (i=0; i < f->channels; ++i)
stbv_inverse_mdct(f->channel_buffers[i], n, f, m->blockflag);
STBV_CHECK(f);



stbv_flush_packet(f);

if (f->first_decode) {




f->current_loc = -n2; 


f->discard_samples_deferred = n - right_end;
f->current_loc_valid = TRUE;
f->first_decode = FALSE;
} else if (f->discard_samples_deferred) {
if (f->discard_samples_deferred >= right_start - left_start) {
f->discard_samples_deferred -= (right_start - left_start);
left_start = right_start;
*p_left = left_start;
} else {
left_start += f->discard_samples_deferred;
*p_left = left_start;
f->discard_samples_deferred = 0;
}
} else if (f->previous_length == 0 && f->current_loc_valid) {






}


if (f->last_seg_which == f->end_seg_with_known_loc) {

if (f->current_loc_valid && (f->page_flag & STBV_PAGEFLAG_last_page)) {
stbv_uint32 current_end = f->known_loc_for_packet;

if (current_end < f->current_loc + (right_end-left_start)) {
if (current_end < f->current_loc) {

*len = 0;
} else {
*len = current_end - f->current_loc;
}
*len += left_start; 
if (*len > right_end) *len = right_end; 
f->current_loc += *len;
return TRUE;
}
}




f->current_loc = f->known_loc_for_packet - (n2-left_start);
f->current_loc_valid = TRUE;
}
if (f->current_loc_valid)
f->current_loc += (right_start - left_start);

if (f->alloc.alloc_buffer)
assert(f->alloc.alloc_buffer_length_in_bytes == f->temp_offset);
*len = right_end; 
STBV_CHECK(f);

return TRUE;
}

static int stbv_vorbis_decode_packet(stbv_vorb *f, int *len, int *p_left, int *p_right)
{
int mode, left_end, right_end;
if (!stbv_vorbis_decode_initial(f, p_left, &left_end, p_right, &right_end, &mode)) return 0;
return stbv_vorbis_decode_packet_rest(f, len, f->mode_config + mode, *p_left, left_end, *p_right, right_end, p_left);
}

static int stbv_vorbis_finish_frame(stb_vorbis *f, int len, int left, int right)
{
int prev,i,j;









if (f->previous_length) {
int i,j, n = f->previous_length;
float *w = stbv_get_window(f, n);
for (i=0; i < f->channels; ++i) {
for (j=0; j < n; ++j)
f->channel_buffers[i][left+j] =
f->channel_buffers[i][left+j]*w[ j] +
f->previous_window[i][ j]*w[n-1-j];
}
}

prev = f->previous_length;


f->previous_length = len - right;







for (i=0; i < f->channels; ++i)
for (j=0; right+j < len; ++j)
f->previous_window[i][j] = f->channel_buffers[i][right+j];

if (!prev)



return 0;


if (len < right) right = len;

f->samples_output += right-left;

return right - left;
}

static int stbv_vorbis_pump_first_frame(stb_vorbis *f)
{
int len, right, left, res;
res = stbv_vorbis_decode_packet(f, &len, &left, &right);
if (res)
stbv_vorbis_finish_frame(f, len, left, right);
return res;
}

#if !defined(STB_VORBIS_NO_PUSHDATA_API)
static int stbv_is_whole_packet_present(stb_vorbis *f, int end_page)
{








int s = f->next_seg, first = TRUE;
stbv_uint8 *p = f->stream;

if (s != -1) { 
for (; s < f->segment_count; ++s) {
p += f->segments[s];
if (f->segments[s] < 255) 
break;
}

if (end_page)
if (s < f->segment_count-1) return stbv_error(f, VORBIS_invalid_stream);
if (s == f->segment_count)
s = -1; 
if (p > f->stream_end) return stbv_error(f, VORBIS_need_more_data);
first = FALSE;
}
for (; s == -1;) {
stbv_uint8 *q; 
int n;


if (p + 26 >= f->stream_end) return stbv_error(f, VORBIS_need_more_data);

if (memcmp(p, stbv_ogg_page_header, 4)) return stbv_error(f, VORBIS_invalid_stream);
if (p[4] != 0) return stbv_error(f, VORBIS_invalid_stream);
if (first) { 
if (f->previous_length)
if ((p[5] & STBV_PAGEFLAG_continued_packet)) return stbv_error(f, VORBIS_invalid_stream);


} else {
if (!(p[5] & STBV_PAGEFLAG_continued_packet)) return stbv_error(f, VORBIS_invalid_stream);
}
n = p[26]; 
q = p+27; 
p = q + n; 

if (p > f->stream_end) return stbv_error(f, VORBIS_need_more_data);
for (s=0; s < n; ++s) {
p += q[s];
if (q[s] < 255)
break;
}
if (end_page)
if (s < n-1) return stbv_error(f, VORBIS_invalid_stream);
if (s == n)
s = -1; 
if (p > f->stream_end) return stbv_error(f, VORBIS_need_more_data);
first = FALSE;
}
return TRUE;
}
#endif 

static int stbv_start_decoder(stbv_vorb *f)
{
stbv_uint8 header[6], x,y;
int len,i,j,k, max_submaps = 0;
int longest_floorlist=0;



if (!stbv_start_page(f)) return FALSE;

if (!(f->page_flag & STBV_PAGEFLAG_first_page)) return stbv_error(f, VORBIS_invalid_first_page);
if (f->page_flag & STBV_PAGEFLAG_last_page) return stbv_error(f, VORBIS_invalid_first_page);
if (f->page_flag & STBV_PAGEFLAG_continued_packet) return stbv_error(f, VORBIS_invalid_first_page);

if (f->segment_count != 1) return stbv_error(f, VORBIS_invalid_first_page);
if (f->segments[0] != 30) return stbv_error(f, VORBIS_invalid_first_page);


if (stbv_get8(f) != STBV_VORBIS_packet_id) return stbv_error(f, VORBIS_invalid_first_page);
if (!stbv_getn(f, header, 6)) return stbv_error(f, VORBIS_unexpected_eof);
if (!stbv_vorbis_validate(header)) return stbv_error(f, VORBIS_invalid_first_page);

if (stbv_get32(f) != 0) return stbv_error(f, VORBIS_invalid_first_page);
f->channels = stbv_get8(f); if (!f->channels) return stbv_error(f, VORBIS_invalid_first_page);
if (f->channels > STB_VORBIS_MAX_CHANNELS) return stbv_error(f, VORBIS_too_many_channels);
f->sample_rate = stbv_get32(f); if (!f->sample_rate) return stbv_error(f, VORBIS_invalid_first_page);
stbv_get32(f); 
stbv_get32(f); 
stbv_get32(f); 
x = stbv_get8(f);
{
int log0,log1;
log0 = x & 15;
log1 = x >> 4;
f->blocksize_0 = 1 << log0;
f->blocksize_1 = 1 << log1;
if (log0 < 6 || log0 > 13) return stbv_error(f, VORBIS_invalid_setup);
if (log1 < 6 || log1 > 13) return stbv_error(f, VORBIS_invalid_setup);
if (log0 > log1) return stbv_error(f, VORBIS_invalid_setup);
}


x = stbv_get8(f);
if (!(x & 1)) return stbv_error(f, VORBIS_invalid_first_page);


if (!stbv_start_page(f)) return FALSE;

if (!stbv_start_packet(f)) return FALSE;
do {
len = stbv_next_segment(f);
stbv_skip(f, len);
f->bytes_in_seg = 0;
} while (len);


if (!stbv_start_packet(f)) return FALSE;

#if !defined(STB_VORBIS_NO_PUSHDATA_API)
if (STBV_IS_PUSH_MODE(f)) {
if (!stbv_is_whole_packet_present(f, TRUE)) {

if (f->error == VORBIS_invalid_stream)
f->error = VORBIS_invalid_setup;
return FALSE;
}
}
#endif

stbv_crc32_init(); 

if (stbv_get8_packet(f) != STBV_VORBIS_packet_setup) return stbv_error(f, VORBIS_invalid_setup);
for (i=0; i < 6; ++i) header[i] = stbv_get8_packet(f);
if (!stbv_vorbis_validate(header)) return stbv_error(f, VORBIS_invalid_setup);



f->codebook_count = stbv_get_bits(f,8) + 1;
f->codebooks = (StbvCodebook *) stbv_setup_malloc(f, sizeof(*f->codebooks) * f->codebook_count);
if (f->codebooks == NULL) return stbv_error(f, VORBIS_outofmem);
memset(f->codebooks, 0, sizeof(*f->codebooks) * f->codebook_count);
for (i=0; i < f->codebook_count; ++i) {
stbv_uint32 *values;
int ordered, sorted_count;
int total=0;
stbv_uint8 *lengths;
StbvCodebook *c = f->codebooks+i;
STBV_CHECK(f);
x = stbv_get_bits(f, 8); if (x != 0x42) return stbv_error(f, VORBIS_invalid_setup);
x = stbv_get_bits(f, 8); if (x != 0x43) return stbv_error(f, VORBIS_invalid_setup);
x = stbv_get_bits(f, 8); if (x != 0x56) return stbv_error(f, VORBIS_invalid_setup);
x = stbv_get_bits(f, 8);
c->dimensions = (stbv_get_bits(f, 8)<<8) + x;
x = stbv_get_bits(f, 8);
y = stbv_get_bits(f, 8);
c->entries = (stbv_get_bits(f, 8)<<16) + (y<<8) + x;
ordered = stbv_get_bits(f,1);
c->sparse = ordered ? 0 : stbv_get_bits(f,1);

if (c->dimensions == 0 && c->entries != 0) return stbv_error(f, VORBIS_invalid_setup);

if (c->sparse)
lengths = (stbv_uint8 *) stbv_setup_temp_malloc(f, c->entries);
else
lengths = c->codeword_lengths = (stbv_uint8 *) stbv_setup_malloc(f, c->entries);

if (!lengths) return stbv_error(f, VORBIS_outofmem);

if (ordered) {
int current_entry = 0;
int current_length = stbv_get_bits(f,5) + 1;
while (current_entry < c->entries) {
int limit = c->entries - current_entry;
int n = stbv_get_bits(f, stbv_ilog(limit));
if (current_entry + n > (int) c->entries) { return stbv_error(f, VORBIS_invalid_setup); }
memset(lengths + current_entry, current_length, n);
current_entry += n;
++current_length;
}
} else {
for (j=0; j < c->entries; ++j) {
int present = c->sparse ? stbv_get_bits(f,1) : 1;
if (present) {
lengths[j] = stbv_get_bits(f, 5) + 1;
++total;
if (lengths[j] == 32)
return stbv_error(f, VORBIS_invalid_setup);
} else {
lengths[j] = NO_CODE;
}
}
}

if (c->sparse && total >= c->entries >> 2) {

if (c->entries > (int) f->setup_temp_memory_required)
f->setup_temp_memory_required = c->entries;

c->codeword_lengths = (stbv_uint8 *) stbv_setup_malloc(f, c->entries);
if (c->codeword_lengths == NULL) return stbv_error(f, VORBIS_outofmem);
memcpy(c->codeword_lengths, lengths, c->entries);
stbv_setup_temp_free(f, lengths, c->entries); 
lengths = c->codeword_lengths;
c->sparse = 0;
}


if (c->sparse) {
sorted_count = total;
} else {
sorted_count = 0;
#if !defined(STB_VORBIS_NO_HUFFMAN_BINARY_SEARCH)
for (j=0; j < c->entries; ++j)
if (lengths[j] > STB_VORBIS_FAST_HUFFMAN_LENGTH && lengths[j] != NO_CODE)
++sorted_count;
#endif
}

c->sorted_entries = sorted_count;
values = NULL;

STBV_CHECK(f);
if (!c->sparse) {
c->codewords = (stbv_uint32 *) stbv_setup_malloc(f, sizeof(c->codewords[0]) * c->entries);
if (!c->codewords) return stbv_error(f, VORBIS_outofmem);
} else {
unsigned int size;
if (c->sorted_entries) {
c->codeword_lengths = (stbv_uint8 *) stbv_setup_malloc(f, c->sorted_entries);
if (!c->codeword_lengths) return stbv_error(f, VORBIS_outofmem);
c->codewords = (stbv_uint32 *) stbv_setup_temp_malloc(f, sizeof(*c->codewords) * c->sorted_entries);
if (!c->codewords) return stbv_error(f, VORBIS_outofmem);
values = (stbv_uint32 *) stbv_setup_temp_malloc(f, sizeof(*values) * c->sorted_entries);
if (!values) return stbv_error(f, VORBIS_outofmem);
}
size = c->entries + (sizeof(*c->codewords) + sizeof(*values)) * c->sorted_entries;
if (size > f->setup_temp_memory_required)
f->setup_temp_memory_required = size;
}

if (!stbv_compute_codewords(c, lengths, c->entries, values)) {
if (c->sparse) stbv_setup_temp_free(f, values, 0);
return stbv_error(f, VORBIS_invalid_setup);
}

if (c->sorted_entries) {

c->sorted_codewords = (stbv_uint32 *) stbv_setup_malloc(f, sizeof(*c->sorted_codewords) * (c->sorted_entries+1));
if (c->sorted_codewords == NULL) return stbv_error(f, VORBIS_outofmem);


c->sorted_values = ( int *) stbv_setup_malloc(f, sizeof(*c->sorted_values ) * (c->sorted_entries+1));
if (c->sorted_values == NULL) return stbv_error(f, VORBIS_outofmem);
++c->sorted_values;
c->sorted_values[-1] = -1;
stbv_compute_sorted_huffman(c, lengths, values);
}

if (c->sparse) {
stbv_setup_temp_free(f, values, sizeof(*values)*c->sorted_entries);
stbv_setup_temp_free(f, c->codewords, sizeof(*c->codewords)*c->sorted_entries);
stbv_setup_temp_free(f, lengths, c->entries);
c->codewords = NULL;
}

stbv_compute_accelerated_huffman(c);

STBV_CHECK(f);
c->lookup_type = stbv_get_bits(f, 4);
if (c->lookup_type > 2) return stbv_error(f, VORBIS_invalid_setup);
if (c->lookup_type > 0) {
stbv_uint16 *mults;
c->minimum_value = stbv_float32_unpack(stbv_get_bits(f, 32));
c->delta_value = stbv_float32_unpack(stbv_get_bits(f, 32));
c->value_bits = stbv_get_bits(f, 4)+1;
c->sequence_p = stbv_get_bits(f,1);
if (c->lookup_type == 1) {
c->lookup_values = stbv_lookup1_values(c->entries, c->dimensions);
} else {
c->lookup_values = c->entries * c->dimensions;
}
if (c->lookup_values == 0) return stbv_error(f, VORBIS_invalid_setup);
mults = (stbv_uint16 *) stbv_setup_temp_malloc(f, sizeof(mults[0]) * c->lookup_values);
if (mults == NULL) return stbv_error(f, VORBIS_outofmem);
for (j=0; j < (int) c->lookup_values; ++j) {
int q = stbv_get_bits(f, c->value_bits);
if (q == STBV_EOP) { stbv_setup_temp_free(f,mults,sizeof(mults[0])*c->lookup_values); return stbv_error(f, VORBIS_invalid_setup); }
mults[j] = q;
}

#if !defined(STB_VORBIS_DIVIDES_IN_CODEBOOK)
if (c->lookup_type == 1) {
int len, sparse = c->sparse;
float last=0;

if (sparse) {
if (c->sorted_entries == 0) goto stbv_skip;
c->multiplicands = (stbv_codetype *) stbv_setup_malloc(f, sizeof(c->multiplicands[0]) * c->sorted_entries * c->dimensions);
} else
c->multiplicands = (stbv_codetype *) stbv_setup_malloc(f, sizeof(c->multiplicands[0]) * c->entries * c->dimensions);
if (c->multiplicands == NULL) { stbv_setup_temp_free(f,mults,sizeof(mults[0])*c->lookup_values); return stbv_error(f, VORBIS_outofmem); }
len = sparse ? c->sorted_entries : c->entries;
for (j=0; j < len; ++j) {
unsigned int z = sparse ? c->sorted_values[j] : j;
unsigned int div=1;
for (k=0; k < c->dimensions; ++k) {
int off = (z / div) % c->lookup_values;
float val = mults[off];
val = mults[off]*c->delta_value + c->minimum_value + last;
c->multiplicands[j*c->dimensions + k] = val;
if (c->sequence_p)
last = val;
if (k+1 < c->dimensions) {
if (div > UINT_MAX / (unsigned int) c->lookup_values) {
stbv_setup_temp_free(f, mults,sizeof(mults[0])*c->lookup_values);
return stbv_error(f, VORBIS_invalid_setup);
}
div *= c->lookup_values;
}
}
}
c->lookup_type = 2;
}
else
#endif
{
float last=0;
STBV_CHECK(f);
c->multiplicands = (stbv_codetype *) stbv_setup_malloc(f, sizeof(c->multiplicands[0]) * c->lookup_values);
if (c->multiplicands == NULL) { stbv_setup_temp_free(f, mults,sizeof(mults[0])*c->lookup_values); return stbv_error(f, VORBIS_outofmem); }
for (j=0; j < (int) c->lookup_values; ++j) {
float val = mults[j] * c->delta_value + c->minimum_value + last;
c->multiplicands[j] = val;
if (c->sequence_p)
last = val;
}
}
#if !defined(STB_VORBIS_DIVIDES_IN_CODEBOOK)
stbv_skip:;
#endif
stbv_setup_temp_free(f, mults, sizeof(mults[0])*c->lookup_values);

STBV_CHECK(f);
}
STBV_CHECK(f);
}



x = stbv_get_bits(f, 6) + 1;
for (i=0; i < x; ++i) {
stbv_uint32 z = stbv_get_bits(f, 16);
if (z != 0) return stbv_error(f, VORBIS_invalid_setup);
}


f->floor_count = stbv_get_bits(f, 6)+1;
f->floor_config = (StbvFloor *) stbv_setup_malloc(f, f->floor_count * sizeof(*f->floor_config));
if (f->floor_config == NULL) return stbv_error(f, VORBIS_outofmem);
for (i=0; i < f->floor_count; ++i) {
f->floor_types[i] = stbv_get_bits(f, 16);
if (f->floor_types[i] > 1) return stbv_error(f, VORBIS_invalid_setup);
if (f->floor_types[i] == 0) {
StbvFloor0 *g = &f->floor_config[i].floor0;
g->order = stbv_get_bits(f,8);
g->rate = stbv_get_bits(f,16);
g->bark_map_size = stbv_get_bits(f,16);
g->amplitude_bits = stbv_get_bits(f,6);
g->amplitude_offset = stbv_get_bits(f,8);
g->number_of_books = stbv_get_bits(f,4) + 1;
for (j=0; j < g->number_of_books; ++j)
g->book_list[j] = stbv_get_bits(f,8);
return stbv_error(f, VORBIS_feature_not_supported);
} else {
stbv_floor_ordering p[31*8+2];
StbvFloor1 *g = &f->floor_config[i].floor1;
int max_class = -1; 
g->partitions = stbv_get_bits(f, 5);
for (j=0; j < g->partitions; ++j) {
g->partition_class_list[j] = stbv_get_bits(f, 4);
if (g->partition_class_list[j] > max_class)
max_class = g->partition_class_list[j];
}
for (j=0; j <= max_class; ++j) {
g->class_dimensions[j] = stbv_get_bits(f, 3)+1;
g->class_subclasses[j] = stbv_get_bits(f, 2);
if (g->class_subclasses[j]) {
g->class_masterbooks[j] = stbv_get_bits(f, 8);
if (g->class_masterbooks[j] >= f->codebook_count) return stbv_error(f, VORBIS_invalid_setup);
}
for (k=0; k < 1 << g->class_subclasses[j]; ++k) {
g->subclass_books[j][k] = stbv_get_bits(f,8)-1;
if (g->subclass_books[j][k] >= f->codebook_count) return stbv_error(f, VORBIS_invalid_setup);
}
}
g->floor1_multiplier = stbv_get_bits(f,2)+1;
g->rangebits = stbv_get_bits(f,4);
g->Xlist[0] = 0;
g->Xlist[1] = 1 << g->rangebits;
g->values = 2;
for (j=0; j < g->partitions; ++j) {
int c = g->partition_class_list[j];
for (k=0; k < g->class_dimensions[c]; ++k) {
g->Xlist[g->values] = stbv_get_bits(f, g->rangebits);
++g->values;
}
}

for (j=0; j < g->values; ++j) {
p[j].x = g->Xlist[j];
p[j].id = j;
}
qsort(p, g->values, sizeof(p[0]), stbv_point_compare);
for (j=0; j < g->values; ++j)
g->sorted_order[j] = (stbv_uint8) p[j].id;

for (j=2; j < g->values; ++j) {
int low,hi;
stbv_neighbors(g->Xlist, j, &low,&hi);
g->stbv_neighbors[j][0] = low;
g->stbv_neighbors[j][1] = hi;
}

if (g->values > longest_floorlist)
longest_floorlist = g->values;
}
}


f->residue_count = stbv_get_bits(f, 6)+1;
f->residue_config = (StbvResidue *) stbv_setup_malloc(f, f->residue_count * sizeof(f->residue_config[0]));
if (f->residue_config == NULL) return stbv_error(f, VORBIS_outofmem);
memset(f->residue_config, 0, f->residue_count * sizeof(f->residue_config[0]));
for (i=0; i < f->residue_count; ++i) {
stbv_uint8 residue_cascade[64];
StbvResidue *r = f->residue_config+i;
f->residue_types[i] = stbv_get_bits(f, 16);
if (f->residue_types[i] > 2) return stbv_error(f, VORBIS_invalid_setup);
r->begin = stbv_get_bits(f, 24);
r->end = stbv_get_bits(f, 24);
if (r->end < r->begin) return stbv_error(f, VORBIS_invalid_setup);
r->part_size = stbv_get_bits(f,24)+1;
r->classifications = stbv_get_bits(f,6)+1;
r->classbook = stbv_get_bits(f,8);
if (r->classbook >= f->codebook_count) return stbv_error(f, VORBIS_invalid_setup);
for (j=0; j < r->classifications; ++j) {
stbv_uint8 high_bits=0;
stbv_uint8 low_bits=stbv_get_bits(f,3);
if (stbv_get_bits(f,1))
high_bits = stbv_get_bits(f,5);
residue_cascade[j] = high_bits*8 + low_bits;
}
r->residue_books = (short (*)[8]) stbv_setup_malloc(f, sizeof(r->residue_books[0]) * r->classifications);
if (r->residue_books == NULL) return stbv_error(f, VORBIS_outofmem);
for (j=0; j < r->classifications; ++j) {
for (k=0; k < 8; ++k) {
if (residue_cascade[j] & (1 << k)) {
r->residue_books[j][k] = stbv_get_bits(f, 8);
if (r->residue_books[j][k] >= f->codebook_count) return stbv_error(f, VORBIS_invalid_setup);
} else {
r->residue_books[j][k] = -1;
}
}
}


r->classdata = (stbv_uint8 **) stbv_setup_malloc(f, sizeof(*r->classdata) * f->codebooks[r->classbook].entries);
if (!r->classdata) return stbv_error(f, VORBIS_outofmem);
memset(r->classdata, 0, sizeof(*r->classdata) * f->codebooks[r->classbook].entries);
for (j=0; j < f->codebooks[r->classbook].entries; ++j) {
int classwords = f->codebooks[r->classbook].dimensions;
int temp = j;
r->classdata[j] = (stbv_uint8 *) stbv_setup_malloc(f, sizeof(r->classdata[j][0]) * classwords);
if (r->classdata[j] == NULL) return stbv_error(f, VORBIS_outofmem);
for (k=classwords-1; k >= 0; --k) {
r->classdata[j][k] = temp % r->classifications;
temp /= r->classifications;
}
}
}

f->mapping_count = stbv_get_bits(f,6)+1;
f->mapping = (StbvMapping *) stbv_setup_malloc(f, f->mapping_count * sizeof(*f->mapping));
if (f->mapping == NULL) return stbv_error(f, VORBIS_outofmem);
memset(f->mapping, 0, f->mapping_count * sizeof(*f->mapping));
for (i=0; i < f->mapping_count; ++i) {
StbvMapping *m = f->mapping + i; 
int mapping_type = stbv_get_bits(f,16);
if (mapping_type != 0) return stbv_error(f, VORBIS_invalid_setup);
m->chan = (StbvMappingChannel *) stbv_setup_malloc(f, f->channels * sizeof(*m->chan));
if (m->chan == NULL) return stbv_error(f, VORBIS_outofmem);
if (stbv_get_bits(f,1))
m->submaps = stbv_get_bits(f,4)+1;
else
m->submaps = 1;
if (m->submaps > max_submaps)
max_submaps = m->submaps;
if (stbv_get_bits(f,1)) {
m->coupling_steps = stbv_get_bits(f,8)+1;
for (k=0; k < m->coupling_steps; ++k) {
m->chan[k].magnitude = stbv_get_bits(f, stbv_ilog(f->channels-1));
m->chan[k].angle = stbv_get_bits(f, stbv_ilog(f->channels-1));
if (m->chan[k].magnitude >= f->channels) return stbv_error(f, VORBIS_invalid_setup);
if (m->chan[k].angle >= f->channels) return stbv_error(f, VORBIS_invalid_setup);
if (m->chan[k].magnitude == m->chan[k].angle) return stbv_error(f, VORBIS_invalid_setup);
}
} else
m->coupling_steps = 0;


if (stbv_get_bits(f,2)) return stbv_error(f, VORBIS_invalid_setup);
if (m->submaps > 1) {
for (j=0; j < f->channels; ++j) {
m->chan[j].mux = stbv_get_bits(f, 4);
if (m->chan[j].mux >= m->submaps) return stbv_error(f, VORBIS_invalid_setup);
}
} else

for (j=0; j < f->channels; ++j)
m->chan[j].mux = 0;

for (j=0; j < m->submaps; ++j) {
stbv_get_bits(f,8); 
m->submap_floor[j] = stbv_get_bits(f,8);
m->submap_residue[j] = stbv_get_bits(f,8);
if (m->submap_floor[j] >= f->floor_count) return stbv_error(f, VORBIS_invalid_setup);
if (m->submap_residue[j] >= f->residue_count) return stbv_error(f, VORBIS_invalid_setup);
}
}


f->mode_count = stbv_get_bits(f, 6)+1;
for (i=0; i < f->mode_count; ++i) {
StbvMode *m = f->mode_config+i;
m->blockflag = stbv_get_bits(f,1);
m->windowtype = stbv_get_bits(f,16);
m->transformtype = stbv_get_bits(f,16);
m->mapping = stbv_get_bits(f,8);
if (m->windowtype != 0) return stbv_error(f, VORBIS_invalid_setup);
if (m->transformtype != 0) return stbv_error(f, VORBIS_invalid_setup);
if (m->mapping >= f->mapping_count) return stbv_error(f, VORBIS_invalid_setup);
}

stbv_flush_packet(f);

f->previous_length = 0;

for (i=0; i < f->channels; ++i) {
f->channel_buffers[i] = (float *) stbv_setup_malloc(f, sizeof(float) * f->blocksize_1);
f->previous_window[i] = (float *) stbv_setup_malloc(f, sizeof(float) * f->blocksize_1/2);
f->finalY[i] = (stbv_int16 *) stbv_setup_malloc(f, sizeof(stbv_int16) * longest_floorlist);
if (f->channel_buffers[i] == NULL || f->previous_window[i] == NULL || f->finalY[i] == NULL) return stbv_error(f, VORBIS_outofmem);
memset(f->channel_buffers[i], 0, sizeof(float) * f->blocksize_1);
#if defined(STB_VORBIS_NO_DEFER_FLOOR)
f->floor_buffers[i] = (float *) stbv_setup_malloc(f, sizeof(float) * f->blocksize_1/2);
if (f->floor_buffers[i] == NULL) return stbv_error(f, VORBIS_outofmem);
#endif
}

if (!stbv_init_blocksize(f, 0, f->blocksize_0)) return FALSE;
if (!stbv_init_blocksize(f, 1, f->blocksize_1)) return FALSE;
f->blocksize[0] = f->blocksize_0;
f->blocksize[1] = f->blocksize_1;

#if defined(STB_VORBIS_DIVIDE_TABLE)
if (stbv_integer_divide_table[1][1]==0)
for (i=0; i < STBV_DIVTAB_NUMER; ++i)
for (j=1; j < STBV_DIVTAB_DENOM; ++j)
stbv_integer_divide_table[i][j] = i / j;
#endif




{
stbv_uint32 imdct_mem = (f->blocksize_1 * sizeof(float) >> 1);
stbv_uint32 classify_mem;
int i,max_part_read=0;
for (i=0; i < f->residue_count; ++i) {
StbvResidue *r = f->residue_config + i;
unsigned int actual_size = f->blocksize_1 / 2;
unsigned int limit_r_begin = r->begin < actual_size ? r->begin : actual_size;
unsigned int limit_r_end = r->end < actual_size ? r->end : actual_size;
int n_read = limit_r_end - limit_r_begin;
int part_read = n_read / r->part_size;
if (part_read > max_part_read)
max_part_read = part_read;
}
#if !defined(STB_VORBIS_DIVIDES_IN_RESIDUE)
classify_mem = f->channels * (sizeof(void*) + max_part_read * sizeof(stbv_uint8 *));
#else
classify_mem = f->channels * (sizeof(void*) + max_part_read * sizeof(int *));
#endif



f->temp_memory_required = classify_mem;
if (imdct_mem > f->temp_memory_required)
f->temp_memory_required = imdct_mem;
}

f->first_decode = TRUE;

if (f->alloc.alloc_buffer) {
assert(f->temp_offset == f->alloc.alloc_buffer_length_in_bytes);

if (f->setup_offset + sizeof(*f) + f->temp_memory_required > (unsigned) f->temp_offset)
return stbv_error(f, VORBIS_outofmem);
}

f->first_audio_page_offset = stb_vorbis_get_file_offset(f);

return TRUE;
}

static void stbv_vorbis_deinit(stb_vorbis *p)
{
int i,j;
if (p->residue_config) {
for (i=0; i < p->residue_count; ++i) {
StbvResidue *r = p->residue_config+i;
if (r->classdata) {
for (j=0; j < p->codebooks[r->classbook].entries; ++j)
stbv_setup_free(p, r->classdata[j]);
stbv_setup_free(p, r->classdata);
}
stbv_setup_free(p, r->residue_books);
}
}

if (p->codebooks) {
STBV_CHECK(p);
for (i=0; i < p->codebook_count; ++i) {
StbvCodebook *c = p->codebooks + i;
stbv_setup_free(p, c->codeword_lengths);
stbv_setup_free(p, c->multiplicands);
stbv_setup_free(p, c->codewords);
stbv_setup_free(p, c->sorted_codewords);

stbv_setup_free(p, c->sorted_values ? c->sorted_values-1 : NULL);
}
stbv_setup_free(p, p->codebooks);
}
stbv_setup_free(p, p->floor_config);
stbv_setup_free(p, p->residue_config);
if (p->mapping) {
for (i=0; i < p->mapping_count; ++i)
stbv_setup_free(p, p->mapping[i].chan);
stbv_setup_free(p, p->mapping);
}
STBV_CHECK(p);
for (i=0; i < p->channels && i < STB_VORBIS_MAX_CHANNELS; ++i) {
stbv_setup_free(p, p->channel_buffers[i]);
stbv_setup_free(p, p->previous_window[i]);
#if defined(STB_VORBIS_NO_DEFER_FLOOR)
stbv_setup_free(p, p->floor_buffers[i]);
#endif
stbv_setup_free(p, p->finalY[i]);
}
for (i=0; i < 2; ++i) {
stbv_setup_free(p, p->A[i]);
stbv_setup_free(p, p->B[i]);
stbv_setup_free(p, p->C[i]);
stbv_setup_free(p, p->window[i]);
stbv_setup_free(p, p->stbv_bit_reverse[i]);
}
#if !defined(STB_VORBIS_NO_STDIO)
if (p->close_on_free) fclose(p->f);
#endif
}

STBVDEF void stb_vorbis_close(stb_vorbis *p)
{
if (p == NULL) return;
stbv_vorbis_deinit(p);
stbv_setup_free(p,p);
}

static void stbv_vorbis_init(stb_vorbis *p, const stb_vorbis_alloc *z)
{
memset(p, 0, sizeof(*p)); 
if (z) {
p->alloc = *z;
p->alloc.alloc_buffer_length_in_bytes = (p->alloc.alloc_buffer_length_in_bytes+3) & ~3;
p->temp_offset = p->alloc.alloc_buffer_length_in_bytes;
}
p->eof = 0;
p->error = VORBIS__no_error;
p->stream = NULL;
p->codebooks = NULL;
p->page_crc_tests = -1;
#if !defined(STB_VORBIS_NO_STDIO)
p->close_on_free = FALSE;
p->f = NULL;
#endif
}

STBVDEF int stb_vorbis_get_sample_offset(stb_vorbis *f)
{
if (f->current_loc_valid)
return f->current_loc;
else
return -1;
}

STBVDEF stb_vorbis_info stb_vorbis_get_info(stb_vorbis *f)
{
stb_vorbis_info d;
d.channels = f->channels;
d.sample_rate = f->sample_rate;
d.setup_memory_required = f->setup_memory_required;
d.setup_temp_memory_required = f->setup_temp_memory_required;
d.temp_memory_required = f->temp_memory_required;
d.max_frame_size = f->blocksize_1 >> 1;
return d;
}

STBVDEF int stb_vorbis_get_error(stb_vorbis *f)
{
int e = f->error;
f->error = VORBIS__no_error;
return e;
}

static stb_vorbis * stbv_vorbis_alloc(stb_vorbis *f)
{
stb_vorbis *p = (stb_vorbis *) stbv_setup_malloc(f, sizeof(*p));
return p;
}

#if !defined(STB_VORBIS_NO_PUSHDATA_API)

STBVDEF void stb_vorbis_flush_pushdata(stb_vorbis *f)
{
f->previous_length = 0;
f->page_crc_tests = 0;
f->discard_samples_deferred = 0;
f->current_loc_valid = FALSE;
f->first_decode = FALSE;
f->samples_output = 0;
f->channel_buffer_start = 0;
f->channel_buffer_end = 0;
}

static int stbv_vorbis_search_for_page_pushdata(stbv_vorb *f, stbv_uint8 *data, int data_len)
{
int i,n;
for (i=0; i < f->page_crc_tests; ++i)
f->scan[i].bytes_done = 0;



if (f->page_crc_tests < STB_VORBIS_PUSHDATA_CRC_COUNT) {
if (data_len < 4) return 0;
data_len -= 3; 

for (i=0; i < data_len; ++i) {
if (data[i] == 0x4f) {
if (0==memcmp(data+i, stbv_ogg_page_header, 4)) {
int j,len;
stbv_uint32 crc;

if (i+26 >= data_len || i+27+data[i+26] >= data_len) {


data_len = i;
break;
}

len = 27 + data[i+26];
for (j=0; j < data[i+26]; ++j)
len += data[i+27+j];

crc = 0;
for (j=0; j < 22; ++j)
crc = stbv_crc32_update(crc, data[i+j]);

for ( ; j < 26; ++j)
crc = stbv_crc32_update(crc, 0);

n = f->page_crc_tests++;
f->scan[n].bytes_left = len-j;
f->scan[n].crc_so_far = crc;
f->scan[n].goal_crc = data[i+22] + (data[i+23] << 8) + (data[i+24]<<16) + (data[i+25]<<24);


if (data[i+27+data[i+26]-1] == 255)
f->scan[n].sample_loc = ~0;
else
f->scan[n].sample_loc = data[i+6] + (data[i+7] << 8) + (data[i+ 8]<<16) + (data[i+ 9]<<24);
f->scan[n].bytes_done = i+j;
if (f->page_crc_tests == STB_VORBIS_PUSHDATA_CRC_COUNT)
break;

}
}
}
}

for (i=0; i < f->page_crc_tests;) {
stbv_uint32 crc;
int j;
int n = f->scan[i].bytes_done;
int m = f->scan[i].bytes_left;
if (m > data_len - n) m = data_len - n;

crc = f->scan[i].crc_so_far;
for (j=0; j < m; ++j)
crc = stbv_crc32_update(crc, data[n+j]);
f->scan[i].bytes_left -= m;
f->scan[i].crc_so_far = crc;
if (f->scan[i].bytes_left == 0) {

if (f->scan[i].crc_so_far == f->scan[i].goal_crc) {

data_len = n+m; 
f->page_crc_tests = -1; 
f->previous_length = 0; 
f->next_seg = -1; 
f->current_loc = f->scan[i].sample_loc; 

f->current_loc_valid = f->current_loc != ~0U;
return data_len;
}

f->scan[i] = f->scan[--f->page_crc_tests];
} else {
++i;
}
}

return data_len;
}


STBVDEF int stb_vorbis_decode_frame_pushdata(
stb_vorbis *f, 
const stbv_uint8 *data, int data_len, 
int *channels, 
float ***output, 
int *samples 
)
{
int i;
int len,right,left;

if (!STBV_IS_PUSH_MODE(f)) return stbv_error(f, VORBIS_invalid_api_mixing);

if (f->page_crc_tests >= 0) {
*samples = 0;
return stbv_vorbis_search_for_page_pushdata(f, (stbv_uint8 *) data, data_len);
}

f->stream = (stbv_uint8 *) data;
f->stream_end = (stbv_uint8 *) data + data_len;
f->error = VORBIS__no_error;


if (!stbv_is_whole_packet_present(f, FALSE)) {
*samples = 0;
return 0;
}

if (!stbv_vorbis_decode_packet(f, &len, &left, &right)) {

enum STBVorbisError error = f->error;
if (error == VORBIS_bad_packet_type) {

f->error = VORBIS__no_error;
while (stbv_get8_packet(f) != STBV_EOP)
if (f->eof) break;
*samples = 0;
return (int) (f->stream - data);
}
if (error == VORBIS_continued_packet_flag_invalid) {
if (f->previous_length == 0) {


f->error = VORBIS__no_error;
while (stbv_get8_packet(f) != STBV_EOP)
if (f->eof) break;
*samples = 0;
return (int) (f->stream - data);
}
}


stb_vorbis_flush_pushdata(f);

f->error = error;
*samples = 0;
return 1;
}


len = stbv_vorbis_finish_frame(f, len, left, right);
for (i=0; i < f->channels; ++i)
f->outputs[i] = f->channel_buffers[i] + left;

if (channels) *channels = f->channels;
*samples = len;
*output = f->outputs;
return (int) (f->stream - data);
}

STBVDEF stb_vorbis *stb_vorbis_open_pushdata(
const unsigned char *data, int data_len, 
int *data_used, 
int *error, const stb_vorbis_alloc *alloc)
{
stb_vorbis *f, p;
stbv_vorbis_init(&p, alloc);
p.stream = (stbv_uint8 *) data;
p.stream_end = (stbv_uint8 *) data + data_len;
p.push_mode = TRUE;
if (!stbv_start_decoder(&p)) {
if (p.eof)
*error = VORBIS_need_more_data;
else
*error = p.error;
return NULL;
}
f = stbv_vorbis_alloc(&p);
if (f) {
*f = p;
*data_used = (int) (f->stream - data);
*error = 0;
return f;
} else {
stbv_vorbis_deinit(&p);
return NULL;
}
}
#endif 

STBVDEF unsigned int stb_vorbis_get_file_offset(stb_vorbis *f)
{
#if !defined(STB_VORBIS_NO_PUSHDATA_API)
if (f->push_mode) return 0;
#endif
if (STBV_USE_MEMORY(f)) return (unsigned int) (f->stream - f->stream_start);
#if !defined(STB_VORBIS_NO_STDIO)
return (unsigned int) (ftell(f->f) - f->f_start);
#endif
}

#if !defined(STB_VORBIS_NO_PULLDATA_API)




static stbv_uint32 stbv_vorbis_find_page(stb_vorbis *f, stbv_uint32 *end, stbv_uint32 *last)
{
for(;;) {
int n;
if (f->eof) return 0;
n = stbv_get8(f);
if (n == 0x4f) { 
unsigned int retry_loc = stb_vorbis_get_file_offset(f);
int i;

if (retry_loc - 25 > f->stream_len)
return 0;

for (i=1; i < 4; ++i)
if (stbv_get8(f) != stbv_ogg_page_header[i])
break;
if (f->eof) return 0;
if (i == 4) {
stbv_uint8 header[27];
stbv_uint32 i, crc, goal, len;
for (i=0; i < 4; ++i)
header[i] = stbv_ogg_page_header[i];
for (; i < 27; ++i)
header[i] = stbv_get8(f);
if (f->eof) return 0;
if (header[4] != 0) goto invalid;
goal = header[22] + (header[23] << 8) + (header[24]<<16) + (header[25]<<24);
for (i=22; i < 26; ++i)
header[i] = 0;
crc = 0;
for (i=0; i < 27; ++i)
crc = stbv_crc32_update(crc, header[i]);
len = 0;
for (i=0; i < header[26]; ++i) {
int s = stbv_get8(f);
crc = stbv_crc32_update(crc, s);
len += s;
}
if (len && f->eof) return 0;
for (i=0; i < len; ++i)
crc = stbv_crc32_update(crc, stbv_get8(f));

if (crc == goal) {








if (end)
*end = stb_vorbis_get_file_offset(f);
if (last) {
if (header[5] & 0x04)
*last = 1;
else
*last = 0;
}
stbv_set_file_offset(f, retry_loc-1);
return 1;
}
}
invalid:

stbv_set_file_offset(f, retry_loc);
}
}
}


#define STBV_SAMPLE_unknown 0xffffffff










static int stbv_get_seek_page_info(stb_vorbis *f, StbvProbedPage *z)
{
stbv_uint8 header[27], lacing[255];
int i,len;


z->page_start = stb_vorbis_get_file_offset(f);


stbv_getn(f, header, 27);
if (header[0] != 'O' || header[1] != 'g' || header[2] != 'g' || header[3] != 'S')
return 0;
stbv_getn(f, lacing, header[26]);


len = 0;
for (i=0; i < header[26]; ++i)
len += lacing[i];


z->page_end = z->page_start + 27 + header[26] + len;


z->last_decoded_sample = header[6] + (header[7] << 8) + (header[8] << 16) + (header[9] << 24);


stbv_set_file_offset(f, z->page_start);
return 1;
}



static int stbv_go_to_page_before(stb_vorbis *f, unsigned int limit_offset)
{
unsigned int previous_safe, end;


if (limit_offset >= 65536 && limit_offset-65536 >= f->first_audio_page_offset)
previous_safe = limit_offset - 65536;
else
previous_safe = f->first_audio_page_offset;

stbv_set_file_offset(f, previous_safe);

while (stbv_vorbis_find_page(f, &end, NULL)) {
if (end >= limit_offset && stb_vorbis_get_file_offset(f) < limit_offset)
return 1;
stbv_set_file_offset(f, end);
}

return 0;
}





static int stbv_seek_to_sample_coarse(stb_vorbis *f, stbv_uint32 sample_number)
{
StbvProbedPage left, right, mid;
int i, start_seg_with_known_loc, end_pos, page_start;
stbv_uint32 delta, stream_length, padding;
double offset, bytes_per_sample;
int probe = 0;


stream_length = stb_vorbis_stream_length_in_samples(f);
if (stream_length == 0) return stbv_error(f, VORBIS_seek_without_length);
if (sample_number > stream_length) return stbv_error(f, VORBIS_seek_invalid);




padding = ((f->blocksize_1 - f->blocksize_0) >> 2);
if (sample_number < padding)
sample_number = 0;
else
sample_number -= padding;

left = f->p_first;
while (left.last_decoded_sample == ~0U) {

stbv_set_file_offset(f, left.page_end);
if (!stbv_get_seek_page_info(f, &left)) goto error;
}

right = f->p_last;
assert(right.last_decoded_sample != ~0U);


if (sample_number <= left.last_decoded_sample) {
if (stb_vorbis_seek_start(f))
return 1;
return 0;
}

while (left.page_end != right.page_start) {
assert(left.page_end < right.page_start);

delta = right.page_start - left.page_end;
if (delta <= 65536) {

stbv_set_file_offset(f, left.page_end);
} else {
if (probe < 2) {
if (probe == 0) {

double data_bytes = right.page_end - left.page_start;
bytes_per_sample = data_bytes / right.last_decoded_sample;
offset = left.page_start + bytes_per_sample * (sample_number - left.last_decoded_sample);
} else {

double error = ((double) sample_number - mid.last_decoded_sample) * bytes_per_sample;
if (error >= 0 && error < 8000) error = 8000;
if (error < 0 && error > -8000) error = -8000;
offset += error * 2;
}


if (offset < left.page_end)
offset = left.page_end;
if (offset > right.page_start - 65536)
offset = right.page_start - 65536;

stbv_set_file_offset(f, (unsigned int) offset);
} else {


stbv_set_file_offset(f, left.page_end + (delta / 2) - 32768);
}

if (!stbv_vorbis_find_page(f, NULL, NULL)) goto error;
}

for (;;) {
if (!stbv_get_seek_page_info(f, &mid)) goto error;
if (mid.last_decoded_sample != ~0U) break;

stbv_set_file_offset(f, mid.page_end);
assert(mid.page_start < right.page_start);
}



if (mid.page_start == right.page_start)
break;

if (sample_number < mid.last_decoded_sample)
right = mid;
else
left = mid;

++probe;
}


page_start = left.page_start;
stbv_set_file_offset(f, page_start);
if (!stbv_start_page(f)) return stbv_error(f, VORBIS_seek_failed);
end_pos = f->end_seg_with_known_loc;
assert(end_pos >= 0);

for (;;) {
for (i = end_pos; i > 0; --i)
if (f->segments[i-1] != 255)
break;

start_seg_with_known_loc = i;

if (start_seg_with_known_loc > 0 || !(f->page_flag & STBV_PAGEFLAG_continued_packet))
break;


if (!stbv_go_to_page_before(f, page_start))
goto error;

page_start = stb_vorbis_get_file_offset(f);
if (!stbv_start_page(f)) goto error;
end_pos = f->segment_count - 1;
}


f->current_loc_valid = FALSE;
f->last_seg = FALSE;
f->valid_bits = 0;
f->packet_bytes = 0;
f->bytes_in_seg = 0;
f->previous_length = 0;
f->next_seg = start_seg_with_known_loc;

for (i = 0; i < start_seg_with_known_loc; i++)
stbv_skip(f, f->segments[i]);


if (!stbv_vorbis_pump_first_frame(f))
return 0;
if (f->current_loc > sample_number)
return stbv_error(f, VORBIS_seek_failed);
return 1;

error:

stb_vorbis_seek_start(f);
return stbv_error(f, VORBIS_seek_failed);
}


static int stbv_peek_decode_initial(stbv_vorb *f, int *p_left_start, int *p_left_end, int *p_right_start, int *p_right_end, int *mode)
{
int bits_read, bytes_read;

if (!stbv_vorbis_decode_initial(f, p_left_start, p_left_end, p_right_start, p_right_end, mode))
return 0;


bits_read = 1 + stbv_ilog(f->mode_count-1);
if (f->mode_config[*mode].blockflag)
bits_read += 2;
bytes_read = (bits_read + 7) / 8;

f->bytes_in_seg += bytes_read;
f->packet_bytes -= bytes_read;
stbv_skip(f, -bytes_read);
if (f->next_seg == -1)
f->next_seg = f->segment_count - 1;
else
f->next_seg--;
f->valid_bits = 0;

return 1;
}

STBVDEF int stb_vorbis_seek_frame(stb_vorbis *f, unsigned int sample_number)
{
stbv_uint32 max_frame_samples;

if (STBV_IS_PUSH_MODE(f)) return stbv_error(f, VORBIS_invalid_api_mixing);


if (!stbv_seek_to_sample_coarse(f, sample_number))
return 0;

assert(f->current_loc_valid);
assert(f->current_loc <= sample_number);


max_frame_samples = (f->blocksize_1*3 - f->blocksize_0) >> 2;
while (f->current_loc < sample_number) {
int left_start, left_end, right_start, right_end, mode, frame_samples;
if (!stbv_peek_decode_initial(f, &left_start, &left_end, &right_start, &right_end, &mode))
return stbv_error(f, VORBIS_seek_failed);

frame_samples = right_start - left_start;
if (f->current_loc + frame_samples > sample_number) {
return 1; 
} else if (f->current_loc + frame_samples + max_frame_samples > sample_number) {

stbv_vorbis_pump_first_frame(f);
} else {

f->current_loc += frame_samples;
f->previous_length = 0;
stbv_maybe_start_packet(f);
stbv_flush_packet(f);
}
}

assert(f->current_loc == sample_number);
return 1;
}

STBVDEF int stb_vorbis_seek(stb_vorbis *f, unsigned int sample_number)
{
if (!stb_vorbis_seek_frame(f, sample_number))
return 0;

if (sample_number != f->current_loc) {
int n;
stbv_uint32 frame_start = f->current_loc;
stb_vorbis_get_frame_float(f, &n, NULL);
assert(sample_number > frame_start);
assert(f->channel_buffer_start + (int) (sample_number-frame_start) <= f->channel_buffer_end);
f->channel_buffer_start += (sample_number - frame_start);
}

return 1;
}

STBVDEF int stb_vorbis_seek_start(stb_vorbis *f)
{
if (STBV_IS_PUSH_MODE(f)) { return stbv_error(f, VORBIS_invalid_api_mixing); }
stbv_set_file_offset(f, f->first_audio_page_offset);
f->previous_length = 0;
f->first_decode = TRUE;
f->next_seg = -1;
return stbv_vorbis_pump_first_frame(f);
}

STBVDEF unsigned int stb_vorbis_stream_length_in_samples(stb_vorbis *f)
{
unsigned int restore_offset, previous_safe;
unsigned int end, last_page_loc;

if (STBV_IS_PUSH_MODE(f)) return stbv_error(f, VORBIS_invalid_api_mixing);
if (!f->total_samples) {
unsigned int last;
stbv_uint32 lo,hi;
char header[6];


restore_offset = stb_vorbis_get_file_offset(f);



if (f->stream_len >= 65536 && f->stream_len-65536 >= f->first_audio_page_offset)
previous_safe = f->stream_len - 65536;
else
previous_safe = f->first_audio_page_offset;

stbv_set_file_offset(f, previous_safe);



if (!stbv_vorbis_find_page(f, &end, &last)) {

f->error = VORBIS_cant_find_last_page;
f->total_samples = 0xffffffff;
goto done;
}


last_page_loc = stb_vorbis_get_file_offset(f);




while (!last) {
stbv_set_file_offset(f, end);
if (!stbv_vorbis_find_page(f, &end, &last)) {


break;
}
previous_safe = last_page_loc+1;
last_page_loc = stb_vorbis_get_file_offset(f);
}

stbv_set_file_offset(f, last_page_loc);


stbv_getn(f, (unsigned char *)header, 6);

lo = stbv_get32(f);
hi = stbv_get32(f);
if (lo == 0xffffffff && hi == 0xffffffff) {
f->error = VORBIS_cant_find_last_page;
f->total_samples = STBV_SAMPLE_unknown;
goto done;
}
if (hi)
lo = 0xfffffffe; 
f->total_samples = lo;

f->p_last.page_start = last_page_loc;
f->p_last.page_end = end;
f->p_last.last_decoded_sample = lo;

done:
stbv_set_file_offset(f, restore_offset);
}
return f->total_samples == STBV_SAMPLE_unknown ? 0 : f->total_samples;
}

STBVDEF float stb_vorbis_stream_length_in_seconds(stb_vorbis *f)
{
return stb_vorbis_stream_length_in_samples(f) / (float) f->sample_rate;
}



STBVDEF int stb_vorbis_get_frame_float(stb_vorbis *f, int *channels, float ***output)
{
int len, right,left,i;
if (STBV_IS_PUSH_MODE(f)) return stbv_error(f, VORBIS_invalid_api_mixing);

if (!stbv_vorbis_decode_packet(f, &len, &left, &right)) {
f->channel_buffer_start = f->channel_buffer_end = 0;
return 0;
}

len = stbv_vorbis_finish_frame(f, len, left, right);
for (i=0; i < f->channels; ++i)
f->outputs[i] = f->channel_buffers[i] + left;

f->channel_buffer_start = left;
f->channel_buffer_end = left+len;

if (channels) *channels = f->channels;
if (output) *output = f->outputs;
return len;
}

#if !defined(STB_VORBIS_NO_STDIO)

STBVDEF stb_vorbis * stb_vorbis_open_file_section(FILE *file, int close_on_free, int *error, const stb_vorbis_alloc *alloc, unsigned int length)
{
stb_vorbis *f, p;
stbv_vorbis_init(&p, alloc);
p.f = file;
p.f_start = (stbv_uint32) ftell(file);
p.stream_len = length;
p.close_on_free = close_on_free;
if (stbv_start_decoder(&p)) {
f = stbv_vorbis_alloc(&p);
if (f) {
*f = p;
stbv_vorbis_pump_first_frame(f);
return f;
}
}
if (error) *error = p.error;
stbv_vorbis_deinit(&p);
return NULL;
}

STBVDEF stb_vorbis * stb_vorbis_open_file(FILE *file, int close_on_free, int *error, const stb_vorbis_alloc *alloc)
{
unsigned int len, start;
start = (unsigned int) ftell(file);
fseek(file, 0, SEEK_END);
len = (unsigned int) (ftell(file) - start);
fseek(file, start, SEEK_SET);
return stb_vorbis_open_file_section(file, close_on_free, error, alloc, len);
}

STBVDEF stb_vorbis * stb_vorbis_open_filename(const char *filename, int *error, const stb_vorbis_alloc *alloc)
{
FILE *f = fopen(filename, "rb");
if (f) 
return stb_vorbis_open_file(f, TRUE, error, alloc);
if (error) *error = VORBIS_file_open_failure;
return NULL;
}
#endif 

STBVDEF stb_vorbis * stb_vorbis_open_memory(const unsigned char *data, int len, int *error, const stb_vorbis_alloc *alloc)
{
stb_vorbis *f, p;
if (data == NULL) return NULL;
stbv_vorbis_init(&p, alloc);
p.stream = (stbv_uint8 *) data;
p.stream_end = (stbv_uint8 *) data + len;
p.stream_start = (stbv_uint8 *) p.stream;
p.stream_len = len;
p.push_mode = FALSE;
if (stbv_start_decoder(&p)) {
f = stbv_vorbis_alloc(&p);
if (f) {
*f = p;
stbv_vorbis_pump_first_frame(f);
if (error) *error = VORBIS__no_error;
return f;
}
}
if (error) *error = p.error;
stbv_vorbis_deinit(&p);
return NULL;
}

#if !defined(STB_VORBIS_NO_INTEGER_CONVERSION)
#define STBV_PLAYBACK_MONO 1
#define STBV_PLAYBACK_LEFT 2
#define STBV_PLAYBACK_RIGHT 4

#define STBV_L (STBV_PLAYBACK_LEFT | STBV_PLAYBACK_MONO)
#define STBV_C (STBV_PLAYBACK_LEFT | STBV_PLAYBACK_RIGHT | STBV_PLAYBACK_MONO)
#define STBV_R (STBV_PLAYBACK_RIGHT | STBV_PLAYBACK_MONO)

static stbv_int8 stbv_channel_position[7][6] =
{
{ 0 },
{ STBV_C },
{ STBV_L, STBV_R },
{ STBV_L, STBV_C, STBV_R },
{ STBV_L, STBV_R, STBV_L, STBV_R },
{ STBV_L, STBV_C, STBV_R, STBV_L, STBV_R },
{ STBV_L, STBV_C, STBV_R, STBV_L, STBV_R, STBV_C },
};


#if !defined(STB_VORBIS_NO_FAST_SCALED_FLOAT)
typedef union {
float f;
int i;
} stbv_float_conv;
typedef char stb_vorbis_float_size_test[sizeof(float)==4 && sizeof(int) == 4];
#define STBV_FASTDEF(x) stbv_float_conv x

#define STBV_MAGIC(SHIFT) (1.5f * (1 << (23-SHIFT)) + 0.5f/(1 << SHIFT))
#define STBV_ADDEND(SHIFT) (((150-SHIFT) << 23) + (1 << 22))
#define STBV_FAST_SCALED_FLOAT_TO_INT(temp,x,s) (temp.f = (x) + STBV_MAGIC(s), temp.i - STBV_ADDEND(s))
#define stbv_check_endianness() 
#else
#define STBV_FAST_SCALED_FLOAT_TO_INT(temp,x,s) ((int) ((x) * (1 << (s))))
#define stbv_check_endianness()
#define STBV_FASTDEF(x)
#endif

static void stbv_copy_samples(short *dest, float *src, int len)
{
int i;
stbv_check_endianness();
for (i=0; i < len; ++i) {
STBV_FASTDEF(temp);
int v = STBV_FAST_SCALED_FLOAT_TO_INT(temp, src[i],15);
if ((unsigned int) (v + 32768) > 65535)
v = v < 0 ? -32768 : 32767;
dest[i] = v;
}
}

static void stbv_compute_samples(int mask, short *output, int num_c, float **data, int d_offset, int len)
{
#define BUFFER_SIZE 32
float buffer[BUFFER_SIZE];
int i,j,o,n = BUFFER_SIZE;
stbv_check_endianness();
for (o = 0; o < len; o += BUFFER_SIZE) {
memset(buffer, 0, sizeof(buffer));
if (o + n > len) n = len - o;
for (j=0; j < num_c; ++j) {
if (stbv_channel_position[num_c][j] & mask) {
for (i=0; i < n; ++i)
buffer[i] += data[j][d_offset+o+i];
}
}
for (i=0; i < n; ++i) {
STBV_FASTDEF(temp);
int v = STBV_FAST_SCALED_FLOAT_TO_INT(temp,buffer[i],15);
if ((unsigned int) (v + 32768) > 65535)
v = v < 0 ? -32768 : 32767;
output[o+i] = v;
}
}
}

static void stbv_compute_stereo_samples(short *output, int num_c, float **data, int d_offset, int len)
{
#define BUFFER_SIZE 32
float buffer[BUFFER_SIZE];
int i,j,o,n = BUFFER_SIZE >> 1;

stbv_check_endianness();
for (o = 0; o < len; o += BUFFER_SIZE >> 1) {

int o2 = o << 1;
memset(buffer, 0, sizeof(buffer));
if (o + n > len) n = len - o;
for (j=0; j < num_c; ++j) {
int m = stbv_channel_position[num_c][j] & (STBV_PLAYBACK_LEFT | STBV_PLAYBACK_RIGHT);
if (m == (STBV_PLAYBACK_LEFT | STBV_PLAYBACK_RIGHT)) {
for (i=0; i < n; ++i) {
buffer[i*2+0] += data[j][d_offset+o+i];
buffer[i*2+1] += data[j][d_offset+o+i];
}
} else if (m == STBV_PLAYBACK_LEFT) {
for (i=0; i < n; ++i) {
buffer[i*2+0] += data[j][d_offset+o+i];
}
} else if (m == STBV_PLAYBACK_RIGHT) {
for (i=0; i < n; ++i) {
buffer[i*2+1] += data[j][d_offset+o+i];
}
}
}
for (i=0; i < (n<<1); ++i) {
STBV_FASTDEF(temp);
int v = STBV_FAST_SCALED_FLOAT_TO_INT(temp,buffer[i],15);
if ((unsigned int) (v + 32768) > 65535)
v = v < 0 ? -32768 : 32767;
output[o2+i] = v;
}
}
}

static void stbv_convert_samples_short(int buf_c, short **buffer, int b_offset, int data_c, float **data, int d_offset, int samples)
{
int i;
if (buf_c != data_c && buf_c <= 2 && data_c <= 6) {
static int channel_selector[3][2] = { {0}, {STBV_PLAYBACK_MONO}, {STBV_PLAYBACK_LEFT, STBV_PLAYBACK_RIGHT} };
for (i=0; i < buf_c; ++i)
stbv_compute_samples(channel_selector[buf_c][i], buffer[i]+b_offset, data_c, data, d_offset, samples);
} else {
int limit = buf_c < data_c ? buf_c : data_c;
for (i=0; i < limit; ++i)
stbv_copy_samples(buffer[i]+b_offset, data[i]+d_offset, samples);
for ( ; i < buf_c; ++i)
memset(buffer[i]+b_offset, 0, sizeof(short) * samples);
}
}

STBVDEF int stb_vorbis_get_frame_short(stb_vorbis *f, int num_c, short **buffer, int num_samples)
{
float **output;
int len = stb_vorbis_get_frame_float(f, NULL, &output);
if (len > num_samples) len = num_samples;
if (len)
stbv_convert_samples_short(num_c, buffer, 0, f->channels, output, 0, len);
return len;
}

static void stbv_convert_channels_short_interleaved(int buf_c, short *buffer, int data_c, float **data, int d_offset, int len)
{
int i;
stbv_check_endianness();
if (buf_c != data_c && buf_c <= 2 && data_c <= 6) {
assert(buf_c == 2);
for (i=0; i < buf_c; ++i)
stbv_compute_stereo_samples(buffer, data_c, data, d_offset, len);
} else {
int limit = buf_c < data_c ? buf_c : data_c;
int j;
for (j=0; j < len; ++j) {
for (i=0; i < limit; ++i) {
STBV_FASTDEF(temp);
float f = data[i][d_offset+j];
int v = STBV_FAST_SCALED_FLOAT_TO_INT(temp, f,15);
if ((unsigned int) (v + 32768) > 65535)
v = v < 0 ? -32768 : 32767;
*buffer++ = v;
}
for ( ; i < buf_c; ++i)
*buffer++ = 0;
}
}
}

STBVDEF int stb_vorbis_get_frame_short_interleaved(stb_vorbis *f, int num_c, short *buffer, int num_shorts)
{
float **output;
int len;
if (num_c == 1) return stb_vorbis_get_frame_short(f,num_c,&buffer, num_shorts);
len = stb_vorbis_get_frame_float(f, NULL, &output);
if (len) {
if (len*num_c > num_shorts) len = num_shorts / num_c;
stbv_convert_channels_short_interleaved(num_c, buffer, f->channels, output, 0, len);
}
return len;
}

STBVDEF int stb_vorbis_get_samples_short_interleaved(stb_vorbis *f, int channels, short *buffer, int num_shorts)
{
float **outputs;
int len = num_shorts / channels;
int n=0;
int z = f->channels;
if (z > channels) z = channels;
while (n < len) {
int k = f->channel_buffer_end - f->channel_buffer_start;
if (n+k >= len) k = len - n;
if (k)
stbv_convert_channels_short_interleaved(channels, buffer, f->channels, f->channel_buffers, f->channel_buffer_start, k);
buffer += k*channels;
n += k;
f->channel_buffer_start += k;
if (n == len) break;
if (!stb_vorbis_get_frame_float(f, NULL, &outputs)) break;
}
return n;
}

STBVDEF int stb_vorbis_get_samples_short(stb_vorbis *f, int channels, short **buffer, int len)
{
float **outputs;
int n=0;
int z = f->channels;
if (z > channels) z = channels;
while (n < len) {
int k = f->channel_buffer_end - f->channel_buffer_start;
if (n+k >= len) k = len - n;
if (k)
stbv_convert_samples_short(channels, buffer, n, f->channels, f->channel_buffers, f->channel_buffer_start, k);
n += k;
f->channel_buffer_start += k;
if (n == len) break;
if (!stb_vorbis_get_frame_float(f, NULL, &outputs)) break;
}
return n;
}

#if !defined(STB_VORBIS_NO_STDIO)
STBVDEF int stb_vorbis_decode_filename(const char *filename, int *channels, int *sample_rate, short **output)
{
int data_len, offset, total, limit, error;
short *data;
stb_vorbis *v = stb_vorbis_open_filename(filename, &error, NULL);
if (v == NULL) return -1;
limit = v->channels * 4096;
*channels = v->channels;
if (sample_rate)
*sample_rate = v->sample_rate;
offset = data_len = 0;
total = limit;
data = (short *) malloc(total * sizeof(*data));
if (data == NULL) {
stb_vorbis_close(v);
return -2;
}
for (;;) {
int n = stb_vorbis_get_frame_short_interleaved(v, v->channels, data+offset, total-offset);
if (n == 0) break;
data_len += n;
offset += n * v->channels;
if (offset + limit > total) {
short *data2;
total *= 2;
data2 = (short *) realloc(data, total * sizeof(*data));
if (data2 == NULL) {
free(data);
stb_vorbis_close(v);
return -2;
}
data = data2;
}
}
*output = data;
stb_vorbis_close(v);
return data_len;
}
#endif 

STBVDEF int stb_vorbis_decode_memory(const stbv_uint8 *mem, int len, int *channels, int *sample_rate, short **output)
{
int data_len, offset, total, limit, error;
short *data;
stb_vorbis *v = stb_vorbis_open_memory(mem, len, &error, NULL);
if (v == NULL) return -1;
limit = v->channels * 4096;
*channels = v->channels;
if (sample_rate)
*sample_rate = v->sample_rate;
offset = data_len = 0;
total = limit;
data = (short *) malloc(total * sizeof(*data));
if (data == NULL) {
stb_vorbis_close(v);
return -2;
}
for (;;) {
int n = stb_vorbis_get_frame_short_interleaved(v, v->channels, data+offset, total-offset);
if (n == 0) break;
data_len += n;
offset += n * v->channels;
if (offset + limit > total) {
short *data2;
total *= 2;
data2 = (short *) realloc(data, total * sizeof(*data));
if (data2 == NULL) {
free(data);
stb_vorbis_close(v);
return -2;
}
data = data2;
}
}
*output = data;
stb_vorbis_close(v);
return data_len;
}
#endif 

STBVDEF int stb_vorbis_get_samples_float_interleaved(stb_vorbis *f, int channels, float *buffer, int num_floats)
{
float **outputs;
int len = num_floats / channels;
int n=0;
int z = f->channels;
if (z > channels) z = channels;
while (n < len) {
int i,j;
int k = f->channel_buffer_end - f->channel_buffer_start;
if (n+k >= len) k = len - n;
for (j=0; j < k; ++j) {
for (i=0; i < z; ++i)
*buffer++ = f->channel_buffers[i][f->channel_buffer_start+j];
for ( ; i < channels; ++i)
*buffer++ = 0;
}
n += k;
f->channel_buffer_start += k;
if (n == len)
break;
if (!stb_vorbis_get_frame_float(f, NULL, &outputs))
break;
}
return n;
}

STBVDEF int stb_vorbis_get_samples_float(stb_vorbis *f, int channels, float **buffer, int num_samples)
{
float **outputs;
int n=0;
int z = f->channels;
if (z > channels) z = channels;
while (n < num_samples) {
int i;
int k = f->channel_buffer_end - f->channel_buffer_start;
if (n+k >= num_samples) k = num_samples - n;
if (k) {
for (i=0; i < z; ++i)
memcpy(buffer[i]+n, f->channel_buffers[i]+f->channel_buffer_start, sizeof(float)*k);
for ( ; i < channels; ++i)
memset(buffer[i]+n, 0, sizeof(float) * k);
}
n += k;
f->channel_buffer_start += k;
if (n == num_samples)
break;
if (!stb_vorbis_get_frame_float(f, NULL, &outputs))
break;
}
return n;
}
#endif 
























































#endif 











































