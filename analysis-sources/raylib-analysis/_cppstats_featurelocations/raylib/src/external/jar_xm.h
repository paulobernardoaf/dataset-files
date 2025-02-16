

















































#if !defined(INCLUDE_JAR_XM_H)
#define INCLUDE_JAR_XM_H

#include <stdint.h>

#define JAR_XM_DEBUG 0
#define JAR_XM_LINEAR_INTERPOLATION 1 
#define JAR_XM_DEFENSIVE 1
#define JAR_XM_RAMPING 1


#if !defined(JARXM_MALLOC)
#define JARXM_MALLOC(sz) malloc(sz)
#endif
#if !defined(JARXM_FREE)
#define JARXM_FREE(p) free(p)
#endif


struct jar_xm_context_s;
typedef struct jar_xm_context_s jar_xm_context_t;

#if defined(__cplusplus)
extern "C" { 
#endif

















int jar_xm_create_context_from_file(jar_xm_context_t** ctx, uint32_t rate, const char* filename);













int jar_xm_create_context(jar_xm_context_t** ctx, const char* moddata, uint32_t rate);











int jar_xm_create_context_safe(jar_xm_context_t** ctx, const char* moddata, size_t moddata_length, uint32_t rate);


void jar_xm_free_context(jar_xm_context_t* ctx);






void jar_xm_generate_samples(jar_xm_context_t* ctx, float* output, size_t numsamples);






void jar_xm_generate_samples_16bit(jar_xm_context_t* ctx, short* output, size_t numsamples)
{
float* musicBuffer = JARXM_MALLOC((2*numsamples)*sizeof(float));
jar_xm_generate_samples(ctx, musicBuffer, numsamples);

if(output){
int x;
for(x=0;x<2*numsamples;x++)
output[x] = musicBuffer[x] * SHRT_MAX;
}

JARXM_FREE(musicBuffer);
}






void jar_xm_generate_samples_8bit(jar_xm_context_t* ctx, char* output, size_t numsamples)
{
float* musicBuffer = JARXM_MALLOC((2*numsamples)*sizeof(float));
jar_xm_generate_samples(ctx, musicBuffer, numsamples);

if(output){
int x;
for(x=0;x<2*numsamples;x++)
output[x] = musicBuffer[x] * CHAR_MAX;
}

JARXM_FREE(musicBuffer);
}










void jar_xm_set_max_loop_count(jar_xm_context_t* ctx, uint8_t loopcnt);




uint8_t jar_xm_get_loop_count(jar_xm_context_t* ctx);









bool jar_xm_mute_channel(jar_xm_context_t* ctx, uint16_t, bool);








bool jar_xm_mute_instrument(jar_xm_context_t* ctx, uint16_t, bool);




const char* jar_xm_get_module_name(jar_xm_context_t* ctx);


const char* jar_xm_get_tracker_name(jar_xm_context_t* ctx);




uint16_t jar_xm_get_number_of_channels(jar_xm_context_t* ctx);


uint16_t jar_xm_get_module_length(jar_xm_context_t*);


uint16_t jar_xm_get_number_of_patterns(jar_xm_context_t* ctx);






uint16_t jar_xm_get_number_of_rows(jar_xm_context_t* ctx, uint16_t);


uint16_t jar_xm_get_number_of_instruments(jar_xm_context_t* ctx);






uint16_t jar_xm_get_number_of_samples(jar_xm_context_t* ctx, uint16_t);








void jar_xm_get_playing_speed(jar_xm_context_t* ctx, uint16_t* bpm, uint16_t* tempo);














void jar_xm_get_position(jar_xm_context_t* ctx, uint8_t* pattern_index, uint8_t* pattern, uint8_t* row, uint64_t* samples);







uint64_t jar_xm_get_latest_trigger_of_instrument(jar_xm_context_t* ctx, uint16_t);










uint64_t jar_xm_get_latest_trigger_of_sample(jar_xm_context_t* ctx, uint16_t instr, uint16_t sample);






uint64_t jar_xm_get_latest_trigger_of_channel(jar_xm_context_t* ctx, uint16_t);






uint64_t jar_xm_get_remaining_samples(jar_xm_context_t* ctx);

#if defined(__cplusplus)
}
#endif








#if defined(JAR_XM_IMPLEMENTATION)

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#if JAR_XM_DEBUG 
#include <stdio.h>
#define DEBUG(fmt, ...) do { fprintf(stderr, "%s(): " fmt "\n", __func__, __VA_ARGS__); fflush(stderr); } while(0)



#else
#define DEBUG(...)
#endif

#if jar_xm_BIG_ENDIAN
#error "Big endian platforms are not yet supported, sorry"

extern int __fail[-1];
#endif



#define SAMPLE_NAME_LENGTH 22
#define INSTRUMENT_NAME_LENGTH 22
#define MODULE_NAME_LENGTH 20
#define TRACKER_NAME_LENGTH 20
#define PATTERN_ORDER_TABLE_LENGTH 256
#define NUM_NOTES 96
#define NUM_ENVELOPE_POINTS 12
#define MAX_NUM_ROWS 256

#if JAR_XM_RAMPING
#define jar_xm_SAMPLE_RAMPING_POINTS 0x20
#endif



enum jar_xm_waveform_type_e {
jar_xm_SINE_WAVEFORM = 0,
jar_xm_RAMP_DOWN_WAVEFORM = 1,
jar_xm_SQUARE_WAVEFORM = 2,
jar_xm_RANDOM_WAVEFORM = 3,
jar_xm_RAMP_UP_WAVEFORM = 4,
};
typedef enum jar_xm_waveform_type_e jar_xm_waveform_type_t;

enum jar_xm_loop_type_e {
jar_xm_NO_LOOP,
jar_xm_FORWARD_LOOP,
jar_xm_PING_PONG_LOOP,
};
typedef enum jar_xm_loop_type_e jar_xm_loop_type_t;

enum jar_xm_frequency_type_e {
jar_xm_LINEAR_FREQUENCIES,
jar_xm_AMIGA_FREQUENCIES,
};
typedef enum jar_xm_frequency_type_e jar_xm_frequency_type_t;

struct jar_xm_envelope_point_s {
uint16_t frame;
uint16_t value;
};
typedef struct jar_xm_envelope_point_s jar_xm_envelope_point_t;

struct jar_xm_envelope_s {
jar_xm_envelope_point_t points[NUM_ENVELOPE_POINTS];
uint8_t num_points;
uint8_t sustain_point;
uint8_t loop_start_point;
uint8_t loop_end_point;
bool enabled;
bool sustain_enabled;
bool loop_enabled;
};
typedef struct jar_xm_envelope_s jar_xm_envelope_t;

struct jar_xm_sample_s {
char name[SAMPLE_NAME_LENGTH + 1];
int8_t bits; 

uint32_t length;
uint32_t loop_start;
uint32_t loop_length;
uint32_t loop_end;
float volume;
int8_t finetune;
jar_xm_loop_type_t loop_type;
float panning;
int8_t relative_note;
uint64_t latest_trigger;

float* data;
};
typedef struct jar_xm_sample_s jar_xm_sample_t;

struct jar_xm_instrument_s {
char name[INSTRUMENT_NAME_LENGTH + 1];
uint16_t num_samples;
uint8_t sample_of_notes[NUM_NOTES];
jar_xm_envelope_t volume_envelope;
jar_xm_envelope_t panning_envelope;
jar_xm_waveform_type_t vibrato_type;
uint8_t vibrato_sweep;
uint8_t vibrato_depth;
uint8_t vibrato_rate;
uint16_t volume_fadeout;
uint64_t latest_trigger;
bool muted;

jar_xm_sample_t* samples;
};
typedef struct jar_xm_instrument_s jar_xm_instrument_t;

struct jar_xm_pattern_slot_s {
uint8_t note; 
uint8_t instrument; 
uint8_t volume_column;
uint8_t effect_type;
uint8_t effect_param;
};
typedef struct jar_xm_pattern_slot_s jar_xm_pattern_slot_t;

struct jar_xm_pattern_s {
uint16_t num_rows;
jar_xm_pattern_slot_t* slots; 
};
typedef struct jar_xm_pattern_s jar_xm_pattern_t;

struct jar_xm_module_s {
char name[MODULE_NAME_LENGTH + 1];
char trackername[TRACKER_NAME_LENGTH + 1];
uint16_t length;
uint16_t restart_position;
uint16_t num_channels;
uint16_t num_patterns;
uint16_t num_instruments;
jar_xm_frequency_type_t frequency_type;
uint8_t pattern_table[PATTERN_ORDER_TABLE_LENGTH];

jar_xm_pattern_t* patterns;
jar_xm_instrument_t* instruments; 

};
typedef struct jar_xm_module_s jar_xm_module_t;

struct jar_xm_channel_context_s {
float note;
float orig_note; 
jar_xm_instrument_t* instrument; 
jar_xm_sample_t* sample; 
jar_xm_pattern_slot_t* current;

float sample_position;
float period;
float frequency;
float step;
bool ping; 

float volume; 
float panning; 

uint16_t autovibrato_ticks;

bool sustained;
float fadeout_volume;
float volume_envelope_volume;
float panning_envelope_panning;
uint16_t volume_envelope_frame_count;
uint16_t panning_envelope_frame_count;

float autovibrato_note_offset;

bool arp_in_progress;
uint8_t arp_note_offset;
uint8_t volume_slide_param;
uint8_t fine_volume_slide_param;
uint8_t global_volume_slide_param;
uint8_t panning_slide_param;
uint8_t portamento_up_param;
uint8_t portamento_down_param;
uint8_t fine_portamento_up_param;
uint8_t fine_portamento_down_param;
uint8_t extra_fine_portamento_up_param;
uint8_t extra_fine_portamento_down_param;
uint8_t tone_portamento_param;
float tone_portamento_target_period;
uint8_t multi_retrig_param;
uint8_t note_delay_param;
uint8_t pattern_loop_origin; 
uint8_t pattern_loop_count; 
bool vibrato_in_progress;
jar_xm_waveform_type_t vibrato_waveform;
bool vibrato_waveform_retrigger; 
uint8_t vibrato_param;
uint16_t vibrato_ticks; 
float vibrato_note_offset;
jar_xm_waveform_type_t tremolo_waveform;
bool tremolo_waveform_retrigger;
uint8_t tremolo_param;
uint8_t tremolo_ticks;
float tremolo_volume;
uint8_t tremor_param;
bool tremor_on;

uint64_t latest_trigger;
bool muted;

#if JAR_XM_RAMPING


float target_panning;
float target_volume;

unsigned long frame_count;
float end_of_previous_sample[jar_xm_SAMPLE_RAMPING_POINTS];
#endif

float actual_panning;
float actual_volume;
};
typedef struct jar_xm_channel_context_s jar_xm_channel_context_t;

struct jar_xm_context_s {
void* allocated_memory;
jar_xm_module_t module;
uint32_t rate;

uint16_t tempo;
uint16_t bpm;
float global_volume;
float amplification;

#if JAR_XM_RAMPING



float volume_ramp;
float panning_ramp; 
#endif

uint8_t current_table_index;
uint8_t current_row;
uint16_t current_tick; 
float remaining_samples_in_tick;
uint64_t generated_samples;

bool position_jump;
bool pattern_break;
uint8_t jump_dest;
uint8_t jump_row;



uint16_t extra_ticks;

uint8_t* row_loop_count; 
uint8_t loop_count;
uint8_t max_loop_count;

jar_xm_channel_context_t* channels;
};



#if JAR_XM_DEFENSIVE





int jar_xm_check_sanity_preload(const char*, size_t);





int jar_xm_check_sanity_postload(jar_xm_context_t*);

#endif
















size_t jar_xm_get_memory_needed_for_context(const char*, size_t);





char* jar_xm_load_module(jar_xm_context_t*, const char*, size_t, char*);

int jar_xm_create_context(jar_xm_context_t** ctxp, const char* moddata, uint32_t rate) {
return jar_xm_create_context_safe(ctxp, moddata, SIZE_MAX, rate);
}

#define ALIGN(x, b) (((x) + ((b) - 1)) & ~((b) - 1))
#define ALIGN_PTR(x, b) (void*)(((uintptr_t)(x) + ((b) - 1)) & ~((b) - 1))
int jar_xm_create_context_safe(jar_xm_context_t** ctxp, const char* moddata, size_t moddata_length, uint32_t rate) {
#if JAR_XM_DEFENSIVE
int ret;
#endif
size_t bytes_needed;
char* mempool;
jar_xm_context_t* ctx;

#if JAR_XM_DEFENSIVE
if((ret = jar_xm_check_sanity_preload(moddata, moddata_length))) {
DEBUG("jar_xm_check_sanity_preload() returned %i, module is not safe to load", ret);
return 1;
}
#endif

bytes_needed = jar_xm_get_memory_needed_for_context(moddata, moddata_length);
mempool = JARXM_MALLOC(bytes_needed);
if(mempool == NULL && bytes_needed > 0) {

DEBUG("call to JARXM_MALLOC() failed, returned %p", (void*)mempool);
return 2;
}


memset(mempool, 0, bytes_needed);

ctx = (*ctxp = (jar_xm_context_t *)mempool);
ctx->allocated_memory = mempool; 
mempool += sizeof(jar_xm_context_t);

ctx->rate = rate;
mempool = jar_xm_load_module(ctx, moddata, moddata_length, mempool);
mempool = ALIGN_PTR(mempool, 16);

ctx->channels = (jar_xm_channel_context_t*)mempool;
mempool += ctx->module.num_channels * sizeof(jar_xm_channel_context_t);
mempool = ALIGN_PTR(mempool, 16);

ctx->global_volume = 1.f;
ctx->amplification = .25f; 

#if JAR_XM_RAMPING
ctx->volume_ramp = (1.f / 128.f);
ctx->panning_ramp = (1.f / 128.f);
#endif

for(uint8_t i = 0; i < ctx->module.num_channels; ++i) {
jar_xm_channel_context_t* ch = ctx->channels + i;

ch->ping = true;
ch->vibrato_waveform = jar_xm_SINE_WAVEFORM;
ch->vibrato_waveform_retrigger = true;
ch->tremolo_waveform = jar_xm_SINE_WAVEFORM;
ch->tremolo_waveform_retrigger = true;

ch->volume = ch->volume_envelope_volume = ch->fadeout_volume = 1.0f;
ch->panning = ch->panning_envelope_panning = .5f;
ch->actual_volume = .0f;
ch->actual_panning = .5f;
}

mempool = ALIGN_PTR(mempool, 16);
ctx->row_loop_count = (uint8_t*)mempool;
mempool += MAX_NUM_ROWS * sizeof(uint8_t);

#if JAR_XM_DEFENSIVE
if((ret = jar_xm_check_sanity_postload(ctx))) {
DEBUG("jar_xm_check_sanity_postload() returned %i, module is not safe to play", ret);
jar_xm_free_context(ctx);
return 1;
}
#endif

return 0;
}

void jar_xm_free_context(jar_xm_context_t* ctx) {
JARXM_FREE(ctx->allocated_memory);
}

void jar_xm_set_max_loop_count(jar_xm_context_t* ctx, uint8_t loopcnt) {
ctx->max_loop_count = loopcnt;
}

uint8_t jar_xm_get_loop_count(jar_xm_context_t* ctx) {
return ctx->loop_count;
}

bool jar_xm_mute_channel(jar_xm_context_t* ctx, uint16_t channel, bool mute) {
bool old = ctx->channels[channel - 1].muted;
ctx->channels[channel - 1].muted = mute;
return old;
}

bool jar_xm_mute_instrument(jar_xm_context_t* ctx, uint16_t instr, bool mute) {
bool old = ctx->module.instruments[instr - 1].muted;
ctx->module.instruments[instr - 1].muted = mute;
return old;
}



const char* jar_xm_get_module_name(jar_xm_context_t* ctx) {
return ctx->module.name;
}

const char* jar_xm_get_tracker_name(jar_xm_context_t* ctx) {
return ctx->module.trackername;
}



uint16_t jar_xm_get_number_of_channels(jar_xm_context_t* ctx) {
return ctx->module.num_channels;
}

uint16_t jar_xm_get_module_length(jar_xm_context_t* ctx) {
return ctx->module.length;
}

uint16_t jar_xm_get_number_of_patterns(jar_xm_context_t* ctx) {
return ctx->module.num_patterns;
}

uint16_t jar_xm_get_number_of_rows(jar_xm_context_t* ctx, uint16_t pattern) {
return ctx->module.patterns[pattern].num_rows;
}

uint16_t jar_xm_get_number_of_instruments(jar_xm_context_t* ctx) {
return ctx->module.num_instruments;
}

uint16_t jar_xm_get_number_of_samples(jar_xm_context_t* ctx, uint16_t instrument) {
return ctx->module.instruments[instrument - 1].num_samples;
}



void jar_xm_get_playing_speed(jar_xm_context_t* ctx, uint16_t* bpm, uint16_t* tempo) {
if(bpm) *bpm = ctx->bpm;
if(tempo) *tempo = ctx->tempo;
}

void jar_xm_get_position(jar_xm_context_t* ctx, uint8_t* pattern_index, uint8_t* pattern, uint8_t* row, uint64_t* samples) {
if(pattern_index) *pattern_index = ctx->current_table_index;
if(pattern) *pattern = ctx->module.pattern_table[ctx->current_table_index];
if(row) *row = ctx->current_row;
if(samples) *samples = ctx->generated_samples;
}

uint64_t jar_xm_get_latest_trigger_of_instrument(jar_xm_context_t* ctx, uint16_t instr) {
return ctx->module.instruments[instr - 1].latest_trigger;
}

uint64_t jar_xm_get_latest_trigger_of_sample(jar_xm_context_t* ctx, uint16_t instr, uint16_t sample) {
return ctx->module.instruments[instr - 1].samples[sample].latest_trigger;
}

uint64_t jar_xm_get_latest_trigger_of_channel(jar_xm_context_t* ctx, uint16_t chn) {
return ctx->channels[chn - 1].latest_trigger;
}







#define READ_U8(offset) (((offset) < moddata_length) ? (*(uint8_t*)(moddata + (offset))) : 0)
#define READ_U16(offset) ((uint16_t)READ_U8(offset) | ((uint16_t)READ_U8((offset) + 1) << 8))
#define READ_U32(offset) ((uint32_t)READ_U16(offset) | ((uint32_t)READ_U16((offset) + 2) << 16))
#define READ_MEMCPY(ptr, offset, length) memcpy_pad(ptr, length, moddata, moddata_length, offset)

static void memcpy_pad(void* dst, size_t dst_len, const void* src, size_t src_len, size_t offset) {
uint8_t* dst_c = dst;
const uint8_t* src_c = src;


size_t copy_bytes = (src_len >= offset) ? (src_len - offset) : 0;
copy_bytes = copy_bytes > dst_len ? dst_len : copy_bytes;

memcpy(dst_c, src_c + offset, copy_bytes);

memset(dst_c + copy_bytes, 0, dst_len - copy_bytes);
}

#if JAR_XM_DEFENSIVE

int jar_xm_check_sanity_preload(const char* module, size_t module_length) {
if(module_length < 60) {
return 4;
}

if(memcmp("Extended Module: ", module, 17) != 0) {
return 1;
}

if(module[37] != 0x1A) {
return 2;
}

if(module[59] != 0x01 || module[58] != 0x04) {

return 3;
}

return 0;
}

int jar_xm_check_sanity_postload(jar_xm_context_t* ctx) {



for(uint8_t i = 0; i < ctx->module.length; ++i) {
if(ctx->module.pattern_table[i] >= ctx->module.num_patterns) {
if(i+1 == ctx->module.length && ctx->module.length > 1) {

--ctx->module.length;
DEBUG("trimming invalid POT at pos %X", i);
} else {
DEBUG("module has invalid POT, pos %X references nonexistent pattern %X",
i,
ctx->module.pattern_table[i]);
return 1;
}
}
}

return 0;
}

#endif

size_t jar_xm_get_memory_needed_for_context(const char* moddata, size_t moddata_length) {
size_t memory_needed = 0;
size_t offset = 60; 
uint16_t num_channels;
uint16_t num_patterns;
uint16_t num_instruments;


num_channels = READ_U16(offset + 8);

num_patterns = READ_U16(offset + 10);
memory_needed += num_patterns * sizeof(jar_xm_pattern_t);
memory_needed = ALIGN(memory_needed, 16);

num_instruments = READ_U16(offset + 12);
memory_needed += num_instruments * sizeof(jar_xm_instrument_t);
memory_needed = ALIGN(memory_needed, 16);

memory_needed += MAX_NUM_ROWS * READ_U16(offset + 4) * sizeof(uint8_t); 


offset += READ_U32(offset);


for(uint16_t i = 0; i < num_patterns; ++i) {
uint16_t num_rows;

num_rows = READ_U16(offset + 5);
memory_needed += num_rows * num_channels * sizeof(jar_xm_pattern_slot_t);


offset += READ_U32(offset) + READ_U16(offset + 7);
}
memory_needed = ALIGN(memory_needed, 16);


for(uint16_t i = 0; i < num_instruments; ++i) {
uint16_t num_samples;
uint32_t sample_header_size = 0;
uint32_t sample_size_aggregate = 0;

num_samples = READ_U16(offset + 27);
memory_needed += num_samples * sizeof(jar_xm_sample_t);

if(num_samples > 0) {
sample_header_size = READ_U32(offset + 29);
}


offset += READ_U32(offset);

for(uint16_t j = 0; j < num_samples; ++j) {
uint32_t sample_size;
uint8_t flags;

sample_size = READ_U32(offset);
flags = READ_U8(offset + 14);
sample_size_aggregate += sample_size;

if(flags & (1 << 4)) {

memory_needed += sample_size * (sizeof(float) >> 1);
} else {

memory_needed += sample_size * sizeof(float);
}

offset += sample_header_size;
}

offset += sample_size_aggregate;
}

memory_needed += num_channels * sizeof(jar_xm_channel_context_t);
memory_needed += sizeof(jar_xm_context_t);

return memory_needed;
}

char* jar_xm_load_module(jar_xm_context_t* ctx, const char* moddata, size_t moddata_length, char* mempool) {
size_t offset = 0;
jar_xm_module_t* mod = &(ctx->module);


READ_MEMCPY(mod->name, offset + 17, MODULE_NAME_LENGTH);
READ_MEMCPY(mod->trackername, offset + 38, TRACKER_NAME_LENGTH);
offset += 60;


uint32_t header_size = READ_U32(offset);

mod->length = READ_U16(offset + 4);
mod->restart_position = READ_U16(offset + 6);
mod->num_channels = READ_U16(offset + 8);
mod->num_patterns = READ_U16(offset + 10);
mod->num_instruments = READ_U16(offset + 12);

mod->patterns = (jar_xm_pattern_t*)mempool;
mempool += mod->num_patterns * sizeof(jar_xm_pattern_t);
mempool = ALIGN_PTR(mempool, 16);

mod->instruments = (jar_xm_instrument_t*)mempool;
mempool += mod->num_instruments * sizeof(jar_xm_instrument_t);
mempool = ALIGN_PTR(mempool, 16);

uint16_t flags = READ_U32(offset + 14);
mod->frequency_type = (flags & (1 << 0)) ? jar_xm_LINEAR_FREQUENCIES : jar_xm_AMIGA_FREQUENCIES;

ctx->tempo = READ_U16(offset + 16);
ctx->bpm = READ_U16(offset + 18);

READ_MEMCPY(mod->pattern_table, offset + 20, PATTERN_ORDER_TABLE_LENGTH);
offset += header_size;


for(uint16_t i = 0; i < mod->num_patterns; ++i) {
uint16_t packed_patterndata_size = READ_U16(offset + 7);
jar_xm_pattern_t* pat = mod->patterns + i;

pat->num_rows = READ_U16(offset + 5);

pat->slots = (jar_xm_pattern_slot_t*)mempool;
mempool += mod->num_channels * pat->num_rows * sizeof(jar_xm_pattern_slot_t);


offset += READ_U32(offset);

if(packed_patterndata_size == 0) {

memset(pat->slots, 0, sizeof(jar_xm_pattern_slot_t) * pat->num_rows * mod->num_channels);
} else {

for(uint16_t j = 0, k = 0; j < packed_patterndata_size; ++k) {
uint8_t note = READ_U8(offset + j);
jar_xm_pattern_slot_t* slot = pat->slots + k;

if(note & (1 << 7)) {

++j;

if(note & (1 << 0)) {

slot->note = READ_U8(offset + j);
++j;
} else {
slot->note = 0;
}

if(note & (1 << 1)) {

slot->instrument = READ_U8(offset + j);
++j;
} else {
slot->instrument = 0;
}

if(note & (1 << 2)) {

slot->volume_column = READ_U8(offset + j);
++j;
} else {
slot->volume_column = 0;
}

if(note & (1 << 3)) {

slot->effect_type = READ_U8(offset + j);
++j;
} else {
slot->effect_type = 0;
}

if(note & (1 << 4)) {

slot->effect_param = READ_U8(offset + j);
++j;
} else {
slot->effect_param = 0;
}
} else {

slot->note = note;
slot->instrument = READ_U8(offset + j + 1);
slot->volume_column = READ_U8(offset + j + 2);
slot->effect_type = READ_U8(offset + j + 3);
slot->effect_param = READ_U8(offset + j + 4);
j += 5;
}
}
}

offset += packed_patterndata_size;
}
mempool = ALIGN_PTR(mempool, 16);


for(uint16_t i = 0; i < ctx->module.num_instruments; ++i) {
uint32_t sample_header_size = 0;
jar_xm_instrument_t* instr = mod->instruments + i;

READ_MEMCPY(instr->name, offset + 4, INSTRUMENT_NAME_LENGTH);
instr->num_samples = READ_U16(offset + 27);

if(instr->num_samples > 0) {

sample_header_size = READ_U32(offset + 29);
READ_MEMCPY(instr->sample_of_notes, offset + 33, NUM_NOTES);

instr->volume_envelope.num_points = READ_U8(offset + 225);
instr->panning_envelope.num_points = READ_U8(offset + 226);

for(uint8_t j = 0; j < instr->volume_envelope.num_points; ++j) {
instr->volume_envelope.points[j].frame = READ_U16(offset + 129 + 4 * j);
instr->volume_envelope.points[j].value = READ_U16(offset + 129 + 4 * j + 2);
}

for(uint8_t j = 0; j < instr->panning_envelope.num_points; ++j) {
instr->panning_envelope.points[j].frame = READ_U16(offset + 177 + 4 * j);
instr->panning_envelope.points[j].value = READ_U16(offset + 177 + 4 * j + 2);
}

instr->volume_envelope.sustain_point = READ_U8(offset + 227);
instr->volume_envelope.loop_start_point = READ_U8(offset + 228);
instr->volume_envelope.loop_end_point = READ_U8(offset + 229);

instr->panning_envelope.sustain_point = READ_U8(offset + 230);
instr->panning_envelope.loop_start_point = READ_U8(offset + 231);
instr->panning_envelope.loop_end_point = READ_U8(offset + 232);

uint8_t flags = READ_U8(offset + 233);
instr->volume_envelope.enabled = flags & (1 << 0);
instr->volume_envelope.sustain_enabled = flags & (1 << 1);
instr->volume_envelope.loop_enabled = flags & (1 << 2);

flags = READ_U8(offset + 234);
instr->panning_envelope.enabled = flags & (1 << 0);
instr->panning_envelope.sustain_enabled = flags & (1 << 1);
instr->panning_envelope.loop_enabled = flags & (1 << 2);

instr->vibrato_type = READ_U8(offset + 235);
if(instr->vibrato_type == 2) {
instr->vibrato_type = 1;
} else if(instr->vibrato_type == 1) {
instr->vibrato_type = 2;
}
instr->vibrato_sweep = READ_U8(offset + 236);
instr->vibrato_depth = READ_U8(offset + 237);
instr->vibrato_rate = READ_U8(offset + 238);
instr->volume_fadeout = READ_U16(offset + 239);

instr->samples = (jar_xm_sample_t*)mempool;
mempool += instr->num_samples * sizeof(jar_xm_sample_t);
} else {
instr->samples = NULL;
}


offset += READ_U32(offset);

for(uint16_t j = 0; j < instr->num_samples; ++j) {

jar_xm_sample_t* sample = instr->samples + j;

sample->length = READ_U32(offset);
sample->loop_start = READ_U32(offset + 4);
sample->loop_length = READ_U32(offset + 8);
sample->loop_end = sample->loop_start + sample->loop_length;
sample->volume = (float)READ_U8(offset + 12) / (float)0x40;
sample->finetune = (int8_t)READ_U8(offset + 13);

uint8_t flags = READ_U8(offset + 14);
if((flags & 3) == 0) {
sample->loop_type = jar_xm_NO_LOOP;
} else if((flags & 3) == 1) {
sample->loop_type = jar_xm_FORWARD_LOOP;
} else {
sample->loop_type = jar_xm_PING_PONG_LOOP;
}

sample->bits = (flags & (1 << 4)) ? 16 : 8;

sample->panning = (float)READ_U8(offset + 15) / (float)0xFF;
sample->relative_note = (int8_t)READ_U8(offset + 16);
READ_MEMCPY(sample->name, 18, SAMPLE_NAME_LENGTH);
sample->data = (float*)mempool;

if(sample->bits == 16) {

mempool += sample->length * (sizeof(float) >> 1);
sample->loop_start >>= 1;
sample->loop_length >>= 1;
sample->loop_end >>= 1;
sample->length >>= 1;
} else {

mempool += sample->length * sizeof(float);
}

offset += sample_header_size;
}

for(uint16_t j = 0; j < instr->num_samples; ++j) {

jar_xm_sample_t* sample = instr->samples + j;
uint32_t length = sample->length;

if(sample->bits == 16) {
int16_t v = 0;
for(uint32_t k = 0; k < length; ++k) {
v = v + (int16_t)READ_U16(offset + (k << 1));
sample->data[k] = (float)v / (float)(1 << 15);
}
offset += sample->length << 1;
} else {
int8_t v = 0;
for(uint32_t k = 0; k < length; ++k) {
v = v + (int8_t)READ_U8(offset + k);
sample->data[k] = (float)v / (float)(1 << 7);
}
offset += sample->length;
}
}
}

return mempool;
}







static float jar_xm_waveform(jar_xm_waveform_type_t, uint8_t);
static void jar_xm_autovibrato(jar_xm_context_t*, jar_xm_channel_context_t*);
static void jar_xm_vibrato(jar_xm_context_t*, jar_xm_channel_context_t*, uint8_t, uint16_t);
static void jar_xm_tremolo(jar_xm_context_t*, jar_xm_channel_context_t*, uint8_t, uint16_t);
static void jar_xm_arpeggio(jar_xm_context_t*, jar_xm_channel_context_t*, uint8_t, uint16_t);
static void jar_xm_tone_portamento(jar_xm_context_t*, jar_xm_channel_context_t*);
static void jar_xm_pitch_slide(jar_xm_context_t*, jar_xm_channel_context_t*, float);
static void jar_xm_panning_slide(jar_xm_channel_context_t*, uint8_t);
static void jar_xm_volume_slide(jar_xm_channel_context_t*, uint8_t);

static float jar_xm_envelope_lerp(jar_xm_envelope_point_t*, jar_xm_envelope_point_t*, uint16_t);
static void jar_xm_envelope_tick(jar_xm_channel_context_t*, jar_xm_envelope_t*, uint16_t*, float*);
static void jar_xm_envelopes(jar_xm_channel_context_t*);

static float jar_xm_linear_period(float);
static float jar_xm_linear_frequency(float);
static float jar_xm_amiga_period(float);
static float jar_xm_amiga_frequency(float);
static float jar_xm_period(jar_xm_context_t*, float);
static float jar_xm_frequency(jar_xm_context_t*, float, float);
static void jar_xm_update_frequency(jar_xm_context_t*, jar_xm_channel_context_t*);

static void jar_xm_handle_note_and_instrument(jar_xm_context_t*, jar_xm_channel_context_t*, jar_xm_pattern_slot_t*);
static void jar_xm_trigger_note(jar_xm_context_t*, jar_xm_channel_context_t*, unsigned int flags);
static void jar_xm_cut_note(jar_xm_channel_context_t*);
static void jar_xm_key_off(jar_xm_channel_context_t*);

static void jar_xm_post_pattern_change(jar_xm_context_t*);
static void jar_xm_row(jar_xm_context_t*);
static void jar_xm_tick(jar_xm_context_t*);

static float jar_xm_next_of_sample(jar_xm_channel_context_t*);
static void jar_xm_sample(jar_xm_context_t*, float*, float*);



#define jar_xm_TRIGGER_KEEP_VOLUME (1 << 0)
#define jar_xm_TRIGGER_KEEP_PERIOD (1 << 1)
#define jar_xm_TRIGGER_KEEP_SAMPLE_POSITION (1 << 2)

static const uint16_t amiga_frequencies[] = {
1712, 1616, 1525, 1440, 
1357, 1281, 1209, 1141, 
1077, 1017, 961, 907, 
856, 
};

static const float multi_retrig_add[] = {
0.f, -1.f, -2.f, -4.f, 
-8.f, -16.f, 0.f, 0.f, 
0.f, 1.f, 2.f, 4.f, 
8.f, 16.f, 0.f, 0.f 
};

static const float multi_retrig_multiply[] = {
1.f, 1.f, 1.f, 1.f, 
1.f, 1.f, .6666667f, .5f, 
1.f, 1.f, 1.f, 1.f, 
1.f, 1.f, 1.5f, 2.f 
};

#define jar_xm_CLAMP_UP1F(vol, limit) do { if((vol) > (limit)) (vol) = (limit); } while(0)


#define jar_xm_CLAMP_UP(vol) jar_xm_CLAMP_UP1F((vol), 1.f)

#define jar_xm_CLAMP_DOWN1F(vol, limit) do { if((vol) < (limit)) (vol) = (limit); } while(0)


#define jar_xm_CLAMP_DOWN(vol) jar_xm_CLAMP_DOWN1F((vol), .0f)

#define jar_xm_CLAMP2F(vol, up, down) do { if((vol) > (up)) (vol) = (up); else if((vol) < (down)) (vol) = (down); } while(0)



#define jar_xm_CLAMP(vol) jar_xm_CLAMP2F((vol), 1.f, .0f)

#define jar_xm_SLIDE_TOWARDS(val, goal, incr) do { if((val) > (goal)) { (val) -= (incr); jar_xm_CLAMP_DOWN1F((val), (goal)); } else if((val) < (goal)) { (val) += (incr); jar_xm_CLAMP_UP1F((val), (goal)); } } while(0)









#define jar_xm_LERP(u, v, t) ((u) + (t) * ((v) - (u)))
#define jar_xm_INVERSE_LERP(u, v, lerp) (((lerp) - (u)) / ((v) - (u)))

#define HAS_TONE_PORTAMENTO(s) ((s)->effect_type == 3 || (s)->effect_type == 5 || ((s)->volume_column >> 4) == 0xF)


#define HAS_ARPEGGIO(s) ((s)->effect_type == 0 && (s)->effect_param != 0)

#define HAS_VIBRATO(s) ((s)->effect_type == 4 || (s)->effect_param == 6 || ((s)->volume_column >> 4) == 0xB)


#define NOTE_IS_VALID(n) ((n) > 0 && (n) < 97)



static float jar_xm_waveform(jar_xm_waveform_type_t waveform, uint8_t step) {
static unsigned int next_rand = 24492;
step %= 0x40;

switch(waveform) {

case jar_xm_SINE_WAVEFORM:


return -sinf(2.f * 3.141592f * (float)step / (float)0x40);

case jar_xm_RAMP_DOWN_WAVEFORM:

return (float)(0x20 - step) / 0x20;

case jar_xm_SQUARE_WAVEFORM:

return (step >= 0x20) ? 1.f : -1.f;

case jar_xm_RANDOM_WAVEFORM:


next_rand = next_rand * 1103515245 + 12345;
return (float)((next_rand >> 16) & 0x7FFF) / (float)0x4000 - 1.f;

case jar_xm_RAMP_UP_WAVEFORM:

return (float)(step - 0x20) / 0x20;

default:
break;

}

return .0f;
}

static void jar_xm_autovibrato(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch) {
if(ch->instrument == NULL || ch->instrument->vibrato_depth == 0) return;
jar_xm_instrument_t* instr = ch->instrument;
float sweep = 1.f;

if(ch->autovibrato_ticks < instr->vibrato_sweep) {

sweep = jar_xm_LERP(0.f, 1.f, (float)ch->autovibrato_ticks / (float)instr->vibrato_sweep);
}

unsigned int step = ((ch->autovibrato_ticks++) * instr->vibrato_rate) >> 2;
ch->autovibrato_note_offset = .25f * jar_xm_waveform(instr->vibrato_type, step)
* (float)instr->vibrato_depth / (float)0xF * sweep;
jar_xm_update_frequency(ctx, ch);
}

static void jar_xm_vibrato(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, uint8_t param, uint16_t pos) {
unsigned int step = pos * (param >> 4);
ch->vibrato_note_offset =
2.f
* jar_xm_waveform(ch->vibrato_waveform, step)
* (float)(param & 0x0F) / (float)0xF;
jar_xm_update_frequency(ctx, ch);
}

static void jar_xm_tremolo(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, uint8_t param, uint16_t pos) {
unsigned int step = pos * (param >> 4);


ch->tremolo_volume = -1.f * jar_xm_waveform(ch->tremolo_waveform, step)
* (float)(param & 0x0F) / (float)0xF;
}

static void jar_xm_arpeggio(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, uint8_t param, uint16_t tick) {
switch(tick % 3) {
case 0:
ch->arp_in_progress = false;
ch->arp_note_offset = 0;
break;
case 2:
ch->arp_in_progress = true;
ch->arp_note_offset = param >> 4;
break;
case 1:
ch->arp_in_progress = true;
ch->arp_note_offset = param & 0x0F;
break;
}

jar_xm_update_frequency(ctx, ch);
}

static void jar_xm_tone_portamento(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch) {


if(ch->tone_portamento_target_period == 0.f) return;

if(ch->period != ch->tone_portamento_target_period) {
jar_xm_SLIDE_TOWARDS(ch->period,
ch->tone_portamento_target_period,
(ctx->module.frequency_type == jar_xm_LINEAR_FREQUENCIES ?
4.f : 1.f) * ch->tone_portamento_param
);
jar_xm_update_frequency(ctx, ch);
}
}

static void jar_xm_pitch_slide(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, float period_offset) {


if(ctx->module.frequency_type == jar_xm_LINEAR_FREQUENCIES) {
period_offset *= 4.f;
}

ch->period += period_offset;
jar_xm_CLAMP_DOWN(ch->period);


jar_xm_update_frequency(ctx, ch);
}

static void jar_xm_panning_slide(jar_xm_channel_context_t* ch, uint8_t rawval) {
float f;

if((rawval & 0xF0) && (rawval & 0x0F)) {

return;
}

if(rawval & 0xF0) {

f = (float)(rawval >> 4) / (float)0xFF;
ch->panning += f;
jar_xm_CLAMP_UP(ch->panning);
} else {

f = (float)(rawval & 0x0F) / (float)0xFF;
ch->panning -= f;
jar_xm_CLAMP_DOWN(ch->panning);
}
}

static void jar_xm_volume_slide(jar_xm_channel_context_t* ch, uint8_t rawval) {
float f;

if((rawval & 0xF0) && (rawval & 0x0F)) {

return;
}

if(rawval & 0xF0) {

f = (float)(rawval >> 4) / (float)0x40;
ch->volume += f;
jar_xm_CLAMP_UP(ch->volume);
} else {

f = (float)(rawval & 0x0F) / (float)0x40;
ch->volume -= f;
jar_xm_CLAMP_DOWN(ch->volume);
}
}

static float jar_xm_envelope_lerp(jar_xm_envelope_point_t* a, jar_xm_envelope_point_t* b, uint16_t pos) {

if(pos <= a->frame) return a->value;
else if(pos >= b->frame) return b->value;
else {
float p = (float)(pos - a->frame) / (float)(b->frame - a->frame);
return a->value * (1 - p) + b->value * p;
}
}

static void jar_xm_post_pattern_change(jar_xm_context_t* ctx) {

if(ctx->current_table_index >= ctx->module.length) {
ctx->current_table_index = ctx->module.restart_position;
}
}

static float jar_xm_linear_period(float note) {
return 7680.f - note * 64.f;
}

static float jar_xm_linear_frequency(float period) {
return 8363.f * powf(2.f, (4608.f - period) / 768.f);
}

static float jar_xm_amiga_period(float note) {
unsigned int intnote = note;
uint8_t a = intnote % 12;
int8_t octave = note / 12.f - 2;
uint16_t p1 = amiga_frequencies[a], p2 = amiga_frequencies[a + 1];

if(octave > 0) {
p1 >>= octave;
p2 >>= octave;
} else if(octave < 0) {
p1 <<= (-octave);
p2 <<= (-octave);
}

return jar_xm_LERP(p1, p2, note - intnote);
}

static float jar_xm_amiga_frequency(float period) {
if(period == .0f) return .0f;



return 7093789.2f / (period * 2.f);
}

static float jar_xm_period(jar_xm_context_t* ctx, float note) {
switch(ctx->module.frequency_type) {
case jar_xm_LINEAR_FREQUENCIES:
return jar_xm_linear_period(note);
case jar_xm_AMIGA_FREQUENCIES:
return jar_xm_amiga_period(note);
}
return .0f;
}

static float jar_xm_frequency(jar_xm_context_t* ctx, float period, float note_offset) {
uint8_t a;
int8_t octave;
float note;
uint16_t p1, p2;

switch(ctx->module.frequency_type) {

case jar_xm_LINEAR_FREQUENCIES:
return jar_xm_linear_frequency(period - 64.f * note_offset);

case jar_xm_AMIGA_FREQUENCIES:
if(note_offset == 0) {

return jar_xm_amiga_frequency(period);
}


a = octave = 0;


if(period > amiga_frequencies[0]) {
--octave;
while(period > (amiga_frequencies[0] << (-octave))) --octave;
} else if(period < amiga_frequencies[12]) {
++octave;
while(period < (amiga_frequencies[12] >> octave)) ++octave;
}


for(uint8_t i = 0; i < 12; ++i) {
p1 = amiga_frequencies[i], p2 = amiga_frequencies[i + 1];

if(octave > 0) {
p1 >>= octave;
p2 >>= octave;
} else if(octave < 0) {
p1 <<= (-octave);
p2 <<= (-octave);
}

if(p2 <= period && period <= p1) {
a = i;
break;
}
}

if(JAR_XM_DEBUG && (p1 < period || p2 > period)) {
DEBUG("%i <= %f <= %i should hold but doesn't, this is a bug", p2, period, p1);
}

note = 12.f * (octave + 2) + a + jar_xm_INVERSE_LERP(p1, p2, period);

return jar_xm_amiga_frequency(jar_xm_amiga_period(note + note_offset));

}

return .0f;
}

static void jar_xm_update_frequency(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch) {
ch->frequency = jar_xm_frequency(
ctx, ch->period,
(ch->arp_note_offset > 0 ? ch->arp_note_offset : (
ch->vibrato_note_offset + ch->autovibrato_note_offset
))
);
ch->step = ch->frequency / ctx->rate;
}

static void jar_xm_handle_note_and_instrument(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch,
jar_xm_pattern_slot_t* s) {
if(s->instrument > 0) {
if(HAS_TONE_PORTAMENTO(ch->current) && ch->instrument != NULL && ch->sample != NULL) {

jar_xm_trigger_note(ctx, ch, jar_xm_TRIGGER_KEEP_PERIOD | jar_xm_TRIGGER_KEEP_SAMPLE_POSITION);
} else if(s->instrument > ctx->module.num_instruments) {

jar_xm_cut_note(ch);
ch->instrument = NULL;
ch->sample = NULL;
} else {
ch->instrument = ctx->module.instruments + (s->instrument - 1);
if(s->note == 0 && ch->sample != NULL) {


jar_xm_trigger_note(ctx, ch, jar_xm_TRIGGER_KEEP_SAMPLE_POSITION);
}
}
}

if(NOTE_IS_VALID(s->note)) {



jar_xm_instrument_t* instr = ch->instrument;

if(HAS_TONE_PORTAMENTO(ch->current) && instr != NULL && ch->sample != NULL) {

ch->note = s->note + ch->sample->relative_note + ch->sample->finetune / 128.f - 1.f;
ch->tone_portamento_target_period = jar_xm_period(ctx, ch->note);
} else if(instr == NULL || ch->instrument->num_samples == 0) {

jar_xm_cut_note(ch);
} else {
if(instr->sample_of_notes[s->note - 1] < instr->num_samples) {
#if JAR_XM_RAMPING
for(unsigned int z = 0; z < jar_xm_SAMPLE_RAMPING_POINTS; ++z) {
ch->end_of_previous_sample[z] = jar_xm_next_of_sample(ch);
}
ch->frame_count = 0;
#endif
ch->sample = instr->samples + instr->sample_of_notes[s->note - 1];
ch->orig_note = ch->note = s->note + ch->sample->relative_note
+ ch->sample->finetune / 128.f - 1.f;
if(s->instrument > 0) {
jar_xm_trigger_note(ctx, ch, 0);
} else {

jar_xm_trigger_note(ctx, ch, jar_xm_TRIGGER_KEEP_VOLUME);
}
} else {

jar_xm_cut_note(ch);
}
}
} else if(s->note == 97) {

jar_xm_key_off(ch);
}

switch(s->volume_column >> 4) {

case 0x5:
if(s->volume_column > 0x50) break;
case 0x1:
case 0x2:
case 0x3:
case 0x4:

ch->volume = (float)(s->volume_column - 0x10) / (float)0x40;
break;

case 0x8: 
jar_xm_volume_slide(ch, s->volume_column & 0x0F);
break;

case 0x9: 
jar_xm_volume_slide(ch, s->volume_column << 4);
break;

case 0xA: 
ch->vibrato_param = (ch->vibrato_param & 0x0F) | ((s->volume_column & 0x0F) << 4);
break;

case 0xC: 
ch->panning = (float)(
((s->volume_column & 0x0F) << 4) | (s->volume_column & 0x0F)
) / (float)0xFF;
break;

case 0xF: 
if(s->volume_column & 0x0F) {
ch->tone_portamento_param = ((s->volume_column & 0x0F) << 4)
| (s->volume_column & 0x0F);
}
break;

default:
break;

}

switch(s->effect_type) {

case 1: 
if(s->effect_param > 0) {
ch->portamento_up_param = s->effect_param;
}
break;

case 2: 
if(s->effect_param > 0) {
ch->portamento_down_param = s->effect_param;
}
break;

case 3: 
if(s->effect_param > 0) {
ch->tone_portamento_param = s->effect_param;
}
break;

case 4: 
if(s->effect_param & 0x0F) {

ch->vibrato_param = (ch->vibrato_param & 0xF0) | (s->effect_param & 0x0F);
}
if(s->effect_param >> 4) {

ch->vibrato_param = (s->effect_param & 0xF0) | (ch->vibrato_param & 0x0F);
}
break;

case 5: 
if(s->effect_param > 0) {
ch->volume_slide_param = s->effect_param;
}
break;

case 6: 
if(s->effect_param > 0) {
ch->volume_slide_param = s->effect_param;
}
break;

case 7: 
if(s->effect_param & 0x0F) {

ch->tremolo_param = (ch->tremolo_param & 0xF0) | (s->effect_param & 0x0F);
}
if(s->effect_param >> 4) {

ch->tremolo_param = (s->effect_param & 0xF0) | (ch->tremolo_param & 0x0F);
}
break;

case 8: 
ch->panning = (float)s->effect_param / (float)0xFF;
break;

case 9: 
if(ch->sample != NULL && NOTE_IS_VALID(s->note)) {
uint32_t final_offset = s->effect_param << (ch->sample->bits == 16 ? 7 : 8);
if(final_offset >= ch->sample->length) {

ch->sample_position = -1;
break;
}
ch->sample_position = final_offset;
}
break;

case 0xA: 
if(s->effect_param > 0) {
ch->volume_slide_param = s->effect_param;
}
break;

case 0xB: 
if(s->effect_param < ctx->module.length) {
ctx->position_jump = true;
ctx->jump_dest = s->effect_param;
}
break;

case 0xC: 
ch->volume = (float)((s->effect_param > 0x40)
? 0x40 : s->effect_param) / (float)0x40;
break;

case 0xD: 

ctx->pattern_break = true;
ctx->jump_row = (s->effect_param >> 4) * 10 + (s->effect_param & 0x0F);
break;

case 0xE: 
switch(s->effect_param >> 4) {

case 1: 
if(s->effect_param & 0x0F) {
ch->fine_portamento_up_param = s->effect_param & 0x0F;
}
jar_xm_pitch_slide(ctx, ch, -ch->fine_portamento_up_param);
break;

case 2: 
if(s->effect_param & 0x0F) {
ch->fine_portamento_down_param = s->effect_param & 0x0F;
}
jar_xm_pitch_slide(ctx, ch, ch->fine_portamento_down_param);
break;

case 4: 
ch->vibrato_waveform = s->effect_param & 3;
ch->vibrato_waveform_retrigger = !((s->effect_param >> 2) & 1);
break;

case 5: 
if(NOTE_IS_VALID(ch->current->note) && ch->sample != NULL) {
ch->note = ch->current->note + ch->sample->relative_note +
(float)(((s->effect_param & 0x0F) - 8) << 4) / 128.f - 1.f;
ch->period = jar_xm_period(ctx, ch->note);
jar_xm_update_frequency(ctx, ch);
}
break;

case 6: 
if(s->effect_param & 0x0F) {
if((s->effect_param & 0x0F) == ch->pattern_loop_count) {

ch->pattern_loop_count = 0;
break;
}


ch->pattern_loop_count++;
ctx->position_jump = true;
ctx->jump_row = ch->pattern_loop_origin;
ctx->jump_dest = ctx->current_table_index;
} else {

ch->pattern_loop_origin = ctx->current_row;

ctx->jump_row = ch->pattern_loop_origin;
}
break;

case 7: 
ch->tremolo_waveform = s->effect_param & 3;
ch->tremolo_waveform_retrigger = !((s->effect_param >> 2) & 1);
break;

case 0xA: 
if(s->effect_param & 0x0F) {
ch->fine_volume_slide_param = s->effect_param & 0x0F;
}
jar_xm_volume_slide(ch, ch->fine_volume_slide_param << 4);
break;

case 0xB: 
if(s->effect_param & 0x0F) {
ch->fine_volume_slide_param = s->effect_param & 0x0F;
}
jar_xm_volume_slide(ch, ch->fine_volume_slide_param);
break;

case 0xD: 




if(s->note == 0 && s->instrument == 0) {
unsigned int flags = jar_xm_TRIGGER_KEEP_VOLUME;

if(ch->current->effect_param & 0x0F) {
ch->note = ch->orig_note;
jar_xm_trigger_note(ctx, ch, flags);
} else {
jar_xm_trigger_note(
ctx, ch,
flags
| jar_xm_TRIGGER_KEEP_PERIOD
| jar_xm_TRIGGER_KEEP_SAMPLE_POSITION
);
}
}
break;

case 0xE: 
ctx->extra_ticks = (ch->current->effect_param & 0x0F) * ctx->tempo;
break;

default:
break;

}
break;

case 0xF: 
if(s->effect_param > 0) {
if(s->effect_param <= 0x1F) {
ctx->tempo = s->effect_param;
} else {
ctx->bpm = s->effect_param;
}
}
break;

case 16: 
ctx->global_volume = (float)((s->effect_param > 0x40)
? 0x40 : s->effect_param) / (float)0x40;
break;

case 17: 
if(s->effect_param > 0) {
ch->global_volume_slide_param = s->effect_param;
}
break;

case 21: 
ch->volume_envelope_frame_count = s->effect_param;
ch->panning_envelope_frame_count = s->effect_param;
break;

case 25: 
if(s->effect_param > 0) {
ch->panning_slide_param = s->effect_param;
}
break;

case 27: 
if(s->effect_param > 0) {
if((s->effect_param >> 4) == 0) {

ch->multi_retrig_param = (ch->multi_retrig_param & 0xF0) | (s->effect_param & 0x0F);
} else {
ch->multi_retrig_param = s->effect_param;
}
}
break;

case 29: 
if(s->effect_param > 0) {


ch->tremor_param = s->effect_param;
}
break;

case 33: 
switch(s->effect_param >> 4) {
case 1: 
if(s->effect_param & 0x0F) {
ch->extra_fine_portamento_up_param = s->effect_param & 0x0F;
}
jar_xm_pitch_slide(ctx, ch, -1.0f * ch->extra_fine_portamento_up_param);
break;

case 2: 
if(s->effect_param & 0x0F) {
ch->extra_fine_portamento_down_param = s->effect_param & 0x0F;
}
jar_xm_pitch_slide(ctx, ch, ch->extra_fine_portamento_down_param);
break;

default:
break;

}
break;

default:
break;

}
}

static void jar_xm_trigger_note(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, unsigned int flags) {
if(!(flags & jar_xm_TRIGGER_KEEP_SAMPLE_POSITION)) {
ch->sample_position = 0.f;
ch->ping = true;
}

if(ch->sample != NULL) {
if(!(flags & jar_xm_TRIGGER_KEEP_VOLUME)) {
ch->volume = ch->sample->volume;
}

ch->panning = ch->sample->panning;
}

ch->sustained = true;
ch->fadeout_volume = ch->volume_envelope_volume = 1.0f;
ch->panning_envelope_panning = .5f;
ch->volume_envelope_frame_count = ch->panning_envelope_frame_count = 0;
ch->vibrato_note_offset = 0.f;
ch->tremolo_volume = 0.f;
ch->tremor_on = false;

ch->autovibrato_ticks = 0;

if(ch->vibrato_waveform_retrigger) {
ch->vibrato_ticks = 0; 

}
if(ch->tremolo_waveform_retrigger) {
ch->tremolo_ticks = 0;
}

if(!(flags & jar_xm_TRIGGER_KEEP_PERIOD)) {
ch->period = jar_xm_period(ctx, ch->note);
jar_xm_update_frequency(ctx, ch);
}

ch->latest_trigger = ctx->generated_samples;
if(ch->instrument != NULL) {
ch->instrument->latest_trigger = ctx->generated_samples;
}
if(ch->sample != NULL) {
ch->sample->latest_trigger = ctx->generated_samples;
}
}

static void jar_xm_cut_note(jar_xm_channel_context_t* ch) {

ch->volume = .0f;
}

static void jar_xm_key_off(jar_xm_channel_context_t* ch) {

ch->sustained = false;


if(ch->instrument == NULL || !ch->instrument->volume_envelope.enabled) {
jar_xm_cut_note(ch);
}
}

static void jar_xm_row(jar_xm_context_t* ctx) {
if(ctx->position_jump) {
ctx->current_table_index = ctx->jump_dest;
ctx->current_row = ctx->jump_row;
ctx->position_jump = false;
ctx->pattern_break = false;
ctx->jump_row = 0;
jar_xm_post_pattern_change(ctx);
} else if(ctx->pattern_break) {
ctx->current_table_index++;
ctx->current_row = ctx->jump_row;
ctx->pattern_break = false;
ctx->jump_row = 0;
jar_xm_post_pattern_change(ctx);
}

jar_xm_pattern_t* cur = ctx->module.patterns + ctx->module.pattern_table[ctx->current_table_index];
bool in_a_loop = false;


for(uint8_t i = 0; i < ctx->module.num_channels; ++i) {
jar_xm_pattern_slot_t* s = cur->slots + ctx->current_row * ctx->module.num_channels + i;
jar_xm_channel_context_t* ch = ctx->channels + i;

ch->current = s;

if(s->effect_type != 0xE || s->effect_param >> 4 != 0xD) {
jar_xm_handle_note_and_instrument(ctx, ch, s);
} else {
ch->note_delay_param = s->effect_param & 0x0F;
}

if(!in_a_loop && ch->pattern_loop_count > 0) {
in_a_loop = true;
}
}

if(!in_a_loop) {

ctx->loop_count = (ctx->row_loop_count[MAX_NUM_ROWS * ctx->current_table_index + ctx->current_row]++);
}

ctx->current_row++; 



if(!ctx->position_jump && !ctx->pattern_break &&
(ctx->current_row >= cur->num_rows || ctx->current_row == 0)) {
ctx->current_table_index++;
ctx->current_row = ctx->jump_row; 


ctx->jump_row = 0;
jar_xm_post_pattern_change(ctx);
}
}

static void jar_xm_envelope_tick(jar_xm_channel_context_t* ch,
jar_xm_envelope_t* env,
uint16_t* counter,
float* outval) {
if(env->num_points < 2) {

if(env->num_points == 1) {

*outval = (float)env->points[0].value / (float)0x40;
if(*outval > 1) {
*outval = 1;
}
}

return;
} else {
uint8_t j;

if(env->loop_enabled) {
uint16_t loop_start = env->points[env->loop_start_point].frame;
uint16_t loop_end = env->points[env->loop_end_point].frame;
uint16_t loop_length = loop_end - loop_start;

if(*counter >= loop_end) {
*counter -= loop_length;
}
}

for(j = 0; j < (env->num_points - 2); ++j) {
if(env->points[j].frame <= *counter &&
env->points[j+1].frame >= *counter) {
break;
}
}

*outval = jar_xm_envelope_lerp(env->points + j, env->points + j + 1, *counter) / (float)0x40;


if(!ch->sustained || !env->sustain_enabled ||
*counter != env->points[env->sustain_point].frame) {
(*counter)++;
}
}
}

static void jar_xm_envelopes(jar_xm_channel_context_t* ch) {
if(ch->instrument != NULL) {
if(ch->instrument->volume_envelope.enabled) {
if(!ch->sustained) {
ch->fadeout_volume -= (float)ch->instrument->volume_fadeout / 65536.f;
jar_xm_CLAMP_DOWN(ch->fadeout_volume);
}

jar_xm_envelope_tick(ch,
&(ch->instrument->volume_envelope),
&(ch->volume_envelope_frame_count),
&(ch->volume_envelope_volume));
}

if(ch->instrument->panning_envelope.enabled) {
jar_xm_envelope_tick(ch,
&(ch->instrument->panning_envelope),
&(ch->panning_envelope_frame_count),
&(ch->panning_envelope_panning));
}
}
}

static void jar_xm_tick(jar_xm_context_t* ctx) {
if(ctx->current_tick == 0) {
jar_xm_row(ctx);
}

for(uint8_t i = 0; i < ctx->module.num_channels; ++i) {
jar_xm_channel_context_t* ch = ctx->channels + i;

jar_xm_envelopes(ch);
jar_xm_autovibrato(ctx, ch);

if(ch->arp_in_progress && !HAS_ARPEGGIO(ch->current)) {
ch->arp_in_progress = false;
ch->arp_note_offset = 0;
jar_xm_update_frequency(ctx, ch);
}
if(ch->vibrato_in_progress && !HAS_VIBRATO(ch->current)) {
ch->vibrato_in_progress = false;
ch->vibrato_note_offset = 0.f;
jar_xm_update_frequency(ctx, ch);
}

switch(ch->current->volume_column >> 4) {

case 0x6: 
if(ctx->current_tick == 0) break;
jar_xm_volume_slide(ch, ch->current->volume_column & 0x0F);
break;

case 0x7: 
if(ctx->current_tick == 0) break;
jar_xm_volume_slide(ch, ch->current->volume_column << 4);
break;

case 0xB: 
if(ctx->current_tick == 0) break;
ch->vibrato_in_progress = false;
jar_xm_vibrato(ctx, ch, ch->vibrato_param, ch->vibrato_ticks++);
break;

case 0xD: 
if(ctx->current_tick == 0) break;
jar_xm_panning_slide(ch, ch->current->volume_column & 0x0F);
break;

case 0xE: 
if(ctx->current_tick == 0) break;
jar_xm_panning_slide(ch, ch->current->volume_column << 4);
break;

case 0xF: 
if(ctx->current_tick == 0) break;
jar_xm_tone_portamento(ctx, ch);
break;

default:
break;

}

switch(ch->current->effect_type) {

case 0: 
if(ch->current->effect_param > 0) {
char arp_offset = ctx->tempo % 3;
switch(arp_offset) {
case 2: 
if(ctx->current_tick == 1) {
ch->arp_in_progress = true;
ch->arp_note_offset = ch->current->effect_param >> 4;
jar_xm_update_frequency(ctx, ch);
break;
}

case 1: 
if(ctx->current_tick == 0) {
ch->arp_in_progress = false;
ch->arp_note_offset = 0;
jar_xm_update_frequency(ctx, ch);
break;
}

case 0: 
jar_xm_arpeggio(ctx, ch, ch->current->effect_param, ctx->current_tick - arp_offset);
default:
break;
}
}
break;

case 1: 
if(ctx->current_tick == 0) break;
jar_xm_pitch_slide(ctx, ch, -ch->portamento_up_param);
break;

case 2: 
if(ctx->current_tick == 0) break;
jar_xm_pitch_slide(ctx, ch, ch->portamento_down_param);
break;

case 3: 
if(ctx->current_tick == 0) break;
jar_xm_tone_portamento(ctx, ch);
break;

case 4: 
if(ctx->current_tick == 0) break;
ch->vibrato_in_progress = true;
jar_xm_vibrato(ctx, ch, ch->vibrato_param, ch->vibrato_ticks++);
break;

case 5: 
if(ctx->current_tick == 0) break;
jar_xm_tone_portamento(ctx, ch);
jar_xm_volume_slide(ch, ch->volume_slide_param);
break;

case 6: 
if(ctx->current_tick == 0) break;
ch->vibrato_in_progress = true;
jar_xm_vibrato(ctx, ch, ch->vibrato_param, ch->vibrato_ticks++);
jar_xm_volume_slide(ch, ch->volume_slide_param);
break;

case 7: 
if(ctx->current_tick == 0) break;
jar_xm_tremolo(ctx, ch, ch->tremolo_param, ch->tremolo_ticks++);
break;

case 0xA: 
if(ctx->current_tick == 0) break;
jar_xm_volume_slide(ch, ch->volume_slide_param);
break;

case 0xE: 
switch(ch->current->effect_param >> 4) {

case 0x9: 
if(ctx->current_tick != 0 && ch->current->effect_param & 0x0F) {
if(!(ctx->current_tick % (ch->current->effect_param & 0x0F))) {
jar_xm_trigger_note(ctx, ch, 0);
jar_xm_envelopes(ch);
}
}
break;

case 0xC: 
if((ch->current->effect_param & 0x0F) == ctx->current_tick) {
jar_xm_cut_note(ch);
}
break;

case 0xD: 
if(ch->note_delay_param == ctx->current_tick) {
jar_xm_handle_note_and_instrument(ctx, ch, ch->current);
jar_xm_envelopes(ch);
}
break;

default:
break;

}
break;

case 17: 
if(ctx->current_tick == 0) break;
if((ch->global_volume_slide_param & 0xF0) &&
(ch->global_volume_slide_param & 0x0F)) {

break;
}
if(ch->global_volume_slide_param & 0xF0) {

float f = (float)(ch->global_volume_slide_param >> 4) / (float)0x40;
ctx->global_volume += f;
jar_xm_CLAMP_UP(ctx->global_volume);
} else {

float f = (float)(ch->global_volume_slide_param & 0x0F) / (float)0x40;
ctx->global_volume -= f;
jar_xm_CLAMP_DOWN(ctx->global_volume);
}
break;

case 20: 


if(ctx->current_tick == ch->current->effect_param) {
jar_xm_key_off(ch);
}
break;

case 25: 
if(ctx->current_tick == 0) break;
jar_xm_panning_slide(ch, ch->panning_slide_param);
break;

case 27: 
if(ctx->current_tick == 0) break;
if(((ch->multi_retrig_param) & 0x0F) == 0) break;
if((ctx->current_tick % (ch->multi_retrig_param & 0x0F)) == 0) {
float v = ch->volume * multi_retrig_multiply[ch->multi_retrig_param >> 4]
+ multi_retrig_add[ch->multi_retrig_param >> 4];
jar_xm_CLAMP(v);
jar_xm_trigger_note(ctx, ch, 0);
ch->volume = v;
}
break;

case 29: 
if(ctx->current_tick == 0) break;
ch->tremor_on = (
(ctx->current_tick - 1) % ((ch->tremor_param >> 4) + (ch->tremor_param & 0x0F) + 2)
>
(ch->tremor_param >> 4)
);
break;

default:
break;

}

float panning, volume;

panning = ch->panning +
(ch->panning_envelope_panning - .5f) * (.5f - fabs(ch->panning - .5f)) * 2.0f;

if(ch->tremor_on) {
volume = .0f;
} else {
volume = ch->volume + ch->tremolo_volume;
jar_xm_CLAMP(volume);
volume *= ch->fadeout_volume * ch->volume_envelope_volume;
}

#if JAR_XM_RAMPING
ch->target_panning = panning;
ch->target_volume = volume;
#else
ch->actual_panning = panning;
ch->actual_volume = volume;
#endif
}

ctx->current_tick++;
if(ctx->current_tick >= ctx->tempo + ctx->extra_ticks) {
ctx->current_tick = 0;
ctx->extra_ticks = 0;
}


ctx->remaining_samples_in_tick += (float)ctx->rate / ((float)ctx->bpm * 0.4f);
}

static float jar_xm_next_of_sample(jar_xm_channel_context_t* ch) {
if(ch->instrument == NULL || ch->sample == NULL || ch->sample_position < 0) {
#if JAR_XM_RAMPING
if(ch->frame_count < jar_xm_SAMPLE_RAMPING_POINTS) {
return jar_xm_LERP(ch->end_of_previous_sample[ch->frame_count], .0f,
(float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
}
#endif
return .0f;
}
if(ch->sample->length == 0) {
return .0f;
}

float u, v, t;
uint32_t a, b;
a = (uint32_t)ch->sample_position; 



if(JAR_XM_LINEAR_INTERPOLATION) {
b = a + 1;
t = ch->sample_position - a; 
}
u = ch->sample->data[a];

switch(ch->sample->loop_type) {

case jar_xm_NO_LOOP:
if(JAR_XM_LINEAR_INTERPOLATION) {
v = (b < ch->sample->length) ? ch->sample->data[b] : .0f;
}
ch->sample_position += ch->step;
if(ch->sample_position >= ch->sample->length) {
ch->sample_position = -1;
}
break;

case jar_xm_FORWARD_LOOP:
if(JAR_XM_LINEAR_INTERPOLATION) {
v = ch->sample->data[
(b == ch->sample->loop_end) ? ch->sample->loop_start : b
];
}
ch->sample_position += ch->step;
while(ch->sample_position >= ch->sample->loop_end) {
ch->sample_position -= ch->sample->loop_length;
}
break;

case jar_xm_PING_PONG_LOOP:
if(ch->ping) {
ch->sample_position += ch->step;
} else {
ch->sample_position -= ch->step;
}


if(ch->ping) {
if(JAR_XM_LINEAR_INTERPOLATION) {
v = (b >= ch->sample->loop_end) ? ch->sample->data[a] : ch->sample->data[b];
}
if(ch->sample_position >= ch->sample->loop_end) {
ch->ping = false;
ch->sample_position = (ch->sample->loop_end << 1) - ch->sample_position;
}

if(ch->sample_position >= ch->sample->length) {
ch->ping = false;
ch->sample_position -= ch->sample->length - 1;
}
} else {
if(JAR_XM_LINEAR_INTERPOLATION) {
v = u;
u = (b == 1 || b - 2 <= ch->sample->loop_start) ? ch->sample->data[a] : ch->sample->data[b - 2];
}
if(ch->sample_position <= ch->sample->loop_start) {
ch->ping = true;
ch->sample_position = (ch->sample->loop_start << 1) - ch->sample_position;
}

if(ch->sample_position <= .0f) {
ch->ping = true;
ch->sample_position = .0f;
}
}
break;

default:
v = .0f;
break;
}

float endval = JAR_XM_LINEAR_INTERPOLATION ? jar_xm_LERP(u, v, t) : u;

#if JAR_XM_RAMPING
if(ch->frame_count < jar_xm_SAMPLE_RAMPING_POINTS) {

return jar_xm_LERP(ch->end_of_previous_sample[ch->frame_count], endval,
(float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
}
#endif

return endval;
}

static void jar_xm_sample(jar_xm_context_t* ctx, float* left, float* right) {
if(ctx->remaining_samples_in_tick <= 0) {
jar_xm_tick(ctx);
}
ctx->remaining_samples_in_tick--;

*left = 0.f;
*right = 0.f;

if(ctx->max_loop_count > 0 && ctx->loop_count >= ctx->max_loop_count) {
return;
}

for(uint8_t i = 0; i < ctx->module.num_channels; ++i) {
jar_xm_channel_context_t* ch = ctx->channels + i;

if(ch->instrument == NULL || ch->sample == NULL || ch->sample_position < 0) {
continue;
}

const float fval = jar_xm_next_of_sample(ch);

if(!ch->muted && !ch->instrument->muted) {
*left += fval * ch->actual_volume * (1.f - ch->actual_panning);
*right += fval * ch->actual_volume * ch->actual_panning;
}

#if JAR_XM_RAMPING
ch->frame_count++;
jar_xm_SLIDE_TOWARDS(ch->actual_volume, ch->target_volume, ctx->volume_ramp);
jar_xm_SLIDE_TOWARDS(ch->actual_panning, ch->target_panning, ctx->panning_ramp);
#endif
}

const float fgvol = ctx->global_volume * ctx->amplification;
*left *= fgvol;
*right *= fgvol;

#if JAR_XM_DEBUG
if(fabs(*left) > 1 || fabs(*right) > 1) {
DEBUG("clipping frame: %f %f, this is a bad module or a libxm bug", *left, *right);
}
#endif
}

void jar_xm_generate_samples(jar_xm_context_t* ctx, float* output, size_t numsamples) {
if(ctx && output) {
ctx->generated_samples += numsamples;
for(size_t i = 0; i < numsamples; i++) {
jar_xm_sample(ctx, output + (2 * i), output + (2 * i + 1));
}
}
}

uint64_t jar_xm_get_remaining_samples(jar_xm_context_t* ctx)
{
uint64_t total = 0;
uint8_t currentLoopCount = jar_xm_get_loop_count(ctx);
jar_xm_set_max_loop_count(ctx, 0);

while(jar_xm_get_loop_count(ctx) == currentLoopCount)
{
total += ctx->remaining_samples_in_tick;
ctx->remaining_samples_in_tick = 0;
jar_xm_tick(ctx);
}

ctx->loop_count = currentLoopCount;
return total;
}





#undef DEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); fflush(stderr); } while(0)




#define DEBUG_ERR(...) do { fprintf(stderr, __VA_ARGS__); fflush(stderr); } while(0)




#define FATAL(...) do { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); } while(0)





#define FATAL_ERR(...) do { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); } while(0)






int jar_xm_create_context_from_file(jar_xm_context_t** ctx, uint32_t rate, const char* filename) {
FILE* xmf;
int size;
int ret;

xmf = fopen(filename, "rb");
if(xmf == NULL) {
DEBUG_ERR("Could not open input file");
*ctx = NULL;
return 3;
}

fseek(xmf, 0, SEEK_END);
size = ftell(xmf);
rewind(xmf);
if(size == -1) {
fclose(xmf);
DEBUG_ERR("fseek() failed");
*ctx = NULL;
return 4;
}

char* data = JARXM_MALLOC(size + 1);
if(!data || fread(data, 1, size, xmf) < size) {
fclose(xmf);
DEBUG_ERR(data ? "fread() failed" : "JARXM_MALLOC() failed");
JARXM_FREE(data);
*ctx = NULL;
return 5;
}

fclose(xmf);

ret = jar_xm_create_context_safe(ctx, data, size, rate);
JARXM_FREE(data);

switch(ret) {
case 0:
break;

case 1:
DEBUG("could not create context: module is not sane\n");
*ctx = NULL;
return 1;
break;

case 2:
FATAL("could not create context: malloc failed\n");
return 2;
break;

default:
FATAL("could not create context: unknown error\n");
return 6;
break;

}

return 0;
}


void jar_xm_reset(jar_xm_context_t* ctx)
{



for (uint16_t i = 0; i < jar_xm_get_number_of_channels(ctx); i++)
{
jar_xm_cut_note(&ctx->channels[i]);
}
ctx->current_row = 0;
ctx->current_table_index = 0;
ctx->current_tick = 0;
}


#endif


#endif
