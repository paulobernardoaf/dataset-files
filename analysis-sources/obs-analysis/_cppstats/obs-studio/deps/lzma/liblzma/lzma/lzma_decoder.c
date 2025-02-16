#include "lz_decoder.h"
#include "lzma_common.h"
#include "lzma_decoder.h"
#include "range_decoder.h"
#if defined(HAVE_SMALL)
#define seq_4(seq) seq
#define seq_6(seq) seq
#define seq_8(seq) seq
#define seq_len(seq) seq ##_CHOICE, seq ##_CHOICE2, seq ##_BITTREE
#define len_decode(target, ld, pos_state, seq) do { case seq ##_CHOICE: rc_if_0(ld.choice, seq ##_CHOICE) { rc_update_0(ld.choice); probs = ld.low[pos_state];limit = LEN_LOW_SYMBOLS; target = MATCH_LEN_MIN; } else { rc_update_1(ld.choice); case seq ##_CHOICE2: rc_if_0(ld.choice2, seq ##_CHOICE2) { rc_update_0(ld.choice2); probs = ld.mid[pos_state]; limit = LEN_MID_SYMBOLS; target = MATCH_LEN_MIN + LEN_LOW_SYMBOLS; } else { rc_update_1(ld.choice2); probs = ld.high; limit = LEN_HIGH_SYMBOLS; target = MATCH_LEN_MIN + LEN_LOW_SYMBOLS + LEN_MID_SYMBOLS; } } symbol = 1; case seq ##_BITTREE: do { rc_bit(probs[symbol], , , seq ##_BITTREE); } while (symbol < limit); target += symbol - limit; } while (0)
#else 
#define seq_4(seq) seq ##0, seq ##1, seq ##2, seq ##3
#define seq_6(seq) seq ##0, seq ##1, seq ##2, seq ##3, seq ##4, seq ##5
#define seq_8(seq) seq ##0, seq ##1, seq ##2, seq ##3, seq ##4, seq ##5, seq ##6, seq ##7
#define seq_len(seq) seq ##_CHOICE, seq ##_LOW0, seq ##_LOW1, seq ##_LOW2, seq ##_CHOICE2, seq ##_MID0, seq ##_MID1, seq ##_MID2, seq ##_HIGH0, seq ##_HIGH1, seq ##_HIGH2, seq ##_HIGH3, seq ##_HIGH4, seq ##_HIGH5, seq ##_HIGH6, seq ##_HIGH7
#define len_decode(target, ld, pos_state, seq) do { symbol = 1; case seq ##_CHOICE: rc_if_0(ld.choice, seq ##_CHOICE) { rc_update_0(ld.choice); rc_bit_case(ld.low[pos_state][symbol], , , seq ##_LOW0); rc_bit_case(ld.low[pos_state][symbol], , , seq ##_LOW1); rc_bit_case(ld.low[pos_state][symbol], , , seq ##_LOW2); target = symbol - LEN_LOW_SYMBOLS + MATCH_LEN_MIN; } else { rc_update_1(ld.choice); case seq ##_CHOICE2: rc_if_0(ld.choice2, seq ##_CHOICE2) { rc_update_0(ld.choice2); rc_bit_case(ld.mid[pos_state][symbol], , , seq ##_MID0); rc_bit_case(ld.mid[pos_state][symbol], , , seq ##_MID1); rc_bit_case(ld.mid[pos_state][symbol], , , seq ##_MID2); target = symbol - LEN_MID_SYMBOLS + MATCH_LEN_MIN + LEN_LOW_SYMBOLS; } else { rc_update_1(ld.choice2); rc_bit_case(ld.high[symbol], , , seq ##_HIGH0); rc_bit_case(ld.high[symbol], , , seq ##_HIGH1); rc_bit_case(ld.high[symbol], , , seq ##_HIGH2); rc_bit_case(ld.high[symbol], , , seq ##_HIGH3); rc_bit_case(ld.high[symbol], , , seq ##_HIGH4); rc_bit_case(ld.high[symbol], , , seq ##_HIGH5); rc_bit_case(ld.high[symbol], , , seq ##_HIGH6); rc_bit_case(ld.high[symbol], , , seq ##_HIGH7); target = symbol - LEN_HIGH_SYMBOLS + MATCH_LEN_MIN + LEN_LOW_SYMBOLS + LEN_MID_SYMBOLS; } } } while (0)
#endif 
typedef struct {
probability choice;
probability choice2;
probability low[POS_STATES_MAX][LEN_LOW_SYMBOLS];
probability mid[POS_STATES_MAX][LEN_MID_SYMBOLS];
probability high[LEN_HIGH_SYMBOLS];
} lzma_length_decoder;
struct lzma_coder_s {
probability literal[LITERAL_CODERS_MAX][LITERAL_CODER_SIZE];
probability is_match[STATES][POS_STATES_MAX];
probability is_rep[STATES];
probability is_rep0[STATES];
probability is_rep1[STATES];
probability is_rep2[STATES];
probability is_rep0_long[STATES][POS_STATES_MAX];
probability pos_slot[LEN_TO_POS_STATES][POS_SLOTS];
probability pos_special[FULL_DISTANCES - END_POS_MODEL_INDEX];
probability pos_align[ALIGN_TABLE_SIZE];
lzma_length_decoder match_len_decoder;
lzma_length_decoder rep_len_decoder;
lzma_range_decoder rc;
lzma_lzma_state state;
uint32_t rep0; 
uint32_t rep1; 
uint32_t rep2; 
uint32_t rep3; 
uint32_t pos_mask; 
uint32_t literal_context_bits;
uint32_t literal_pos_mask;
lzma_vli uncompressed_size;
enum {
SEQ_NORMALIZE,
SEQ_IS_MATCH,
seq_8(SEQ_LITERAL),
seq_8(SEQ_LITERAL_MATCHED),
SEQ_LITERAL_WRITE,
SEQ_IS_REP,
seq_len(SEQ_MATCH_LEN),
seq_6(SEQ_POS_SLOT),
SEQ_POS_MODEL,
SEQ_DIRECT,
seq_4(SEQ_ALIGN),
SEQ_EOPM,
SEQ_IS_REP0,
SEQ_SHORTREP,
SEQ_IS_REP0_LONG,
SEQ_IS_REP1,
SEQ_IS_REP2,
seq_len(SEQ_REP_LEN),
SEQ_COPY,
} sequence;
probability *probs;
uint32_t symbol;
uint32_t limit;
uint32_t offset;
uint32_t len;
};
static lzma_ret
lzma_decode(lzma_coder *restrict coder, lzma_dict *restrict dictptr,
const uint8_t *restrict in,
size_t *restrict in_pos, size_t in_size)
{
if (!rc_read_init(&coder->rc, in, in_pos, in_size))
return LZMA_OK;
lzma_dict dict = *dictptr;
const size_t dict_start = dict.pos;
rc_to_local(coder->rc, *in_pos);
uint32_t state = coder->state;
uint32_t rep0 = coder->rep0;
uint32_t rep1 = coder->rep1;
uint32_t rep2 = coder->rep2;
uint32_t rep3 = coder->rep3;
const uint32_t pos_mask = coder->pos_mask;
probability *probs = coder->probs;
uint32_t symbol = coder->symbol;
uint32_t limit = coder->limit;
uint32_t offset = coder->offset;
uint32_t len = coder->len;
const uint32_t literal_pos_mask = coder->literal_pos_mask;
const uint32_t literal_context_bits = coder->literal_context_bits;
uint32_t pos_state = dict.pos & pos_mask;
lzma_ret ret = LZMA_OK;
const bool no_eopm = coder->uncompressed_size
!= LZMA_VLI_UNKNOWN;
if (no_eopm && coder->uncompressed_size < dict.limit - dict.pos)
dict.limit = dict.pos + (size_t)(coder->uncompressed_size);
switch (coder->sequence)
while (true) {
pos_state = dict.pos & pos_mask;
case SEQ_NORMALIZE:
case SEQ_IS_MATCH:
if (unlikely(no_eopm && dict.pos == dict.limit))
break;
rc_if_0(coder->is_match[state][pos_state], SEQ_IS_MATCH) {
rc_update_0(coder->is_match[state][pos_state]);
probs = literal_subcoder(coder->literal,
literal_context_bits, literal_pos_mask,
dict.pos, dict_get(&dict, 0));
symbol = 1;
if (is_literal_state(state)) {
#if defined(HAVE_SMALL)
case SEQ_LITERAL:
do {
rc_bit(probs[symbol], , , SEQ_LITERAL);
} while (symbol < (1 << 8));
#else
rc_bit_case(probs[symbol], , , SEQ_LITERAL0);
rc_bit_case(probs[symbol], , , SEQ_LITERAL1);
rc_bit_case(probs[symbol], , , SEQ_LITERAL2);
rc_bit_case(probs[symbol], , , SEQ_LITERAL3);
rc_bit_case(probs[symbol], , , SEQ_LITERAL4);
rc_bit_case(probs[symbol], , , SEQ_LITERAL5);
rc_bit_case(probs[symbol], , , SEQ_LITERAL6);
rc_bit_case(probs[symbol], , , SEQ_LITERAL7);
#endif
} else {
len = dict_get(&dict, rep0) << 1;
offset = 0x100;
#if defined(HAVE_SMALL)
case SEQ_LITERAL_MATCHED:
do {
const uint32_t match_bit
= len & offset;
const uint32_t subcoder_index
= offset + match_bit
+ symbol;
rc_bit(probs[subcoder_index],
offset &= ~match_bit,
offset &= match_bit,
SEQ_LITERAL_MATCHED);
len <<= 1;
} while (symbol < (1 << 8));
#else
uint32_t match_bit;
uint32_t subcoder_index;
#define d(seq) case seq: match_bit = len & offset; subcoder_index = offset + match_bit + symbol; rc_bit(probs[subcoder_index], offset &= ~match_bit, offset &= match_bit, seq)
d(SEQ_LITERAL_MATCHED0);
len <<= 1;
d(SEQ_LITERAL_MATCHED1);
len <<= 1;
d(SEQ_LITERAL_MATCHED2);
len <<= 1;
d(SEQ_LITERAL_MATCHED3);
len <<= 1;
d(SEQ_LITERAL_MATCHED4);
len <<= 1;
d(SEQ_LITERAL_MATCHED5);
len <<= 1;
d(SEQ_LITERAL_MATCHED6);
len <<= 1;
d(SEQ_LITERAL_MATCHED7);
#undef d
#endif
}
static const lzma_lzma_state next_state[] = {
STATE_LIT_LIT,
STATE_LIT_LIT,
STATE_LIT_LIT,
STATE_LIT_LIT,
STATE_MATCH_LIT_LIT,
STATE_REP_LIT_LIT,
STATE_SHORTREP_LIT_LIT,
STATE_MATCH_LIT,
STATE_REP_LIT,
STATE_SHORTREP_LIT,
STATE_MATCH_LIT,
STATE_REP_LIT
};
state = next_state[state];
case SEQ_LITERAL_WRITE:
if (unlikely(dict_put(&dict, symbol))) {
coder->sequence = SEQ_LITERAL_WRITE;
goto out;
}
continue;
}
rc_update_1(coder->is_match[state][pos_state]);
case SEQ_IS_REP:
rc_if_0(coder->is_rep[state], SEQ_IS_REP) {
rc_update_0(coder->is_rep[state]);
update_match(state);
rep3 = rep2;
rep2 = rep1;
rep1 = rep0;
len_decode(len, coder->match_len_decoder,
pos_state, SEQ_MATCH_LEN);
probs = coder->pos_slot[get_len_to_pos_state(len)];
symbol = 1;
#if defined(HAVE_SMALL)
case SEQ_POS_SLOT:
do {
rc_bit(probs[symbol], , , SEQ_POS_SLOT);
} while (symbol < POS_SLOTS);
#else
rc_bit_case(probs[symbol], , , SEQ_POS_SLOT0);
rc_bit_case(probs[symbol], , , SEQ_POS_SLOT1);
rc_bit_case(probs[symbol], , , SEQ_POS_SLOT2);
rc_bit_case(probs[symbol], , , SEQ_POS_SLOT3);
rc_bit_case(probs[symbol], , , SEQ_POS_SLOT4);
rc_bit_case(probs[symbol], , , SEQ_POS_SLOT5);
#endif
symbol -= POS_SLOTS;
assert(symbol <= 63);
if (symbol < START_POS_MODEL_INDEX) {
rep0 = symbol;
} else {
limit = (symbol >> 1) - 1;
assert(limit >= 1 && limit <= 30);
rep0 = 2 + (symbol & 1);
if (symbol < END_POS_MODEL_INDEX) {
assert(limit <= 5);
rep0 <<= limit;
assert(rep0 <= 96);
assert((int32_t)(rep0 - symbol - 1)
>= -1);
assert((int32_t)(rep0 - symbol - 1)
<= 82);
probs = coder->pos_special + rep0
- symbol - 1;
symbol = 1;
offset = 0;
case SEQ_POS_MODEL:
#if defined(HAVE_SMALL)
do {
rc_bit(probs[symbol], ,
rep0 += 1 << offset,
SEQ_POS_MODEL);
} while (++offset < limit);
#else
switch (limit) {
case 5:
assert(offset == 0);
rc_bit(probs[symbol], ,
rep0 += 1,
SEQ_POS_MODEL);
++offset;
--limit;
case 4:
rc_bit(probs[symbol], ,
rep0 += 1 << offset,
SEQ_POS_MODEL);
++offset;
--limit;
case 3:
rc_bit(probs[symbol], ,
rep0 += 1 << offset,
SEQ_POS_MODEL);
++offset;
--limit;
case 2:
rc_bit(probs[symbol], ,
rep0 += 1 << offset,
SEQ_POS_MODEL);
++offset;
--limit;
case 1:
rc_bit_last(probs[symbol], ,
rep0 += 1 << offset,
SEQ_POS_MODEL);
}
#endif
} else {
assert(symbol >= 14);
assert(limit >= 6);
limit -= ALIGN_BITS;
assert(limit >= 2);
case SEQ_DIRECT:
do {
rc_direct(rep0, SEQ_DIRECT);
} while (--limit > 0);
rep0 <<= ALIGN_BITS;
symbol = 1;
#if defined(HAVE_SMALL)
offset = 0;
case SEQ_ALIGN:
do {
rc_bit(coder->pos_align[
symbol], ,
rep0 += 1 << offset,
SEQ_ALIGN);
} while (++offset < ALIGN_BITS);
#else
case SEQ_ALIGN0:
rc_bit(coder->pos_align[symbol], ,
rep0 += 1, SEQ_ALIGN0);
case SEQ_ALIGN1:
rc_bit(coder->pos_align[symbol], ,
rep0 += 2, SEQ_ALIGN1);
case SEQ_ALIGN2:
rc_bit(coder->pos_align[symbol], ,
rep0 += 4, SEQ_ALIGN2);
case SEQ_ALIGN3:
rc_bit_last(coder->pos_align[symbol], ,
rep0 += 8, SEQ_ALIGN3);
#endif
if (rep0 == UINT32_MAX) {
if (coder->uncompressed_size
!= LZMA_VLI_UNKNOWN) {
ret = LZMA_DATA_ERROR;
goto out;
}
case SEQ_EOPM:
rc_normalize(SEQ_EOPM);
ret = LZMA_STREAM_END;
goto out;
}
}
}
if (unlikely(!dict_is_distance_valid(&dict, rep0))) {
ret = LZMA_DATA_ERROR;
goto out;
}
} else {
rc_update_1(coder->is_rep[state]);
if (unlikely(!dict_is_distance_valid(&dict, 0))) {
ret = LZMA_DATA_ERROR;
goto out;
}
case SEQ_IS_REP0:
rc_if_0(coder->is_rep0[state], SEQ_IS_REP0) {
rc_update_0(coder->is_rep0[state]);
case SEQ_IS_REP0_LONG:
rc_if_0(coder->is_rep0_long[state][pos_state],
SEQ_IS_REP0_LONG) {
rc_update_0(coder->is_rep0_long[
state][pos_state]);
update_short_rep(state);
case SEQ_SHORTREP:
if (unlikely(dict_put(&dict, dict_get(
&dict, rep0)))) {
coder->sequence = SEQ_SHORTREP;
goto out;
}
continue;
}
rc_update_1(coder->is_rep0_long[
state][pos_state]);
} else {
rc_update_1(coder->is_rep0[state]);
case SEQ_IS_REP1:
rc_if_0(coder->is_rep1[state], SEQ_IS_REP1) {
rc_update_0(coder->is_rep1[state]);
const uint32_t distance = rep1;
rep1 = rep0;
rep0 = distance;
} else {
rc_update_1(coder->is_rep1[state]);
case SEQ_IS_REP2:
rc_if_0(coder->is_rep2[state],
SEQ_IS_REP2) {
rc_update_0(coder->is_rep2[
state]);
const uint32_t distance = rep2;
rep2 = rep1;
rep1 = rep0;
rep0 = distance;
} else {
rc_update_1(coder->is_rep2[
state]);
const uint32_t distance = rep3;
rep3 = rep2;
rep2 = rep1;
rep1 = rep0;
rep0 = distance;
}
}
}
update_long_rep(state);
len_decode(len, coder->rep_len_decoder,
pos_state, SEQ_REP_LEN);
}
assert(len >= MATCH_LEN_MIN);
assert(len <= MATCH_LEN_MAX);
case SEQ_COPY:
if (unlikely(dict_repeat(&dict, rep0, &len))) {
coder->sequence = SEQ_COPY;
goto out;
}
}
rc_normalize(SEQ_NORMALIZE);
coder->sequence = SEQ_IS_MATCH;
out:
dictptr->pos = dict.pos;
dictptr->full = dict.full;
rc_from_local(coder->rc, *in_pos);
coder->state = state;
coder->rep0 = rep0;
coder->rep1 = rep1;
coder->rep2 = rep2;
coder->rep3 = rep3;
coder->probs = probs;
coder->symbol = symbol;
coder->limit = limit;
coder->offset = offset;
coder->len = len;
if (coder->uncompressed_size != LZMA_VLI_UNKNOWN) {
coder->uncompressed_size -= dict.pos - dict_start;
if (coder->uncompressed_size == 0 && ret == LZMA_OK
&& coder->sequence != SEQ_NORMALIZE)
ret = coder->sequence == SEQ_IS_MATCH
? LZMA_STREAM_END : LZMA_DATA_ERROR;
}
if (ret == LZMA_STREAM_END) {
if (!rc_is_finished(coder->rc))
ret = LZMA_DATA_ERROR;
rc_reset(coder->rc);
}
return ret;
}
static void
lzma_decoder_uncompressed(lzma_coder *coder, lzma_vli uncompressed_size)
{
coder->uncompressed_size = uncompressed_size;
}
static void
lzma_decoder_reset(lzma_coder *coder, const void *opt)
{
const lzma_options_lzma *options = opt;
coder->pos_mask = (1U << options->pb) - 1;
literal_init(coder->literal, options->lc, options->lp);
coder->literal_context_bits = options->lc;
coder->literal_pos_mask = (1U << options->lp) - 1;
coder->state = STATE_LIT_LIT;
coder->rep0 = 0;
coder->rep1 = 0;
coder->rep2 = 0;
coder->rep3 = 0;
coder->pos_mask = (1U << options->pb) - 1;
rc_reset(coder->rc);
for (uint32_t i = 0; i < STATES; ++i) {
for (uint32_t j = 0; j <= coder->pos_mask; ++j) {
bit_reset(coder->is_match[i][j]);
bit_reset(coder->is_rep0_long[i][j]);
}
bit_reset(coder->is_rep[i]);
bit_reset(coder->is_rep0[i]);
bit_reset(coder->is_rep1[i]);
bit_reset(coder->is_rep2[i]);
}
for (uint32_t i = 0; i < LEN_TO_POS_STATES; ++i)
bittree_reset(coder->pos_slot[i], POS_SLOT_BITS);
for (uint32_t i = 0; i < FULL_DISTANCES - END_POS_MODEL_INDEX; ++i)
bit_reset(coder->pos_special[i]);
bittree_reset(coder->pos_align, ALIGN_BITS);
const uint32_t num_pos_states = 1U << options->pb;
bit_reset(coder->match_len_decoder.choice);
bit_reset(coder->match_len_decoder.choice2);
bit_reset(coder->rep_len_decoder.choice);
bit_reset(coder->rep_len_decoder.choice2);
for (uint32_t pos_state = 0; pos_state < num_pos_states; ++pos_state) {
bittree_reset(coder->match_len_decoder.low[pos_state],
LEN_LOW_BITS);
bittree_reset(coder->match_len_decoder.mid[pos_state],
LEN_MID_BITS);
bittree_reset(coder->rep_len_decoder.low[pos_state],
LEN_LOW_BITS);
bittree_reset(coder->rep_len_decoder.mid[pos_state],
LEN_MID_BITS);
}
bittree_reset(coder->match_len_decoder.high, LEN_HIGH_BITS);
bittree_reset(coder->rep_len_decoder.high, LEN_HIGH_BITS);
coder->sequence = SEQ_IS_MATCH;
coder->probs = NULL;
coder->symbol = 0;
coder->limit = 0;
coder->offset = 0;
coder->len = 0;
return;
}
extern lzma_ret
lzma_lzma_decoder_create(lzma_lz_decoder *lz, lzma_allocator *allocator,
const void *opt, lzma_lz_options *lz_options)
{
if (lz->coder == NULL) {
lz->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (lz->coder == NULL)
return LZMA_MEM_ERROR;
lz->code = &lzma_decode;
lz->reset = &lzma_decoder_reset;
lz->set_uncompressed = &lzma_decoder_uncompressed;
}
const lzma_options_lzma *options = opt;
lz_options->dict_size = options->dict_size;
lz_options->preset_dict = options->preset_dict;
lz_options->preset_dict_size = options->preset_dict_size;
return LZMA_OK;
}
static lzma_ret
lzma_decoder_init(lzma_lz_decoder *lz, lzma_allocator *allocator,
const void *options, lzma_lz_options *lz_options)
{
if (!is_lclppb_valid(options))
return LZMA_PROG_ERROR;
return_if_error(lzma_lzma_decoder_create(
lz, allocator, options, lz_options));
lzma_decoder_reset(lz->coder, options);
lzma_decoder_uncompressed(lz->coder, LZMA_VLI_UNKNOWN);
return LZMA_OK;
}
extern lzma_ret
lzma_lzma_decoder_init(lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters)
{
assert(filters[1].init == NULL);
return lzma_lz_decoder_init(next, allocator, filters,
&lzma_decoder_init);
}
extern bool
lzma_lzma_lclppb_decode(lzma_options_lzma *options, uint8_t byte)
{
if (byte > (4 * 5 + 4) * 9 + 8)
return true;
options->pb = byte / (9 * 5);
byte -= options->pb * 9 * 5;
options->lp = byte / 9;
options->lc = byte - options->lp * 9;
return options->lc + options->lp > LZMA_LCLP_MAX;
}
extern uint64_t
lzma_lzma_decoder_memusage_nocheck(const void *options)
{
const lzma_options_lzma *const opt = options;
return sizeof(lzma_coder) + lzma_lz_decoder_memusage(opt->dict_size);
}
extern uint64_t
lzma_lzma_decoder_memusage(const void *options)
{
if (!is_lclppb_valid(options))
return UINT64_MAX;
return lzma_lzma_decoder_memusage_nocheck(options);
}
extern lzma_ret
lzma_lzma_props_decode(void **options, lzma_allocator *allocator,
const uint8_t *props, size_t props_size)
{
if (props_size != 5)
return LZMA_OPTIONS_ERROR;
lzma_options_lzma *opt
= lzma_alloc(sizeof(lzma_options_lzma), allocator);
if (opt == NULL)
return LZMA_MEM_ERROR;
if (lzma_lzma_lclppb_decode(opt, props[0]))
goto error;
opt->dict_size = unaligned_read32le(props + 1);
opt->preset_dict = NULL;
opt->preset_dict_size = 0;
*options = opt;
return LZMA_OK;
error:
lzma_free(opt, allocator);
return LZMA_OPTIONS_ERROR;
}
